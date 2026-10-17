#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioenginebaseapo.h>
#include <audiomediatype.h>
#include <propsys.h>
#include <propvarutil.h>
#include <functiondiscoverykeys_devpkey.h>
#include <ks.h>
#include <ksmedia.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "pinned_binary.h"
#include "fixed_quantum.h"

using Microsoft::WRL::ComPtr;
namespace fs=std::filesystem;
static void check(HRESULT hr,const char* action){if(FAILED(hr)){std::cerr<<action<<" 0x"<<std::hex<<unsigned(hr)<<std::dec<<'\n';throw std::runtime_error(action);}}
static unsigned number(const wchar_t* text,unsigned minimum,unsigned maximum){std::wstring value=text;size_t used=0;auto parsed=std::stoull(value,&used);if(used!=value.size()||parsed<minimum||parsed>maximum)throw std::runtime_error("Invalid numeric parameter");return static_cast<unsigned>(parsed);}
static std::wstring configPath;
static unsigned configReads=0,blockedRegistryOpens=0;
static HKEY fakeKey=reinterpret_cast<HKEY>(static_cast<uintptr_t>(0x4e41544956454150ull));
class FloatMediaType final:public IAudioMediaType {
    LONG references=1;WAVEFORMATEXTENSIBLE wave;
public:
    explicit FloatMediaType(const WAVEFORMATEXTENSIBLE& value):wave(value){}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** object)override{if(!object)return E_POINTER;*object=nullptr;if(iid!=__uuidof(IUnknown)&&iid!=__uuidof(IAudioMediaType))return E_NOINTERFACE;*object=static_cast<IAudioMediaType*>(this);AddRef();return S_OK;}
    ULONG STDMETHODCALLTYPE AddRef()override{return static_cast<ULONG>(InterlockedIncrement(&references));}
    ULONG STDMETHODCALLTYPE Release()override{auto count=InterlockedDecrement(&references);if(!count)delete this;return static_cast<ULONG>(count);}
    HRESULT STDMETHODCALLTYPE IsCompressedFormat(BOOL* value)override{if(!value)return E_POINTER;*value=FALSE;return S_OK;}
    HRESULT STDMETHODCALLTYPE IsEqual(IAudioMediaType* other,DWORD* flags)override{if(!other||!flags)return E_POINTER;*flags=0;UNCOMPRESSEDAUDIOFORMAT a{},b{};GetUncompressedAudioFormat(&a);HRESULT hr=other->GetUncompressedAudioFormat(&b);if(FAILED(hr))return hr;if(a.guidFormatType==b.guidFormatType)*flags|=AUDIOMEDIATYPE_EQUAL_FORMAT_TYPES;if(a.dwSamplesPerFrame==b.dwSamplesPerFrame&&a.dwBytesPerSampleContainer==b.dwBytesPerSampleContainer&&a.dwValidBitsPerSample==b.dwValidBitsPerSample&&a.fFramesPerSecond==b.fFramesPerSecond&&a.dwChannelMask==b.dwChannelMask)*flags|=AUDIOMEDIATYPE_EQUAL_FORMAT_DATA;*flags|=AUDIOMEDIATYPE_EQUAL_FORMAT_USER_DATA;return *flags==14?S_OK:S_FALSE;}
    const WAVEFORMATEX* STDMETHODCALLTYPE GetAudioFormat()override{return &wave.Format;}
    HRESULT STDMETHODCALLTYPE GetUncompressedAudioFormat(UNCOMPRESSEDAUDIOFORMAT* value)override{if(!value)return E_POINTER;*value={wave.SubFormat,wave.Format.nChannels,4,32,static_cast<float>(wave.Format.nSamplesPerSec),wave.dwChannelMask};return S_OK;}
};

