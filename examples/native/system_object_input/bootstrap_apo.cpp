#include "bootstrap_api.h"
#include <bcrypt.h>
#include <filesystem>
#include <mutex>
#include <new>
#include <cstring>
#include <cmath>
#include <algorithm>

static std::mutex settingsMutex;
static HANDLE logFile=INVALID_HANDLE_VALUE;
static HMODULE preloadedCpu{};
static WCHAR preloadPath[1024]{};
static BYTE approvedHash[32]{};
static bool preloadEnabled{};
static ULONG instances{},factories{},serverLocks{},calls{},errors{},eventCount{};
static bool configured{};
static HRESULT ConfigureSettings(const wchar_t*,bool,const wchar_t*,const BYTE*,bool);
static constexpr GUID FloatFormat={3,0,0x10,{0x80,0,0,0xaa,0,0x38,0x9b,0x71}};

static bool FileHash(const wchar_t* path,BYTE* hash) noexcept {
 HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return false;
 BCRYPT_ALG_HANDLE algorithm{};BCRYPT_HASH_HANDLE state{};
 bool ok=BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
 if(ok)ok=BCryptCreateHash(algorithm,&state,nullptr,0,nullptr,0,0)>=0;
 BYTE block[16384];DWORD size{};
 while(ok){if(!ReadFile(file,block,sizeof(block),&size,nullptr)){ok=false;break;}if(!size)break;ok=BCryptHashData(state,block,size,0)>=0;}
 if(ok)ok=BCryptFinishHash(state,hash,32,0)>=0;
 if(state)BCryptDestroyHash(state);if(algorithm)BCryptCloseAlgorithmProvider(algorithm,0);CloseHandle(file);return ok;
}
static HRESULT ConfigureFromProperties(IPropertyStore* store){
 if(!store)return S_FALSE;
 WCHAR log[1024]{},cpu[1024]{},digest[65]{};bool enable=false,any=false;
 const PROPERTYKEY keys[]={BootstrapLogPathKey,BootstrapFactoryPathKey,BootstrapPreloadKey,BootstrapFactorySha256Key};
 for(unsigned i=0;i<4;i++){
  PROPVARIANT value{};HRESULT hr=store->GetValue(keys[i],&value);
  if(hr==HRESULT_FROM_WIN32(ERROR_NOT_FOUND)||hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)){PropVariantClear(&value);continue;}
  if(FAILED(hr)){PropVariantClear(&value);return hr;}
  if(value.vt==VT_EMPTY){PropVariantClear(&value);continue;}any=true;
  if(i==2){if(value.vt!=VT_BOOL||(value.boolVal!=VARIANT_TRUE&&value.boolVal!=VARIANT_FALSE)){PropVariantClear(&value);return E_INVALIDARG;}enable=value.boolVal==VARIANT_TRUE;}
  else{const size_t limit=i==3?65:1024;if(value.vt!=VT_LPWSTR||!value.pwszVal||!*value.pwszVal||wcsnlen_s(value.pwszVal,limit)>=limit){PropVariantClear(&value);return E_INVALIDARG;}wcscpy_s(i==0?log:i==1?cpu:digest,limit,value.pwszVal);}
  PropVariantClear(&value);
 }
 if(!any)return S_FALSE;
 try{if(*cpu&&!std::filesystem::path(cpu).is_absolute())return E_INVALIDARG;}catch(...){return E_INVALIDARG;}
 BYTE hash[32]{};if(enable||*digest){if((enable&&!*cpu)||wcslen(digest)!=64)return E_INVALIDARG;for(unsigned i=0;i<64;i++){wchar_t digit=digest[i];unsigned value=digit>=L'0'&&digit<=L'9'?digit-L'0':digit>=L'a'&&digit<=L'f'?digit-L'a'+10:digit>=L'A'&&digit<=L'F'?digit-L'A'+10:256;if(value>15)return E_INVALIDARG;hash[i/2]=static_cast<BYTE>((hash[i/2]<<4)|value);}}
 return ConfigureSettings(log,enable,cpu,enable?hash:nullptr,true);
}
static void Event(UINT32 phase,REFGUID endpoint,HRESULT status) noexcept {
 std::lock_guard<std::mutex> lock(settingsMutex);
 if(logFile==INVALID_HANDLE_VALUE||eventCount>=64)return;
 BootstrapEvent record{};record.size=sizeof(record);record.sequence=++eventCount;record.phase=phase;record.pid=GetCurrentProcessId();record.tid=GetCurrentThreadId();record.status=status;record.endpoint=endpoint;
 LARGE_INTEGER now{};QueryPerformanceCounter(&now);record.qpc=now.QuadPart;
 if(HMODULE cpu=GetModuleHandleW(L"HrtfDspCpu.dll")){record.cpuPresent=1;DWORD n=GetModuleFileNameW(cpu,record.cpuPath,1024);if(n&&n<1024)record.hashPresent=FileHash(record.cpuPath,record.cpuSha256);}
 DWORD written{};if(!WriteFile(logFile,&record,sizeof(record),&written,nullptr)||written!=sizeof(record))InterlockedIncrement(&errors);
}
static HRESULT Format(IAudioMediaType* type,UNCOMPRESSEDAUDIOFORMAT& result) noexcept {
 if(!type)return E_POINTER;HRESULT hr=type->GetUncompressedAudioFormat(&result);if(FAILED(hr))return hr;
 if(result.guidFormatType!=FloatFormat||result.dwSamplesPerFrame<1||result.dwSamplesPerFrame>64||result.dwBytesPerSampleContainer!=4||result.dwValidBitsPerSample!=32||!std::isfinite(result.fFramesPerSecond)||result.fFramesPerSecond<10||result.fFramesPerSecond>384000)return APOERR_FORMAT_NOT_SUPPORTED;
 return S_OK;
}
static bool Same(const UNCOMPRESSEDAUDIOFORMAT& a,const UNCOMPRESSEDAUDIOFORMAT& b) noexcept{return a.dwSamplesPerFrame==b.dwSamplesPerFrame&&a.fFramesPerSecond==b.fFramesPerSecond&&a.dwChannelMask==b.dwChannelMask;}

