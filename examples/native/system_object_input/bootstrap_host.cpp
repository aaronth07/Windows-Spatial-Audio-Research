#include "bootstrap_api.h"
#include <wrl/client.h>
#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <string>
using Microsoft::WRL::ComPtr;
static void Check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static void Ok(HRESULT value){Check(SUCCEEDED(value),"HRESULT failure");}
static constexpr GUID FloatFormat={3,0,0x10,{0x80,0,0,0xaa,0,0x38,0x9b,0x71}};
static constexpr GUID TestEndpoint={0x87654321,0x1234,0x5678,{0x90,0xab,0xcd,0xef,0x01,0x23,0x45,0x67}};
class Media final:public IAudioMediaType {
 ULONG references{1};
public:
 UNCOMPRESSEDAUDIOFORMAT format;WAVEFORMATEX wave{};
 explicit Media(UINT32 channels):format{FloatFormat,channels,4,32,48000,0}{wave={3,static_cast<WORD>(channels),48000,48000*channels*4,static_cast<WORD>(channels*4),32,0};}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(iid!=IID_IUnknown&&iid!=__uuidof(IAudioMediaType))return E_NOINTERFACE;*out=this;AddRef();return S_OK;}
 ULONG STDMETHODCALLTYPE AddRef()override{return ++references;}
 ULONG STDMETHODCALLTYPE Release()override{ULONG n=--references;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE IsCompressedFormat(BOOL* out)override{if(!out)return E_POINTER;*out=FALSE;return S_OK;}
 HRESULT STDMETHODCALLTYPE IsEqual(IAudioMediaType*,DWORD* flags)override{if(!flags)return E_POINTER;*flags=0;return S_FALSE;}
 const WAVEFORMATEX* STDMETHODCALLTYPE GetAudioFormat()override{return &wave;}
 HRESULT STDMETHODCALLTYPE GetUncompressedAudioFormat(UNCOMPRESSEDAUDIOFORMAT* out)override{if(!out)return E_POINTER;*out=format;return S_OK;}
};
class Properties final:public IPropertyStore {
 ULONG references{1};
public:
 std::wstring logPath,factoryPath,sha256;bool bridge{};int malformed{};bool enable{},badEndpoint{};
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(iid!=IID_IUnknown&&iid!=__uuidof(IPropertyStore))return E_NOINTERFACE;*out=this;AddRef();return S_OK;}
 ULONG STDMETHODCALLTYPE AddRef()override{return ++references;}
 ULONG STDMETHODCALLTYPE Release()override{ULONG n=--references;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE GetCount(DWORD* out)override{if(!out)return E_POINTER;*out=1;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetAt(DWORD,PROPERTYKEY*)override{return E_NOTIMPL;}
 HRESULT STDMETHODCALLTYPE GetValue(REFPROPERTYKEY key,PROPVARIANT* out)override{
  if(!out)return E_POINTER;PropVariantInit(out);if(key==EndpointGuidKey){out->vt=VT_LPWSTR;if(badEndpoint){out->pwszVal=static_cast<wchar_t*>(CoTaskMemAlloc(4*sizeof(wchar_t)));if(!out->pwszVal)return E_OUTOFMEMORY;wcscpy_s(out->pwszVal,4,L"bad");return S_OK;}return StringFromCLSID(TestEndpoint,&out->pwszVal);}if(!bridge)return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
  if(key==BootstrapPreloadKey){if(malformed==2){out->vt=VT_UI4;out->ulVal=1;}else{out->vt=VT_BOOL;out->boolVal=enable?VARIANT_TRUE:VARIANT_FALSE;}return S_OK;}
  const std::wstring* value=key==BootstrapLogPathKey?&logPath:key==BootstrapFactoryPathKey?&factoryPath:key==BootstrapFactorySha256Key?&sha256:nullptr;
  if(!value||value->empty())return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);if(key==BootstrapLogPathKey&&malformed==1){out->vt=VT_UI4;out->ulVal=1;return S_OK;}
  out->pwszVal=static_cast<wchar_t*>(CoTaskMemAlloc((value->size()+1)*sizeof(wchar_t)));if(!out->pwszVal)return E_OUTOFMEMORY;std::memcpy(out->pwszVal,value->c_str(),(value->size()+1)*sizeof(wchar_t));out->vt=VT_LPWSTR;return S_OK;
 }
 HRESULT STDMETHODCALLTYPE SetValue(REFPROPERTYKEY,REFPROPVARIANT)override{return E_ACCESSDENIED;}
 HRESULT STDMETHODCALLTYPE Commit()override{return E_ACCESSDENIED;}
};
int wmain(int argc,wchar_t** argv){
 if(argc!=3){std::cerr<<"Usage: bootstrap_host NEW-output-directory PINNED-native-CPU-path\n";return 2;}
 try{
  const auto directory=std::filesystem::absolute(argv[1]);Check(std::filesystem::create_directory(directory),"Output exists");
  wchar_t executable[32768]{};Check(GetModuleFileNameW(nullptr,executable,32768)>0,"Host path");const auto library=std::filesystem::path(executable).parent_path()/L"bootstrap_apo.dll";
  HMODULE module=LoadLibraryExW(library.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);Check(module!=nullptr,"Bootstrap load");
  auto configure=reinterpret_cast<ConfigureBootstrapFn>(GetProcAddress(module,"ConfigureBootstrap"));auto close=reinterpret_cast<CloseBootstrapFn>(GetProcAddress(module,"CloseBootstrap"));auto calls=reinterpret_cast<GetBootstrapCounterFn>(GetProcAddress(module,"GetBootstrapCalls"));auto errors=reinterpret_cast<GetBootstrapCounterFn>(GetProcAddress(module,"GetBootstrapErrors"));auto instances=reinterpret_cast<GetBootstrapCounterFn>(GetProcAddress(module,"GetBootstrapInstances"));auto getClass=reinterpret_cast<HRESULT(WINAPI*)(REFCLSID,REFIID,void**)>(GetProcAddress(module,"DllGetClassObject"));auto unload=reinterpret_cast<HRESULT(WINAPI*)()>(GetProcAddress(module,"DllCanUnloadNow"));Check(configure&&close&&calls&&errors&&instances&&getClass&&unload,"Exports");
  Check(configure(nullptr,FALSE,nullptr)==E_INVALIDARG&&configure(L"relative.bin",FALSE,nullptr)==E_INVALIDARG,"Configuration negatives");
  ComPtr<IClassFactory> factory;Ok(getClass(BootstrapClass,IID_PPV_ARGS(&factory)));Check(unload()==S_FALSE,"Factory prevents unload");unsigned exactCases=0,negativeCases=0;ULONG expectedErrors=0;
  // No Configure: construction and processing must perform no logging or preload.
  {ComPtr<IAudioProcessingObject> a;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&a)));APOInitBaseStruct init{sizeof(init),BootstrapClass};Ok(a->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init)));Check(GetModuleHandleW(L"HrtfDspCpu.dll")==nullptr,"Default no preload");}
  for(unsigned mode=0;mode<2;mode++){
   auto log=directory/(mode?L"preload.events":L"inert.events");Ok(configure(log.c_str(),mode?TRUE:FALSE,mode?argv[2]:nullptr));Check(unload()==S_FALSE,"Configuration prevents unload");
   for(UINT32 channelCount:{1U,2U,16U,64U}){
    ComPtr<IAudioProcessingObject> apo;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&apo)));ComPtr<IAudioProcessingObjectConfiguration> config;Ok(apo.As(&config));ComPtr<IAudioProcessingObjectRT> rt;Ok(apo.As(&rt));ComPtr<IAudioSystemEffects> effect;Ok(apo.As(&effect));ComPtr<IUnknown> identity,other;Ok(apo.As(&identity));Ok(rt.As(&other));Check(identity.Get()==other.Get(),"COM identity");
    Check(close()==E_UNEXPECTED&&configure(log.c_str(),FALSE,nullptr)==E_UNEXPECTED,"Live configuration rejection");
    APO_REG_PROPERTIES* properties{};Ok(apo->GetRegistrationProperties(&properties));Check(properties->u32NumAPOInterfaces==4&&properties->clsid==BootstrapClass,"Registration properties");CoTaskMemFree(properties);
    HNSTIME latency=-1;Ok(apo->GetLatency(&latency));Check(latency==0,"Latency");ComPtr<Properties> endpoint;endpoint.Attach(new Properties);APOInitSystemEffects init{};init.APOInit={sizeof(init),BootstrapClass};init.pAPOEndpointProperties=endpoint.Get();Check(apo->Initialize(0,nullptr)==E_INVALIDARG,"Initialize null");Ok(apo->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init)));Check(apo->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init))==APOERR_ALREADY_INITIALIZED,"Double initialization");
    ComPtr<Media> media;media.Attach(new Media(channelCount));ComPtr<IAudioMediaType> supported;Ok(apo->IsInputFormatSupported(nullptr,media.Get(),&supported));Check(supported.Get()==media.Get(),"Format ownership");supported.Reset();Ok(apo->IsOutputFormatSupported(media.Get(),media.Get(),&supported));supported.Reset();auto original=media->format;media->format.dwValidBitsPerSample=16;Check(apo->IsInputFormatSupported(nullptr,media.Get(),&supported)==APOERR_FORMAT_NOT_SUPPORTED&&!supported,"Unsupported format");media->format=original;
    APO_CONNECTION_DESCRIPTOR in{APO_CONNECTION_BUFFER_TYPE_EXTERNAL,0,1024,media.Get(),APO_CONNECTION_DESCRIPTOR_SIGNATURE},out=in;auto pi=&in;auto po=&out;Check(config->LockForProcess(0,&pi,1,&po)==APOERR_NUM_CONNECTIONS_INVALID,"Connection count");out.u32MaxFrameCount=512;Check(config->LockForProcess(1,&pi,1,&po)==APOERR_INVALID_OUTPUT_MAXFRAMECOUNT,"Output bound");out.u32MaxFrameCount=1024;Ok(config->LockForProcess(1,&pi,1,&po));Check(apo->Reset()==APOERR_APO_LOCKED&&config->LockForProcess(1,&pi,1,&po)==APOERR_APO_LOCKED,"Locked state");UINT32 count{};Ok(apo->GetInputChannelCount(&count));Check(count==channelCount,"Channel count");
    for(UINT32 frames:{0U,1U,48U,128U,480U,1024U}){
     std::vector<float> source(size_t(1024)*channelCount),guarded(source.size()+32,12345.f);for(size_t i=0;i<source.size();i++)source[i]=float(int(i%97)-48)/2048.f;float* target=guarded.data()+16;
     APO_CONNECTION_PROPERTY input{reinterpret_cast<UINT_PTR>(source.data()),frames,BUFFER_VALID,APO_CONNECTION_PROPERTY_SIGNATURE},output{reinterpret_cast<UINT_PTR>(target),0,BUFFER_INVALID,APO_CONNECTION_PROPERTY_SIGNATURE};auto inputPtr=&input;auto outputPtr=&output;rt->APOProcess(1,&inputPtr,1,&outputPtr);Check(output.u32ValidFrameCount==frames&&output.u32BufferFlags==BUFFER_VALID,"Valid flags");Check(std::equal(source.begin(),source.begin()+size_t(frames)*channelCount,target),"Exact PCM");Check(std::all_of(guarded.begin(),guarded.begin()+16,[](float n){return n==12345.f;})&&std::all_of(guarded.begin()+16+size_t(frames)*channelCount,guarded.end(),[](float n){return n==12345.f;}),"Output guards");
     input.u32BufferFlags=BUFFER_SILENT;rt->APOProcess(1,&inputPtr,1,&outputPtr);Check(output.u32BufferFlags==BUFFER_SILENT&&output.u32ValidFrameCount==frames&&std::all_of(target,target+size_t(frames)*channelCount,[](float n){return n==0;}),"Silent");
     input.u32BufferFlags=BUFFER_VALID;auto copy=source;input.pBuffer=reinterpret_cast<UINT_PTR>(copy.data());rt->APOProcess(1,&inputPtr,1,&inputPtr);Check(input.u32ValidFrameCount==frames&&input.u32BufferFlags==BUFFER_VALID&&copy==source,"Same property in-place");Check(rt->CalcInputFrames(frames)==frames&&rt->CalcOutputFrames(frames)==frames,"Frame calculators");++exactCases;
    }
    std::vector<float> sentinel(size_t(1024)*channelCount,54321.f);APO_CONNECTION_PROPERTY invalid{0,1025,BUFFER_VALID,APO_CONNECTION_PROPERTY_SIGNATURE},result{reinterpret_cast<UINT_PTR>(sentinel.data()),0,BUFFER_VALID,APO_CONNECTION_PROPERTY_SIGNATURE};auto bad=&invalid;auto resultPtr=&result;rt->APOProcess(1,&bad,1,&resultPtr);++expectedErrors;Check(result.u32BufferFlags==BUFFER_INVALID&&result.u32ValidFrameCount==0&&std::all_of(sentinel.begin(),sentinel.end(),[](float n){return n==54321.f;}),"Oversized packet rejection");++negativeCases;Ok(config->UnlockForProcess());Check(config->UnlockForProcess()==APOERR_ALREADY_UNLOCKED,"Double unlock");Ok(apo->Reset());
   }
   Check(instances()==0,"Object cleanup");Ok(close());Check(configure(log.c_str(),FALSE,nullptr)==HRESULT_FROM_WIN32(ERROR_FILE_EXISTS),"Existing log rejected");
   std::ifstream file(log,std::ios::binary);std::vector<BootstrapEvent> events;BootstrapEvent event{};while(file.read(reinterpret_cast<char*>(&event),sizeof(event)))events.push_back(event);Check(events.size()==24,"Bounded event count");bool initialized=false,hashed=false;for(size_t i=0;i<events.size();i++){Check(events[i].size==sizeof(event)&&events[i].sequence==i+1&&events[i].pid==GetCurrentProcessId(),"Event header");if(events[i].phase==2){Check(events[i].endpoint==TestEndpoint,"Endpoint GUID supplied");initialized=true;}if(events[i].cpuPresent&&events[i].hashPresent)hashed=true;}Check(initialized&&(!mode||hashed),"Preload event hash");
  }
  auto boundedLog=directory/L"bounded.events";Ok(configure(boundedLog.c_str(),FALSE,nullptr));for(unsigned n=0;n<32;n++){ComPtr<IAudioProcessingObject> a;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&a)));APOInitBaseStruct init{sizeof(init),BootstrapClass};Ok(a->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init)));}Ok(close());Check(std::filesystem::file_size(boundedLog)==64*sizeof(BootstrapEvent),"Event capacity bound");
  auto discoveryLog=directory/L"discovery.events";Ok(configure(discoveryLog.c_str(),TRUE,argv[2]));{ComPtr<IAudioProcessingObject> a;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&a)));APOInitSystemEffects2 init{};init.APOInit={sizeof(init),BootstrapClass};init.InitializeForDiscoveryOnly=TRUE;Ok(a->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init)));Check(GetModuleHandleW(L"HrtfDspCpu.dll")==nullptr,"Discovery must not preload");}Ok(close());
  auto missingEndpointLog=directory/L"missing_endpoint.events";Ok(configure(missingEndpointLog.c_str(),TRUE,argv[2]));{ComPtr<IAudioProcessingObject> a;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&a)));APOInitBaseStruct init{sizeof(init),BootstrapClass};Check(a->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init))==E_INVALIDARG,"Preload requires supplied endpoint GUID");Check(GetModuleHandleW(L"HrtfDspCpu.dll")==nullptr,"Unknown endpoint must not preload");}Ok(close());
  auto invalidExportsLog=directory/L"invalid_exports.events";Ok(configure(invalidExportsLog.c_str(),TRUE,library.c_str()));{ComPtr<IAudioProcessingObject> a;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&a)));ComPtr<Properties> props;props.Attach(new Properties);APOInitSystemEffects init{};init.APOInit={sizeof(init),BootstrapClass};init.pAPOEndpointProperties=props.Get();Check(a->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init))==E_NOINTERFACE,"Missing CPU exports rejected");}Ok(close());
  // This path never calls ConfigureBootstrap before Initialize.
  for(unsigned test=0;test<13;test++){
   auto path=directory/(L"properties_"+std::to_wstring(test)+L".events");
   {ComPtr<Properties> props;props.Attach(new Properties);props->bridge=test!=0;props->logPath=path;props->factoryPath=argv[2];props->sha256=L"4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9";props->enable=true;
    HRESULT expected=S_OK;if(test==1){props->malformed=1;expected=E_INVALIDARG;}if(test==2){props->logPath=L"relative.events";expected=E_INVALIDARG;}if(test==3){props->factoryPath=L"relative.dll";expected=E_INVALIDARG;}if(test==4){props->malformed=2;expected=E_INVALIDARG;}if(test==5){props->sha256=L"not-a-hash";expected=E_INVALIDARG;}if(test==6){props->sha256=std::wstring(64,L'0');expected=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);}if(test==9){props->enable=false;props->factoryPath.clear();props->sha256.clear();}if(test==10){props->sha256=std::wstring(64,L'z');expected=E_INVALIDARG;}
    if(test==11){props->badEndpoint=true;expected=E_INVALIDARG;}if(test==12){props->enable=false;props->sha256=L"bad";expected=E_INVALIDARG;}ComPtr<IAudioProcessingObject> a;Ok(factory->CreateInstance(nullptr,IID_PPV_ARGS(&a)));APOInitSystemEffects2 init{};init.APOInit={sizeof(init),BootstrapClass};init.pAPOEndpointProperties=props.Get();init.pAPOSystemEffectsProperties=props.Get();init.InitializeForDiscoveryOnly=test==7;Check(a->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init))==expected,"Initialization property result");Check((GetModuleHandleW(L"HrtfDspCpu.dll")!=nullptr)==(test==8),"Property preload selection");if(test!=7&&test!=8&&test!=9&&test!=11)Check(!std::filesystem::exists(path),"Rejected/absent properties write no log");}
   Ok(close());
  }
  Check(errors()==expectedErrors,"Unexpected process errors");factory.Reset();Check(instances()==0&&unload()==S_OK,"Final unload state");FreeLibrary(module);
  std::ofstream result(directory/L"result.json");result<<"{\"passed\":true,\"exact_PCM_silent_inplace_frame_cases\":"<<exactCases<<",\"oversized_frame_rejections\":"<<negativeCases<<",\"inert_default_no_preload\":true,\"explicit_preload_and_module_hash_recorded\":true,\"synthetic_endpoint_GUID_preserved\":true,\"live_instances_after_release\":0,\"scope\":\"Owned COM host only; synthetic initialization and media types, no endpoint registration, system admission, audio I/O or actual host mapping.\"}\n";Check(bool(result),"Result file");std::cout<<"PASS bootstrap cases="<<exactCases<<"\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