// These imports are replaced only in our own loaded DLL image. No registry write
// or system registration is performed. Every unrelated registry open fails closed.
static LSTATUS WINAPI openKey(HKEY root,LPCWSTR path,DWORD,REGSAM,PHKEY result){
    if(!result)return ERROR_INVALID_PARAMETER;
    if(root==HKEY_LOCAL_MACHINE&&path&&_wcsicmp(path,L"SOFTWARE\\EqualizerAPO")==0){*result=fakeKey;return ERROR_SUCCESS;}
    *result=nullptr;++blockedRegistryOpens;return ERROR_FILE_NOT_FOUND;
}
static LSTATUS WINAPI queryValue(HKEY key,LPCWSTR name,LPDWORD,LPDWORD type,LPBYTE value,LPDWORD bytes){
    if(key!=fakeKey||!name||!bytes)return ERROR_FILE_NOT_FOUND;
    std::wstring answer;
    if(_wcsicmp(name,L"ConfigPath")==0){answer=configPath;++configReads;}
    else if(_wcsicmp(name,L"EnableTrace")==0)answer=L"false";
    else return ERROR_FILE_NOT_FOUND;
    DWORD required=static_cast<DWORD>((answer.size()+1)*sizeof(wchar_t));
    if(type)*type=REG_SZ;
    if(!value){*bytes=required;return ERROR_SUCCESS;}
    if(*bytes<required){*bytes=required;return ERROR_MORE_DATA;}
    std::memcpy(value,answer.c_str(),required);*bytes=required;return ERROR_SUCCESS;
}
static LSTATUS WINAPI closeKey(HKEY key){return key==fakeKey?ERROR_SUCCESS:ERROR_INVALID_HANDLE;}
static DWORD WINAPI tempPath(DWORD size,LPWSTR output){
    auto path=configPath+L"\\";
    if(size<=path.size())return static_cast<DWORD>(path.size()+1);
    std::memcpy(output,path.c_str(),(path.size()+1)*sizeof(wchar_t));return static_cast<DWORD>(path.size());
}
class Imports {
    struct Entry{void** slot;void* original;};std::vector<Entry> entries;
    void restore()noexcept{for(auto it=entries.rbegin();it!=entries.rend();++it){DWORD previous{};if(VirtualProtect(it->slot,sizeof(void*),PAGE_READWRITE,&previous)){InterlockedExchangePointer(it->slot,it->original);DWORD ignored{};VirtualProtect(it->slot,sizeof(void*),previous,&ignored);}}entries.clear();}
public:
    explicit Imports(HMODULE module){try{
        entries.reserve(16);
        auto base=reinterpret_cast<BYTE*>(module);auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
        auto directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        if(!directory.VirtualAddress)throw std::runtime_error("No DLL imports");
        auto descriptor=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+directory.VirtualAddress);
        std::array<bool,4> found{};
        for(;descriptor->Name;descriptor++){
            if(!descriptor->OriginalFirstThunk)continue;
            auto names=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+descriptor->OriginalFirstThunk);
            auto values=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+descriptor->FirstThunk);
            for(;names->u1.AddressOfData;names++,values++){
                if(IMAGE_SNAP_BY_ORDINAL64(names->u1.Ordinal))continue;
                auto name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData)->Name;
                const char* expected[]={"RegOpenKeyExW","RegQueryValueExW","RegCloseKey","GetTempPathW"};
                void* replacement[]={reinterpret_cast<void*>(&openKey),reinterpret_cast<void*>(&queryValue),reinterpret_cast<void*>(&closeKey),reinterpret_cast<void*>(&tempPath)};
                for(size_t i=0;i<4;i++)if(std::strcmp(name,expected[i])==0){
                    auto slot=reinterpret_cast<void**>(&values->u1.Function);DWORD previous{};
                    entries.push_back({slot,*slot});
                    if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&previous))throw std::runtime_error("Import protection");
                    InterlockedExchangePointer(slot,replacement[i]);DWORD ignored{};
                    if(!VirtualProtect(slot,sizeof(void*),previous,&ignored))throw std::runtime_error("Import protection restore");found[i]=true;
                }
            }
        }
        if(!std::all_of(found.begin(),found.end(),[](bool value){return value;}))throw std::runtime_error("Expected research imports missing");
    }catch(...){restore();throw;}}
    ~Imports(){restore();}
};