class Bootstrap final:public IAudioProcessingObject,public IAudioProcessingObjectConfiguration,public IAudioProcessingObjectRT,public IAudioSystemEffects {
 ULONG references{1};bool initialized{},locked{};UINT32 channels{},maxFrames{};GUID endpoint{};
 HRESULT Support(IAudioMediaType* opposite,IAudioMediaType* requested,IAudioMediaType** out){
  if(!out)return E_POINTER;*out=nullptr;UNCOMPRESSEDAUDIOFORMAT a{},b{};HRESULT hr=Format(requested,a);if(FAILED(hr))return hr;if(opposite){hr=Format(opposite,b);if(FAILED(hr)||!Same(a,b))return APOERR_FORMAT_NOT_SUPPORTED;}requested->AddRef();*out=requested;return S_OK;
 }
public:
 Bootstrap(){InterlockedIncrement(&instances);Event(1,endpoint,S_OK);}
 ~Bootstrap(){Event(5,endpoint,S_OK);InterlockedDecrement(&instances);}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(iid==IID_IUnknown||iid==__uuidof(IAudioProcessingObject))*out=static_cast<IAudioProcessingObject*>(this);else if(iid==__uuidof(IAudioProcessingObjectConfiguration))*out=static_cast<IAudioProcessingObjectConfiguration*>(this);else if(iid==__uuidof(IAudioProcessingObjectRT))*out=static_cast<IAudioProcessingObjectRT*>(this);else if(iid==__uuidof(IAudioSystemEffects))*out=static_cast<IAudioSystemEffects*>(this);else return E_NOINTERFACE;AddRef();return S_OK;}
 ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&references);}
 ULONG STDMETHODCALLTYPE Release()override{ULONG n=InterlockedDecrement(&references);if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE Reset()override{return locked?APOERR_APO_LOCKED:S_OK;}
 HRESULT STDMETHODCALLTYPE GetLatency(HNSTIME* out)override{if(!out)return E_POINTER;*out=0;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetRegistrationProperties(APO_REG_PROPERTIES** out)override{
  if(!out)return E_POINTER;*out=nullptr;constexpr size_t size=sizeof(APO_REG_PROPERTIES)+3*sizeof(IID);auto value=static_cast<APO_REG_PROPERTIES*>(CoTaskMemAlloc(size));if(!value)return E_OUTOFMEMORY;std::memset(value,0,size);value->clsid=BootstrapClass;value->Flags=static_cast<APO_FLAG>(APO_FLAG_DEFAULT|APO_FLAG_INPLACE);wcscpy_s(value->szFriendlyName,L"Spatial renderer bootstrap probe");value->u32MajorVersion=1;value->u32MinInputConnections=value->u32MaxInputConnections=value->u32MinOutputConnections=value->u32MaxOutputConnections=1;value->u32MaxInstances=64;value->u32NumAPOInterfaces=4;value->iidAPOInterfaceList[0]=__uuidof(IAudioProcessingObject);value->iidAPOInterfaceList[1]=__uuidof(IAudioProcessingObjectConfiguration);value->iidAPOInterfaceList[2]=__uuidof(IAudioProcessingObjectRT);value->iidAPOInterfaceList[3]=__uuidof(IAudioSystemEffects);*out=value;return S_OK;
 }
 HRESULT STDMETHODCALLTYPE Initialize(UINT32 size,BYTE* data)override{
  if(initialized)return APOERR_ALREADY_INITIALIZED;
  if(!data||size<sizeof(APOInitBaseStruct))return E_INVALIDARG;
  auto base=reinterpret_cast<APOInitBaseStruct*>(data);if(base->cbSize!=size||base->clsid!=BootstrapClass)return APOERR_INVALID_APO_CLSID;
  if(size!=sizeof(APOInitBaseStruct)&&size!=sizeof(APOInitSystemEffects)&&size!=sizeof(APOInitSystemEffects2))return E_INVALIDARG;
  if(size>=sizeof(APOInitSystemEffects)){auto init=reinterpret_cast<APOInitSystemEffects*>(data);if(init->pAPOEndpointProperties){PROPVARIANT value{};HRESULT hr=init->pAPOEndpointProperties->GetValue(EndpointGuidKey,&value);if(SUCCEEDED(hr)&&value.vt==VT_LPWSTR&&value.pwszVal){GUID parsed{};if(SUCCEEDED(CLSIDFromString(value.pwszVal,&parsed)))endpoint=parsed;}PropVariantClear(&value);}}
  if(size>=sizeof(APOInitSystemEffects)){bool active{};{std::lock_guard<std::mutex> lock(settingsMutex);active=configured;}if(!active){HRESULT hr=ConfigureFromProperties(reinterpret_cast<APOInitSystemEffects*>(data)->pAPOSystemEffectsProperties);if(FAILED(hr))return hr;}}
  Event(2,endpoint,S_OK);HRESULT hr=S_OK;const bool discovery=size==sizeof(APOInitSystemEffects2)&&reinterpret_cast<APOInitSystemEffects2*>(data)->InitializeForDiscoveryOnly;
  {std::lock_guard<std::mutex> lock(settingsMutex);if(configured&&preloadEnabled&&!preloadedCpu&&!discovery){BYTE current[32]{};if(endpoint==GUID_NULL)hr=E_INVALIDARG;else if(GetModuleHandleW(L"HrtfDspCpu.dll"))hr=HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);else if(!FileHash(preloadPath,current)||std::memcmp(current,approvedHash,32))hr=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);else{HMODULE module=LoadLibraryExW(preloadPath,nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);if(!module)hr=HRESULT_FROM_WIN32(GetLastError());else if(!GetProcAddress(module,"CreateHrtfDspFactory")||!GetProcAddress(module,"CreateHrtfDataHelper")){FreeLibrary(module);hr=E_NOINTERFACE;}else{auto prepare=reinterpret_cast<HRESULT(WINAPI*)()>(GetProcAddress(module,"InitializeCpuFactory"));if(prepare)hr=prepare();if(FAILED(hr))FreeLibrary(module);else preloadedCpu=module;}}}}
  Event(6,endpoint,hr);if(FAILED(hr))return hr;initialized=true;return S_OK;
 }
 HRESULT STDMETHODCALLTYPE IsInputFormatSupported(IAudioMediaType* o,IAudioMediaType* r,IAudioMediaType** out)override{return Support(o,r,out);}
 HRESULT STDMETHODCALLTYPE IsOutputFormatSupported(IAudioMediaType* o,IAudioMediaType* r,IAudioMediaType** out)override{return Support(o,r,out);}
 HRESULT STDMETHODCALLTYPE GetInputChannelCount(UINT32* out)override{if(!out)return E_POINTER;*out=channels;return initialized?S_OK:APOERR_NOT_INITIALIZED;}
 HRESULT STDMETHODCALLTYPE LockForProcess(UINT32 ni,APO_CONNECTION_DESCRIPTOR** input,UINT32 no,APO_CONNECTION_DESCRIPTOR** output)override{
  if(!initialized)return APOERR_NOT_INITIALIZED;if(locked)return APOERR_APO_LOCKED;if(ni!=1||no!=1)return APOERR_NUM_CONNECTIONS_INVALID;if(!input||!output||!input[0]||!output[0])return E_POINTER;
  auto& a=*input[0];auto& b=*output[0];if(a.u32Signature!=APO_CONNECTION_DESCRIPTOR_SIGNATURE||b.u32Signature!=APO_CONNECTION_DESCRIPTOR_SIGNATURE)return E_INVALIDARG;
  UNCOMPRESSEDAUDIOFORMAT af{},bf{};HRESULT hr=Format(a.pFormat,af);if(FAILED(hr))return hr;hr=Format(b.pFormat,bf);if(FAILED(hr)||!Same(af,bf))return APOERR_INVALID_CONNECTION_FORMAT;if(!a.u32MaxFrameCount||a.u32MaxFrameCount>8192||b.u32MaxFrameCount<a.u32MaxFrameCount)return APOERR_INVALID_OUTPUT_MAXFRAMECOUNT;
  channels=af.dwSamplesPerFrame;maxFrames=a.u32MaxFrameCount;locked=true;Event(3,endpoint,S_OK);return S_OK;
 }
 HRESULT STDMETHODCALLTYPE UnlockForProcess()override{if(!locked)return APOERR_ALREADY_UNLOCKED;locked=false;Event(4,endpoint,S_OK);return S_OK;}
 void STDMETHODCALLTYPE APOProcess(UINT32 ni,APO_CONNECTION_PROPERTY** input,UINT32 no,APO_CONNECTION_PROPERTY** output)override{
  InterlockedIncrement(&calls);if(no!=1||!output||!output[0]){InterlockedIncrement(&errors);return;}APO_CONNECTION_PROPERTY saved{};if(ni==1&&input&&input[0])saved=*input[0];auto& out=*output[0];out.u32ValidFrameCount=0;out.u32BufferFlags=BUFFER_INVALID;
  if(!locked||ni!=1||!input||!input[0]||out.u32Signature!=APO_CONNECTION_PROPERTY_SIGNATURE||input[0]->u32Signature!=APO_CONNECTION_PROPERTY_SIGNATURE||saved.u32ValidFrameCount>maxFrames){InterlockedIncrement(&errors);return;}
  const auto& in=saved;const UINT32 frames=in.u32ValidFrameCount;
  if(in.u32BufferFlags==BUFFER_INVALID)return;
  if(in.u32BufferFlags!=BUFFER_VALID&&in.u32BufferFlags!=BUFFER_SILENT){InterlockedIncrement(&errors);return;}
  if(frames&&!out.pBuffer){InterlockedIncrement(&errors);return;}
  const size_t bytes=size_t(frames)*channels*sizeof(float);
  if(in.u32BufferFlags==BUFFER_SILENT){if(bytes)std::memset(reinterpret_cast<void*>(out.pBuffer),0,bytes);}else{if(frames&&!in.pBuffer){InterlockedIncrement(&errors);return;}if(bytes)std::memmove(reinterpret_cast<void*>(out.pBuffer),reinterpret_cast<const void*>(in.pBuffer),bytes);}
  out.u32ValidFrameCount=frames;out.u32BufferFlags=in.u32BufferFlags;
 }
 UINT32 STDMETHODCALLTYPE CalcInputFrames(UINT32 n)override{return n;}
 UINT32 STDMETHODCALLTYPE CalcOutputFrames(UINT32 n)override{return n;}
};
class Factory final:public IClassFactory {
 ULONG references{1};
public:
 Factory(){InterlockedIncrement(&factories);}
 ~Factory(){InterlockedDecrement(&factories);}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(iid!=IID_IUnknown&&iid!=IID_IClassFactory)return E_NOINTERFACE;*out=this;AddRef();return S_OK;}
 ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&references);}
 ULONG STDMETHODCALLTYPE Release()override{ULONG n=InterlockedDecrement(&references);if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer,REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(outer)return CLASS_E_NOAGGREGATION;Bootstrap* value{};try{value=new Bootstrap;}catch(const std::bad_alloc&){return E_OUTOFMEMORY;}HRESULT hr=value->QueryInterface(iid,out);value->Release();return hr;}
 HRESULT STDMETHODCALLTYPE LockServer(BOOL value)override{if(value)InterlockedIncrement(&serverLocks);else if(serverLocks)InterlockedDecrement(&serverLocks);return S_OK;}
};
static HRESULT ConfigureSettings(const wchar_t* path,bool enable,const wchar_t* cpu,const BYTE* expected,bool fromInitialize){
 if(!path||!*path)return E_INVALIDARG;try{if(!std::filesystem::path(path).is_absolute()||(enable&&(!cpu||!*cpu||!std::filesystem::path(cpu).is_absolute()||wcslen(cpu)>=1024)))return E_INVALIDARG;}catch(...){return E_INVALIDARG;}
 std::lock_guard<std::mutex> lock(settingsMutex);if(instances>(fromInitialize?1U:0U)||configured)return E_UNEXPECTED;
 BYTE hash[32]{};if(enable){if(!FileHash(cpu,hash))return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);if(expected&&std::memcmp(hash,expected,32))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);}
 HANDLE file=CreateFileW(path,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return HRESULT_FROM_WIN32(GetLastError());logFile=file;configured=true;preloadEnabled=enable;if(preloadEnabled){wcscpy_s(preloadPath,cpu);std::memcpy(approvedHash,hash,32);}return S_OK;
}
extern "C" __declspec(dllexport) HRESULT WINAPI ConfigureBootstrap(const wchar_t* path,BOOL enable,const wchar_t* cpu){return ConfigureSettings(path,enable!=FALSE,cpu,nullptr,false);}
extern "C" __declspec(dllexport) HRESULT WINAPI CloseBootstrap(){std::lock_guard<std::mutex> lock(settingsMutex);if(instances)return E_UNEXPECTED;if(logFile!=INVALID_HANDLE_VALUE){CloseHandle(logFile);logFile=INVALID_HANDLE_VALUE;}configured=false;preloadEnabled=false;preloadPath[0]=0;eventCount=0;if(preloadedCpu){FreeLibrary(preloadedCpu);preloadedCpu=nullptr;}return S_OK;}
extern "C" __declspec(dllexport) ULONG WINAPI GetBootstrapCalls(){return calls;}
extern "C" __declspec(dllexport) ULONG WINAPI GetBootstrapErrors(){return errors;}
extern "C" __declspec(dllexport) ULONG WINAPI GetBootstrapInstances(){return instances;}
STDAPI DllGetClassObject(REFCLSID cls,REFIID iid,void** out){if(!out)return E_POINTER;*out=nullptr;if(cls!=BootstrapClass)return CLASS_E_CLASSNOTAVAILABLE;auto factory=new(std::nothrow) Factory;if(!factory)return E_OUTOFMEMORY;HRESULT hr=factory->QueryInterface(iid,out);factory->Release();return hr;}
STDAPI DllCanUnloadNow(){return instances||factories||serverLocks||configured?S_FALSE:S_OK;}