int wmain(int argc,wchar_t** argv){
    if(argc!=9){std::cerr<<"Usage: eqapo_host DLL config-directory input.f32 output.f32 metrics.json channels block-frames valid|silent-tail|vary|vary-small-first|adapted|query-downmix\n";return 2;}
    try{
        unsigned channels=number(argv[6],1,32),block=number(argv[7],16,2048);
        const std::wstring mode=argv[8];
        if(mode!=L"valid"&&mode!=L"silent-tail"&&mode!=L"vary"&&mode!=L"vary-small-first"&&mode!=L"adapted"&&mode!=L"query-downmix")throw std::runtime_error("Invalid parameters");
        fs::path dll=fs::canonical(argv[1]);configPath=fs::canonical(argv[2]).wstring();
        if(configPath.size()>253)throw std::runtime_error("Research configuration path exceeds the vendor log buffer");
        RequirePinnedBinary(dll.c_str(),"fb73fc8b94f271464ae4f83918bb3cb3243c4f50a8035c07fa538ad16ad66aa4");
        const fs::path out=fs::absolute(argv[4]).lexically_normal(),metrics=fs::absolute(argv[5]).lexically_normal();
        if(fs::exists(out)||fs::exists(metrics)||CompareStringOrdinal(out.c_str(),-1,metrics.c_str(),-1,TRUE)==CSTR_EQUAL||!fs::exists(fs::path(configPath)/L"config.txt"))throw std::runtime_error("Use separate new outputs and a test configuration");
        std::ifstream input(fs::path(argv[3]),std::ios::binary|std::ios::ate);if(!input)throw std::runtime_error("Input missing");
        auto byteCount=input.tellg();if(byteCount<=0||byteCount>512ll*1024*1024||byteCount%static_cast<std::streamoff>(channels*sizeof(float)))throw std::runtime_error("Invalid PCM extent");
        input.seekg(0);std::vector<float> source(static_cast<size_t>(byteCount)/sizeof(float));input.read(reinterpret_cast<char*>(source.data()),byteCount);
        if(!input||!std::all_of(source.begin(),source.end(),[](float value){return std::isfinite(value);}))throw std::runtime_error("Invalid PCM data");
        size_t frames=source.size()/channels;if(frames%block)throw std::runtime_error("Input must have whole maximum blocks");
        check(CoInitializeEx(nullptr,COINIT_MULTITHREADED),"CoInitialize");
        auto module=LoadLibraryExW(dll.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!module)throw std::runtime_error("Load EqualizerAPO DLL");
        Imports imports(module);
        auto getFactory=reinterpret_cast<HRESULT(WINAPI*)(REFCLSID,REFIID,void**)>(GetProcAddress(module,"DllGetClassObject"));
        if(!getFactory)throw std::runtime_error("Factory export missing");
        const GUID clsid={0xec1cc9ce,0xfaed,0x4822,{0x82,0x8a,0x82,0xa8,0x1a,0x6f,0x01,0x8f}};
        ComPtr<IClassFactory> factory;check(getFactory(clsid,IID_PPV_ARGS(&factory)),"DllGetClassObject");
        ComPtr<IAudioProcessingObject> apo;check(factory->CreateInstance(nullptr,IID_PPV_ARGS(&apo)),"CreateInstance");
        ComPtr<IAudioProcessingObjectConfiguration> configuration;check(apo.As(&configuration),"APO configuration");
        ComPtr<IAudioProcessingObjectRT> rt;check(apo.As(&rt),"APO realtime");
        auto rtMethods=*reinterpret_cast<uintptr_t**>(rt.Get());
        std::cout<<"APO_PROCESS_RVA 0x"<<std::hex<<(rtMethods[3]-reinterpret_cast<uintptr_t>(module))<<std::dec<<'\n';
        ComPtr<IPropertyStore> properties;check(PSCreateMemoryPropertyStore(IID_PPV_ARGS(&properties)),"Memory property store");
        PROPVARIANT guid{};check(InitPropVariantFromString(L"{00000000-0000-0000-0000-000000000000}",&guid),"Dummy endpoint string");
        check(properties->SetValue(PKEY_AudioEndpoint_GUID,guid),"Dummy property");PropVariantClear(&guid);
        APOInitSystemEffects init{};init.APOInit.cbSize=sizeof(init);init.APOInit.clsid=clsid;init.pAPOEndpointProperties=properties.Get();init.pAPOSystemEffectsProperties=properties.Get();
        check(apo->Initialize(sizeof(init),reinterpret_cast<BYTE*>(&init)),"APO Initialize");
        WAVEFORMATEXTENSIBLE wave{};wave.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;wave.Format.nChannels=static_cast<WORD>(channels);
        wave.Format.nSamplesPerSec=48000;wave.Format.wBitsPerSample=32;wave.Format.nBlockAlign=static_cast<WORD>(channels*4);wave.Format.nAvgBytesPerSec=48000*wave.Format.nBlockAlign;wave.Format.cbSize=22;
        wave.Samples.wValidBitsPerSample=32;wave.dwChannelMask=0;wave.SubFormat=KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        ComPtr<IAudioMediaType> media;media.Attach(new FloatMediaType(wave));
        if(mode==L"query-downmix"){
            wave.Format.nChannels=2;wave.Format.nBlockAlign=8;wave.Format.nAvgBytesPerSec=384000;
            ComPtr<IAudioMediaType> outputType;outputType.Attach(new FloatMediaType(wave));ComPtr<IAudioMediaType> alternative;
            const HRESULT hr=apo->IsInputFormatSupported(outputType.Get(),media.Get(),&alternative);
            UNCOMPRESSEDAUDIOFORMAT value{};if(alternative)check(alternative->GetUncompressedAudioFormat(&value),"Suggested format");
            std::ofstream record(metrics);record<<"{\"requested_input_channels\":"<<channels<<",\"requested_output_channels\":2,\"hresult\":"<<static_cast<unsigned>(hr)<<",\"suggested_input_channels\":"<<value.dwSamplesPerFrame<<",\"device_stream\":false}\n";
            if(!record)throw std::runtime_error("Write format result");return 0;
        }
        ComPtr<IAudioMediaType> suggested;HRESULT support=apo->IsInputFormatSupported(media.Get(),media.Get(),&suggested);
        if(support!=S_OK)check(FAILED(support)?support:E_FAIL,"Exact float format unsupported");
        constexpr float sentinel=12345.0f;const size_t guard=32,count=static_cast<size_t>(block)*channels;
        std::vector<float> in(count+2*guard,sentinel),buffer(count+2*guard,sentinel),rendered(source.size());
        APO_CONNECTION_DESCRIPTOR inDesc{APO_CONNECTION_BUFFER_TYPE_EXTERNAL,reinterpret_cast<UINT_PTR>(in.data()+guard),block,media.Get(),APO_CONNECTION_DESCRIPTOR_SIGNATURE};
        APO_CONNECTION_DESCRIPTOR outDesc{APO_CONNECTION_BUFFER_TYPE_EXTERNAL,reinterpret_cast<UINT_PTR>(buffer.data()+guard),block,media.Get(),APO_CONNECTION_DESCRIPTOR_SIGNATURE};
        auto inPointer=&inDesc,outPointer=&outDesc;check(configuration->LockForProcess(1,&inPointer,1,&outPointer),"LockForProcess");
        struct Unlock{IAudioProcessingObjectConfiguration* apo;bool active=true;~Unlock(){if(active)apo->UnlockForProcess();}} unlock{configuration.Get()};
        HNSTIME latency{};check(apo->GetLatency(&latency),"GetLatency");
        std::vector<double> timings;timings.reserve(frames/(block/2)+1);size_t index=0,silentBlocks=0,producerPackets=0;unsigned sequence=0;
        auto process=[&](const float* samples,unsigned current,size_t destination){
            std::fill(in.begin(),in.end(),sentinel);std::fill(buffer.begin(),buffer.end(),sentinel);
            std::copy_n(samples,static_cast<size_t>(current)*channels,in.data()+guard);
            bool silent=mode==L"silent-tail"&&std::all_of(in.begin()+guard,in.begin()+guard+static_cast<size_t>(current)*channels,[](float value){return value==0;});
            APO_CONNECTION_PROPERTY a{},b{};a.pBuffer=reinterpret_cast<UINT_PTR>(in.data()+guard);a.u32ValidFrameCount=current;a.u32BufferFlags=silent?BUFFER_SILENT:BUFFER_VALID;a.u32Signature=APO_CONNECTION_PROPERTY_SIGNATURE;
            b.pBuffer=reinterpret_cast<UINT_PTR>(buffer.data()+guard);b.u32ValidFrameCount=0;b.u32BufferFlags=BUFFER_INVALID;b.u32Signature=APO_CONNECTION_PROPERTY_SIGNATURE;
            auto ap=&a,bp=&b;auto start=std::chrono::steady_clock::now();rt->APOProcess(1,&ap,1,&bp);auto end=std::chrono::steady_clock::now();
            timings.push_back(std::chrono::duration<double,std::micro>(end-start).count());
            if(b.u32ValidFrameCount!=current||(b.u32BufferFlags!=BUFFER_VALID&&b.u32BufferFlags!=BUFFER_SILENT))throw std::runtime_error("Output contract mismatch");
            for(const auto* v:{&in,&buffer})if(!std::all_of(v->begin(),v->begin()+guard,[sentinel](float value){return value==sentinel;})||!std::all_of(v->begin()+guard+static_cast<size_t>(current)*channels,v->end(),[sentinel](float value){return value==sentinel;}))throw std::runtime_error("Buffer extent guard changed");
            if(b.u32BufferFlags==BUFFER_SILENT)std::fill_n(rendered.data()+destination*channels,static_cast<size_t>(current)*channels,0.f);
            else std::copy_n(buffer.data()+guard,static_cast<size_t>(current)*channels,rendered.data()+destination*channels);
            if(silent)++silentBlocks;
        };
        if(mode==L"adapted"){
            FixedQuantum adapter(channels,block);size_t outputFrame=0;
            constexpr size_t packets[]={1,47,80,480,1024,13,256,127};
            while(index<frames){size_t size=std::min(packets[producerPackets%8],frames-index);adapter.push(source.data()+index*channels,size,[&](const float* samples,size_t length){process(samples,static_cast<unsigned>(length),outputFrame);outputFrame+=length;});index+=size;++producerPackets;}
            if(adapter.remaining()!=0||outputFrame!=frames)throw std::runtime_error("Fixed quantum did not conserve the padded stream");
        }else while(index<frames){
            unsigned current=block;if(mode==L"vary")current=sequence%2?block/2:block;if(mode==L"vary-small-first")current=sequence%2?block:block/2;current=static_cast<unsigned>(std::min<size_t>(current,frames-index));
            process(source.data()+index*channels,current,index);index+=current;++sequence;
        }
        check(configuration->UnlockForProcess(),"UnlockForProcess");
        unlock.active=false;
        if(!configReads)throw std::runtime_error("Test configuration was never read");
        if(!std::all_of(rendered.begin(),rendered.end(),[](float value){return std::isfinite(value);}))throw std::runtime_error("Nonfinite output");
        std::ofstream output(out,std::ios::binary);output.write(reinterpret_cast<char*>(rendered.data()),static_cast<std::streamsize>(rendered.size()*sizeof(float)));if(!output)throw std::runtime_error("Write PCM");
        std::sort(timings.begin(),timings.end());double total=0;for(double time:timings)total+=time;
        std::ofstream record(metrics);record<<std::setprecision(12)<<"{\"channels\":"<<channels<<",\"frames\":"<<frames<<",\"max_block_frames\":"<<block<<",\"calls\":"<<timings.size()<<",\"mean_us\":"<<total/timings.size()<<",\"p99_us\":"<<timings[static_cast<size_t>(.99*(timings.size()-1))]<<",\"maximum_us\":"<<timings.back()<<",\"reported_latency_100ns\":"<<latency<<",\"config_value_reads\":"<<configReads<<",\"blocked_registry_opens\":"<<blockedRegistryOpens<<",\"silent_flag_blocks\":"<<silentBlocks<<",\"adapter_producer_packets\":"<<producerPackets<<",\"guard_passed\":true,\"device_stream\":false}\n";
        if(!record)throw std::runtime_error("Write metrics");
        // COM objects release before the import guard. The module remains mapped
        // until this short-lived worker exits, avoiding unload with runtime state.
        std::cout<<"APO16_PASS channels="<<channels<<" frames="<<frames<<" calls="<<timings.size()<<" latency_100ns="<<latency<<'\n';return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
