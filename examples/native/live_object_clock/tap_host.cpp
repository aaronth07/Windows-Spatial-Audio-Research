#include "protocol.h"
#include <mmdeviceapi.h>
#include <spatialaudioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;namespace fs=std::filesystem;
thread_local bool trackAllocations=false;
thread_local uint64_t callbackNew=0,callbackDelete=0;
void* operator new(size_t n){if(trackAllocations)callbackNew++;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{if(trackAllocations&&p)callbackDelete++;std::free(p);}
void* operator new[](size_t n){return ::operator new(n);}
void operator delete[](void* p)noexcept{::operator delete(p);}
void operator delete(void* p,size_t)noexcept{::operator delete(p);}
void operator delete[](void* p,size_t)noexcept{::operator delete(p);}
static void require(bool v,const char* text){if(!v)throw std::runtime_error(text);}
static void check(HRESULT hr,const char* text){if(FAILED(hr))throw std::runtime_error(text);}
struct State;struct StreamState;
static std::atomic<uint64_t> nextStreamId{1},nextObjectId{1};
enum Kind{EnumeratorKind,CollectionKind,DeviceKind,ClientKind,StreamKind,ObjectKind,KindCount};
struct Statistics {
    std::array<uint32_t,KindCount> created{},released{};
    uint32_t coCreateIntercepts=0,realCoCreates=0,unwrappedInterfaces=0;
    uint32_t beginCalls=0,endCalls=0,recordsBeforeEnd=0,buffersCleared=0;
    HRESULT captureError=S_OK;
};
class ObjectProxy final:public ISpatialAudioObject {
    LONG references=0;
public:
    StreamState* state=nullptr;ComPtr<ISpatialAudioObject> real;
    uint64_t id=0;uint32_t sourceIndex=0,generation=UINT32_MAX;AudioObjectType type=AudioObjectType_None;
    float position[3]{},volume=1;BYTE* buffer=nullptr;uint32_t bytes=0,eosFrames=0;bool eos=false;
    bool free()const noexcept{return references==0;}
    void attach(StreamState* owner,ISpatialAudioObject* value,uint64_t objectId,uint32_t index,AudioObjectType objectType);
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override;
    ULONG STDMETHODCALLTYPE AddRef()override{return ULONG(InterlockedIncrement(&references));}
    ULONG STDMETHODCALLTYPE Release()override;
    HRESULT STDMETHODCALLTYPE GetBuffer(BYTE** data,UINT32* length)override;
    HRESULT STDMETHODCALLTYPE SetEndOfStream(UINT32 frames)override;
    HRESULT STDMETHODCALLTYPE IsActive(BOOL* active)override{return real->IsActive(active);}
    HRESULT STDMETHODCALLTYPE GetAudioObjectType(AudioObjectType* value)override{return real->GetAudioObjectType(value);}
    HRESULT STDMETHODCALLTYPE SetPosition(float x,float y,float z)override;
    HRESULT STDMETHODCALLTYPE SetVolume(float value)override;
};
struct StreamState {
    Statistics stats;SourceLog capture;uint64_t streamId=0;
    std::array<ObjectProxy,4> objects;
    uint64_t update=0;uint32_t frames=0;bool updating=false;
    void captureBeforeEnd()noexcept {
        if(!updating){stats.captureError=E_UNEXPECTED;return;}
        for(auto& object:objects)if(!object.free()&&object.buffer){
            if(object.bytes!=frames*sizeof(float)){stats.captureError=E_UNEXPECTED;continue;}
            SourceRecord record;record.update=update;record.id=object.id;record.streamId=streamId;record.generation=object.generation;record.sourceIndex=object.sourceIndex;
            record.frames=frames;record.validFrames=object.eos?object.eosFrames:frames;record.eos=object.eos;
            record.type=object.type;record.volume=object.volume;std::copy_n(object.position,3,record.position);
            if(!appendRecord(capture,record,reinterpret_cast<float*>(object.buffer)))stats.captureError=E_BOUNDS;
            else stats.recordsBeforeEnd++;
        }
    }
    void clearBuffers()noexcept {for(auto& object:objects){if(object.buffer)stats.buffersCleared++;object.buffer=nullptr;object.bytes=0;}}
};
struct State {Statistics stats;std::array<StreamState,TapStreams> streams;};
void ObjectProxy::attach(StreamState* owner,ISpatialAudioObject* value,uint64_t objectId,uint32_t index,AudioObjectType objectType){
    state=owner;real.Attach(value);id=objectId;sourceIndex=index;generation++;type=objectType;references=1;volume=1;
    std::fill_n(position,3,0.f);buffer=nullptr;bytes=eosFrames=0;eos=false;state->stats.created[ObjectKind]++;
}
HRESULT ObjectProxy::QueryInterface(REFIID iid,void** out){
    if(!out)return E_POINTER;*out=nullptr;
    if(iid==__uuidof(IUnknown)||iid==__uuidof(ISpatialAudioObject)||iid==__uuidof(ISpatialAudioObjectBase)){
        *out=static_cast<ISpatialAudioObject*>(this);AddRef();return S_OK;
    }
    state->stats.unwrappedInterfaces++;return real->QueryInterface(iid,out);
}
ULONG ObjectProxy::Release(){const LONG count=InterlockedDecrement(&references);if(!count){buffer=nullptr;bytes=0;real.Reset();state->stats.released[ObjectKind]++;}return ULONG(count);}
HRESULT ObjectProxy::GetBuffer(BYTE** data,UINT32* length){
    const HRESULT hr=real->GetBuffer(data,length);
    if(SUCCEEDED(hr)&&data&&length){if(state->updating){buffer=*data;bytes=*length;}else state->stats.captureError=E_UNEXPECTED;}
    return hr;
}
HRESULT ObjectProxy::SetEndOfStream(UINT32 frames){const HRESULT hr=real->SetEndOfStream(frames);if(SUCCEEDED(hr)){eos=true;eosFrames=frames;}return hr;}
HRESULT ObjectProxy::SetPosition(float x,float y,float z){const HRESULT hr=real->SetPosition(x,y,z);if(SUCCEEDED(hr)){position[0]=x;position[1]=y;position[2]=z;}return hr;}
HRESULT ObjectProxy::SetVolume(float value){const HRESULT hr=real->SetVolume(value);if(SUCCEEDED(hr))volume=value;return hr;}

template<class T,Kind K>class ProxyBase:public T {
    LONG references=1;
protected:
    State* state;ComPtr<T> real;
    virtual ~ProxyBase(){real.Reset();state->stats.released[K]++;}
public:
    ProxyBase(State* owner,T* value):state(owner){real.Attach(value);state->stats.created[K]++;}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{
        if(!out)return E_POINTER;*out=nullptr;
        if(iid==__uuidof(IUnknown)||iid==__uuidof(T)){*out=static_cast<T*>(this);AddRef();return S_OK;}
        state->stats.unwrappedInterfaces++;return real->QueryInterface(iid,out);
    }
    ULONG STDMETHODCALLTYPE AddRef()override{return ULONG(InterlockedIncrement(&references));}
    ULONG STDMETHODCALLTYPE Release()override{const LONG count=InterlockedDecrement(&references);if(!count)delete this;return ULONG(count);}
};
class StreamProxy final:public ProxyBase<ISpatialAudioObjectRenderStream,StreamKind>{
public:
    StreamState* streamState;
    StreamProxy(State* owner,ISpatialAudioObjectRenderStream* value,StreamState* perStream):ProxyBase(owner,value),streamState(perStream){}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{
        if(iid==__uuidof(ISpatialAudioObjectRenderStreamBase)){if(!out)return E_POINTER;*out=static_cast<ISpatialAudioObjectRenderStream*>(this);AddRef();return S_OK;}
        return ProxyBase::QueryInterface(iid,out);
    }
    HRESULT STDMETHODCALLTYPE GetAvailableDynamicObjectCount(UINT32* value)override{return real->GetAvailableDynamicObjectCount(value);}
    HRESULT STDMETHODCALLTYPE GetService(REFIID iid,void** out)override{return real->GetService(iid,out);}
    HRESULT STDMETHODCALLTYPE Start()override{return real->Start();}
    HRESULT STDMETHODCALLTYPE Stop()override{return real->Stop();}
    HRESULT STDMETHODCALLTYPE Reset()override{return real->Reset();}
    HRESULT STDMETHODCALLTYPE BeginUpdatingAudioObjects(UINT32* available,UINT32* frames)override{
        const HRESULT hr=real->BeginUpdatingAudioObjects(available,frames);streamState->stats.beginCalls++;
        if(SUCCEEDED(hr)&&frames){streamState->updating=true;streamState->frames=*frames;streamState->update++;streamState->clearBuffers();trackAllocations=true;}
        return hr;
    }
    HRESULT STDMETHODCALLTYPE EndUpdatingAudioObjects()override{
        const uint32_t start=streamState->capture.count;streamState->captureBeforeEnd();
        const HRESULT hr=real->EndUpdatingAudioObjects();streamState->stats.endCalls++;
        for(uint32_t i=start;i<streamState->capture.count;i++)streamState->capture.records[i].endResult=hr;
        streamState->updating=false;streamState->clearBuffers();trackAllocations=false;return hr;
    }
    HRESULT STDMETHODCALLTYPE ActivateSpatialAudioObject(AudioObjectType type,ISpatialAudioObject** out)override{
        if(!out)return real->ActivateSpatialAudioObject(type,out);*out=nullptr;
        ISpatialAudioObject* object=nullptr;const HRESULT hr=real->ActivateSpatialAudioObject(type,&object);if(FAILED(hr))return hr;
        for(uint32_t i=0;i<streamState->objects.size();i++)if(streamState->objects[i].free()){
            streamState->objects[i].attach(streamState,object,nextObjectId.fetch_add(1,std::memory_order_relaxed),i,type);*out=&streamState->objects[i];return hr;
        }
        object->Release();streamState->stats.captureError=E_BOUNDS;return E_OUTOFMEMORY;
    }
};
class ClientProxy final:public ProxyBase<ISpatialAudioClient,ClientKind>{
public:
    using ProxyBase::ProxyBase;
    HRESULT STDMETHODCALLTYPE GetStaticObjectPosition(AudioObjectType type,float* x,float* y,float* z)override{return real->GetStaticObjectPosition(type,x,y,z);}
    HRESULT STDMETHODCALLTYPE GetNativeStaticObjectTypeMask(AudioObjectType* mask)override{return real->GetNativeStaticObjectTypeMask(mask);}
    HRESULT STDMETHODCALLTYPE GetMaxDynamicObjectCount(UINT32* value)override{return real->GetMaxDynamicObjectCount(value);}
    HRESULT STDMETHODCALLTYPE GetSupportedAudioObjectFormatEnumerator(IAudioFormatEnumerator** out)override{return real->GetSupportedAudioObjectFormatEnumerator(out);}
    HRESULT STDMETHODCALLTYPE GetMaxFrameCount(const WAVEFORMATEX* format,UINT32* value)override{return real->GetMaxFrameCount(format,value);}
    HRESULT STDMETHODCALLTYPE IsAudioObjectFormatSupported(const WAVEFORMATEX* format)override{return real->IsAudioObjectFormatSupported(format);}
    HRESULT STDMETHODCALLTYPE IsSpatialAudioStreamAvailable(REFIID iid,const PROPVARIANT* value)override{return real->IsSpatialAudioStreamAvailable(iid,value);}
    HRESULT STDMETHODCALLTYPE ActivateSpatialAudioStream(const PROPVARIANT* parameters,REFIID iid,void** out)override{
        if(iid!=__uuidof(ISpatialAudioObjectRenderStream))return real->ActivateSpatialAudioStream(parameters,iid,out);
        if(!out)return real->ActivateSpatialAudioStream(parameters,iid,out);*out=nullptr;
        ISpatialAudioObjectRenderStream* stream=nullptr;const HRESULT hr=real->ActivateSpatialAudioStream(parameters,iid,reinterpret_cast<void**>(&stream));
        if(FAILED(hr))return hr;
        const uint64_t streamId=nextStreamId.fetch_add(1,std::memory_order_relaxed);
        if(streamId>TapStreams){stream->Release();return E_BOUNDS;}
        auto& perStream=state->streams[size_t(streamId-1)];perStream.streamId=streamId;
        auto* proxy=new(std::nothrow) StreamProxy(state,stream,&perStream);
        if(!proxy){stream->Release();return E_OUTOFMEMORY;}*out=proxy;return hr;
    }
};
class DeviceProxy final:public ProxyBase<IMMDevice,DeviceKind>{
public:
    using ProxyBase::ProxyBase;
    HRESULT STDMETHODCALLTYPE Activate(REFIID iid,DWORD context,PROPVARIANT* parameters,void** out)override{
        if(iid!=__uuidof(ISpatialAudioClient))return real->Activate(iid,context,parameters,out);
        if(!out)return real->Activate(iid,context,parameters,out);*out=nullptr;
        ISpatialAudioClient* client=nullptr;const HRESULT hr=real->Activate(iid,context,parameters,reinterpret_cast<void**>(&client));
        if(FAILED(hr))return hr;auto* proxy=new(std::nothrow) ClientProxy(state,client);
        if(!proxy){client->Release();return E_OUTOFMEMORY;}*out=proxy;return hr;
    }
    HRESULT STDMETHODCALLTYPE OpenPropertyStore(DWORD access,IPropertyStore** out)override{return real->OpenPropertyStore(access,out);}
    HRESULT STDMETHODCALLTYPE GetId(LPWSTR* out)override{return real->GetId(out);}
    HRESULT STDMETHODCALLTYPE GetState(DWORD* out)override{return real->GetState(out);}
};
static HRESULT wrapDevice(State* state,HRESULT hr,IMMDevice* value,IMMDevice** out){
    if(FAILED(hr))return hr;auto* proxy=new(std::nothrow) DeviceProxy(state,value);
    if(!proxy){value->Release();return E_OUTOFMEMORY;}*out=proxy;return hr;
}
class CollectionProxy final:public ProxyBase<IMMDeviceCollection,CollectionKind>{
public:
    using ProxyBase::ProxyBase;
    HRESULT STDMETHODCALLTYPE GetCount(UINT* count)override{return real->GetCount(count);}
    HRESULT STDMETHODCALLTYPE Item(UINT index,IMMDevice** out)override{
        if(!out)return real->Item(index,out);*out=nullptr;IMMDevice* device=nullptr;const HRESULT hr=real->Item(index,&device);return wrapDevice(state,hr,device,out);
    }
};
class EnumeratorProxy final:public ProxyBase<IMMDeviceEnumerator,EnumeratorKind>{
public:
    using ProxyBase::ProxyBase;
    HRESULT STDMETHODCALLTYPE EnumAudioEndpoints(EDataFlow flow,DWORD mask,IMMDeviceCollection** out)override{
        if(!out)return real->EnumAudioEndpoints(flow,mask,out);*out=nullptr;IMMDeviceCollection* collection=nullptr;
        const HRESULT hr=real->EnumAudioEndpoints(flow,mask,&collection);if(FAILED(hr))return hr;
        auto* proxy=new(std::nothrow) CollectionProxy(state,collection);if(!proxy){collection->Release();return E_OUTOFMEMORY;}*out=proxy;return hr;
    }
    HRESULT STDMETHODCALLTYPE GetDefaultAudioEndpoint(EDataFlow flow,ERole role,IMMDevice** out)override{
        if(!out)return real->GetDefaultAudioEndpoint(flow,role,out);*out=nullptr;IMMDevice* device=nullptr;
        const HRESULT hr=real->GetDefaultAudioEndpoint(flow,role,&device);return wrapDevice(state,hr,device,out);
    }
    HRESULT STDMETHODCALLTYPE GetDevice(LPCWSTR id,IMMDevice** out)override{
        if(!out)return real->GetDevice(id,out);*out=nullptr;IMMDevice* device=nullptr;const HRESULT hr=real->GetDevice(id,&device);return wrapDevice(state,hr,device,out);
    }
    HRESULT STDMETHODCALLTYPE RegisterEndpointNotificationCallback(IMMNotificationClient* client)override{return real->RegisterEndpointNotificationCallback(client);}
    HRESULT STDMETHODCALLTYPE UnregisterEndpointNotificationCallback(IMMNotificationClient* client)override{return real->UnregisterEndpointNotificationCallback(client);}
};
using CreateInstance=HRESULT(WINAPI*)(REFCLSID,IUnknown*,DWORD,REFIID,void**);
static CreateInstance realCreate=nullptr;static State* currentState=nullptr;
static HRESULT WINAPI interceptedCreate(REFCLSID clsid,IUnknown* outer,DWORD context,REFIID iid,void** out){
    if(clsid!=__uuidof(MMDeviceEnumerator)||iid!=__uuidof(IMMDeviceEnumerator)||outer)return realCreate(clsid,outer,context,iid,out);
    if(!out)return realCreate(clsid,outer,context,iid,out);*out=nullptr;currentState->stats.coCreateIntercepts++;
    IMMDeviceEnumerator* enumerator=nullptr;const HRESULT hr=realCreate(clsid,outer,context,iid,reinterpret_cast<void**>(&enumerator));
    if(FAILED(hr))return hr;currentState->stats.realCoCreates++;
    auto* proxy=new(std::nothrow) EnumeratorProxy(currentState,enumerator);if(!proxy){enumerator->Release();return E_OUTOFMEMORY;}*out=proxy;return hr;
}
class ProducerImport {
    struct Patch{void** slot=nullptr;void* original=nullptr;DWORD protection=0;bool changed=false;};std::array<Patch,8> patches{};size_t count=0;
public:
    explicit ProducerImport(HMODULE producer){try{
        auto* base=reinterpret_cast<BYTE*>(producer);auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);const auto directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        require(directory.VirtualAddress!=0,"Producer imports missing");auto* descriptor=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+directory.VirtualAddress);
        for(;descriptor->Name;descriptor++)if(descriptor->OriginalFirstThunk){
            auto* names=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+descriptor->OriginalFirstThunk);auto* values=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+descriptor->FirstThunk);
            for(;names->u1.AddressOfData;names++,values++)if(!IMAGE_SNAP_BY_ORDINAL64(names->u1.Ordinal)){
                auto* name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData)->Name;
                if(std::strcmp(name,"CoCreateInstance"))continue;require(count<patches.size(),"Too many CoCreate imports");
                auto& p=patches[count++];p.slot=reinterpret_cast<void**>(&values->u1.Function);p.original=*p.slot;
                if(!realCreate)realCreate=reinterpret_cast<CreateInstance>(p.original);
                require(VirtualProtect(p.slot,sizeof(void*),PAGE_READWRITE,&p.protection)!=FALSE,"Import page protection");
                InterlockedExchangePointer(p.slot,reinterpret_cast<void*>(&interceptedCreate));p.changed=true;DWORD ignored=0;
                require(VirtualProtect(p.slot,sizeof(void*),p.protection,&ignored)!=FALSE,"Import protection restore");
            }
        }
        require(count>0,"Producer has no CoCreateInstance import");
    }catch(...){restore();throw;}}
    void restore()noexcept{for(size_t i=count;i>0;i--){auto& p=patches[i-1];if(p.changed){DWORD previous=0,ignored=0;if(VirtualProtect(p.slot,sizeof(void*),PAGE_READWRITE,&previous)){InterlockedExchangePointer(p.slot,p.original);VirtualProtect(p.slot,sizeof(void*),p.protection,&ignored);p.changed=false;}}}}
    bool restored()const noexcept{for(size_t i=0;i<count;i++)if(*patches[i].slot!=patches[i].original)return false;return true;}
    ~ProducerImport(){restore();}
};

static std::string utf8(const std::wstring& value){const int n=WideCharToMultiByte(CP_UTF8,0,value.c_str(),int(value.size()),nullptr,0,nullptr,nullptr);std::string text(size_t(n),' ');if(n)WideCharToMultiByte(CP_UTF8,0,value.c_str(),int(value.size()),text.data(),n,nullptr,nullptr);return text;}
static std::string quoted(const std::string& value){std::string result="\"";for(unsigned char c:value){if(c=='\\'||c=='\"'){result+='\\';result+=char(c);}else if(c<32)result+='?';else result+=char(c);}return result+'\"';}
struct Endpoint{std::wstring id,name;};
static Endpoint endpoint(){
    ComPtr<IMMDeviceEnumerator> enumerator;ComPtr<IMMDevice> device;
    check(CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator)),"Endpoint enumerator");
    check(enumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&device),"Default render endpoint");
    LPWSTR id=nullptr;check(device->GetId(&id),"Endpoint identity");
    Endpoint result;result.id=id;result.name=L"default render endpoint";CoTaskMemFree(id);
    return result;
}
static void writeNew(const fs::path& path,const void* data,size_t bytes){
    require(bytes<=MAXDWORD,"Output too large");HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);require(file!=INVALID_HANDLE_VALUE,"Output exists or cannot be created");
    DWORD written=0;const BOOL ok=WriteFile(file,data,DWORD(bytes),&written,nullptr);const BOOL closed=CloseHandle(file);require(ok&&written==bytes&&closed,"Output write failed");
}
static void writeLog(const fs::path& folder,const char* name,const SourceLog& log){
    std::ostringstream json;json<<std::setprecision(9)<<"{\"coordinate_system\":\"Windows: +X right,+Y up,+Z backward\",\"rate\":48000,\"records\":[";
    for(uint32_t i=0;i<log.count;i++){const auto& r=log.records[i];if(i)json<<',';json<<"{\"update\":"<<r.update<<",\"object_id\":"<<r.id<<",\"stream_id\":"<<r.streamId<<",\"generation\":"<<r.generation<<",\"source_index\":"<<r.sourceIndex
        <<",\"frames\":"<<r.frames<<",\"valid_frames\":"<<r.validFrames<<",\"eos\":"<<r.eos<<",\"type\":"<<r.type<<",\"position\":["<<r.position[0]<<','<<r.position[1]<<','<<r.position[2]
        <<"],\"volume\":"<<r.volume<<",\"sample_offset\":"<<r.sampleOffset<<",\"forward_end_hresult\":"<<uint32_t(r.endResult)<<'}';}
    json<<"]}\n";const auto text=json.str();writeNew(folder/(std::string(name)+".json"),text.data(),text.size());writeNew(folder/(std::string(name)+".f32"),log.samples,size_t(log.sampleCount)*sizeof(float));
}


static void writeClocks(const fs::path& folder,const ProducerResult& result){
    std::ostringstream json;json<<"{\"host_qpc_frequency\":"<<result.hostQpcFrequency<<",\"clock_log_hresult\":"<<uint32_t(result.clockLogResult)<<",\"phase_names\":[\"before_start\",\"after_start\",\"after_begin\",\"after_end\"],\"streams\":[";
    for(uint32_t i=0;i<TapStreams;i++){const auto& r=result.streams[i];if(i)json<<',';json<<"{\"producer_stream\":"<<i<<",\"clock_service_hresult\":"<<uint32_t(r.clockServiceResult)<<",\"clock2_service_hresult\":"<<uint32_t(r.clock2ServiceResult)<<",\"frequency_hresult\":"<<uint32_t(r.frequencyResult)<<",\"clock_frequency\":"<<r.clockFrequency<<",\"clock_interfaces_released\":"<<r.clockInterfacesReleased<<'}';}
    json<<"],\"records\":[";
    for(uint32_t i=0;i<result.clockCount;i++){const auto& r=result.clocks[i];if(i)json<<',';json<<"{\"producer_stream\":"<<r.stream<<",\"phase\":"<<r.phase<<",\"update\":"<<r.update<<",\"submitted_update_frames\":"<<r.submittedUpdateFrames<<",\"position_hresult\":"<<uint32_t(r.positionResult)<<",\"position\":"<<r.position<<",\"position_qpc_100ns\":"<<r.positionQpc100ns<<",\"device_hresult\":"<<uint32_t(r.deviceResult)<<",\"device_position\":"<<r.devicePosition<<",\"device_qpc_100ns\":"<<r.deviceQpc100ns<<",\"host_qpc_before\":"<<r.hostQpcBefore<<",\"host_qpc_after\":"<<r.hostQpcAfter<<'}';}
    json<<"]}\n";const auto text=json.str();writeNew(folder/L"clocks.json",text.data(),text.size());
}
int wmain(int argc,wchar_t** argv){
    if(argc!=3){std::cerr<<"Use tap_host producer.dll NEW-output-directory\n";return 2;}
    fs::path output=fs::absolute(argv[2]);bool directoryCreated=false,apartment=false;HMODULE producer=nullptr;
    try{
        require(fs::create_directory(output),"Output directory already exists");directoryCreated=true;
        check(CoInitializeEx(nullptr,COINIT_MULTITHREADED),"Host CoInitializeEx");apartment=true;const auto selected=endpoint();
        std::array<std::vector<SourceRecord>,TapStreams> captureRecords,referenceRecords;
        std::array<std::vector<float>,TapStreams> captureSamples,referenceSamples;
        State state;std::array<SourceLog,TapStreams> references;
        for(uint32_t i=0;i<TapStreams;i++){
            captureRecords[i].resize(TapMaxRecords);referenceRecords[i].resize(TapMaxRecords);
            captureSamples[i].resize(TapMaxSamples);referenceSamples[i].resize(TapMaxSamples);
            state.streams[i].capture={captureRecords[i].data(),TapMaxRecords,0,captureSamples[i].data(),TapMaxSamples,0};
            references[i]={referenceRecords[i].data(),TapMaxRecords,0,referenceSamples[i].data(),TapMaxSamples,0};
        }
        currentState=&state;ProducerResult result;
        producer=LoadLibraryExW(fs::canonical(argv[1]).c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);require(producer!=nullptr,"Load producer DLL");
        auto run=reinterpret_cast<RunProducer>(GetProcAddress(producer,"RunSpatialProducer"));require(run!=nullptr,"Producer export missing");
        ProducerImport patch(producer);const auto start=std::chrono::steady_clock::now();const HRESULT hr=run(selected.id.c_str(),references.data(),&result);
        const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();patch.restore();const bool importsRestored=patch.restored();
        bool identical=true,released=true,allComplete=true;uint32_t totalBegin=0,totalEnd=0,totalCopies=0,totalClears=0,totalUnknown=state.stats.unwrappedInterfaces;
        std::array<uint64_t,TapStreams*3> lifetimeIds{};size_t lifetimeCount=0;
        std::vector<SourceRecord> mergedCaptureRecords,mergedReferenceRecords;std::vector<float> mergedCaptureSamples,mergedReferenceSamples;
        std::ostringstream streams;streams<<'[';
        for(uint32_t index=0;index<TapStreams;index++){
            const auto& stream=state.streams[index];const auto& actualLog=stream.capture;const auto& reference=references[index];const auto& r=result.streams[index];
            identical&=stream.streamId==index+1&&actualLog.count==reference.count&&actualLog.sampleCount==reference.sampleCount;
            std::array<std::array<uint64_t,2>,TapObjects> ids{};uint32_t eos=0;
            for(uint32_t i=0;identical&&i<reference.count;i++){
                const auto& actual=actualLog.records[i];const auto& expected=reference.records[i];
                identical=actual.sourceIndex<TapObjects&&actual.generation<2&&actual.streamId==expected.streamId&&actual.sourceIndex==expected.sourceIndex&&actual.generation==expected.generation&&actual.update==expected.update&&actual.frames==expected.frames&&actual.validFrames==expected.validFrames&&actual.eos==expected.eos&&actual.type==expected.type&&actual.volume==expected.volume&&std::equal(actual.position,actual.position+3,expected.position)&&actual.endResult==expected.endResult;
                if(actual.sourceIndex<TapObjects&&actual.generation<2){auto& id=ids[actual.sourceIndex][actual.generation];if(!id)id=actual.id;else identical&=id==actual.id;}
                identical&=actual.id>0&&actual.volume<=.001f;
                if(identical)identical=std::memcmp(actualLog.samples+actual.sampleOffset,reference.samples+expected.sampleOffset,size_t(actual.frames)*4)==0;
                eos+=actual.eos;
            }
            identical&=ids[0][0]>0&&ids[0][1]>ids[0][0]&&ids[1][0]>0&&ids[1][1]==0&&eos==3;
            for(auto id:{ids[0][0],ids[0][1],ids[1][0]})lifetimeIds[lifetimeCount++]=id;
            for(size_t k=0;k<KindCount;k++)released&=stream.stats.created[k]==stream.stats.released[k];
            allComplete&=SUCCEEDED(r.result)&&SUCCEEDED(r.stopResult)&&SUCCEEDED(r.resetResult)&&r.eosInactiveObjects==TapObjects&&r.recreatedObjects==1&&r.validSourceFrames==TapTargetFrames&&SUCCEEDED(stream.stats.captureError);
            totalBegin+=stream.stats.beginCalls;totalEnd+=stream.stats.endCalls;totalCopies+=stream.stats.recordsBeforeEnd;totalClears+=stream.stats.buffersCleared;
            totalUnknown+=stream.stats.unwrappedInterfaces;
            for(uint32_t i=0;i<actualLog.count;i++){auto record=actualLog.records[i];record.sampleOffset+=mergedCaptureSamples.size();mergedCaptureRecords.push_back(record);}
            mergedCaptureSamples.insert(mergedCaptureSamples.end(),actualLog.samples,actualLog.samples+actualLog.sampleCount);
            for(uint32_t i=0;i<reference.count;i++){auto record=reference.records[i];record.sampleOffset+=mergedReferenceSamples.size();mergedReferenceRecords.push_back(record);}
            mergedReferenceSamples.insert(mergedReferenceSamples.end(),reference.samples,reference.samples+reference.sampleCount);
            if(index)streams<<',';streams<<"{\"stream_id\":"<<stream.streamId<<",\"producer_stream\":"<<index<<",\"producer_hresult\":"<<uint32_t(r.result)<<",\"stage\":"<<quoted(r.stage)<<",\"maximum_frame_count\":"<<r.maxFrames<<",\"source_updates\":"<<r.updates<<",\"drain_updates\":"<<r.drainUpdates<<",\"valid_frames\":"<<r.validSourceFrames<<",\"eos_inactive_objects\":"<<r.eosInactiveObjects<<",\"recreated_objects\":"<<r.recreatedObjects<<",\"captured_records\":"<<actualLog.count<<",\"capture_hresult\":"<<uint32_t(stream.stats.captureError)<<",\"stop_hresult\":"<<uint32_t(r.stopResult)<<",\"reset_hresult\":"<<uint32_t(r.resetResult)<<'}';
        }
        streams<<']';std::sort(lifetimeIds.begin(),lifetimeIds.end());identical&=lifetimeIds.front()>0&&std::adjacent_find(lifetimeIds.begin(),lifetimeIds.end())==lifetimeIds.end();
        for(size_t k=0;k<KindCount;k++)released&=state.stats.created[k]==state.stats.released[k];
        const bool passed=SUCCEEDED(hr)&&identical&&released&&allComplete&&importsRestored&&state.stats.realCoCreates==1&&totalUnknown==0&&callbackNew==0&&callbackDelete==0&&elapsed<2.0;
        SourceLog mergedCapture{mergedCaptureRecords.data(),uint32_t(mergedCaptureRecords.size()),uint32_t(mergedCaptureRecords.size()),mergedCaptureSamples.data(),uint32_t(mergedCaptureSamples.size()),uint32_t(mergedCaptureSamples.size())};
        SourceLog mergedReference{mergedReferenceRecords.data(),uint32_t(mergedReferenceRecords.size()),uint32_t(mergedReferenceRecords.size()),mergedReferenceSamples.data(),uint32_t(mergedReferenceSamples.size()),uint32_t(mergedReferenceSamples.size())};
        writeLog(output,"capture",mergedCapture);writeLog(output,"reference",mergedReference);writeClocks(output,result);
        std::ostringstream record;record<<std::setprecision(12)<<"{\"passed\":"<<(passed?"true":"false")<<",\"producer_hresult\":"<<uint32_t(hr)<<",\"stage\":"<<quoted(result.stage)<<",\"endpoint_name\":"<<quoted(utf8(selected.name))<<",\"endpoint_id\":"<<quoted("omitted")<<",\"actual_windows_objects_forwarded\":true,\"rate\":"<<result.rate<<",\"maximum_dynamic_objects\":"<<result.maxDynamic<<",\"exact_reference_match\":"<<(identical?"true":"false")<<",\"streams\":"<<streams.str()<<",\"unique_object_lifetimes\":"<<lifetimeCount<<",\"begin_calls\":"<<totalBegin<<",\"forwarded_end_calls\":"<<totalEnd<<",\"records_copied_before_forward_end\":"<<totalCopies<<",\"buffer_pointers_cleared_after_end\":"<<totalClears<<",\"unknown_interface_passthroughs\":"<<totalUnknown<<",\"proxy_references_released\":"<<(released?"true":"false")<<",\"producer_imports_restored\":"<<(importsRestored?"true":"false")<<",\"host_callback_operator_new_calls\":"<<callbackNew<<",\"host_callback_operator_delete_calls\":"<<callbackDelete<<",\"elapsed_seconds\":"<<elapsed<<",\"scope\":\"Two actual interleaved Windows streams on one producer thread, independent capture pools, explicit object lifetime IDs\"}\n";
        const auto text=record.str();writeNew(output/L"result.json",text.data(),text.size());std::cout<<text;currentState=nullptr;FreeLibrary(producer);producer=nullptr;CoUninitialize();apartment=false;return passed?0:1;
    }catch(const std::exception& error){trackAllocations=false;if(producer)FreeLibrary(producer);if(apartment)CoUninitialize();
        if(directoryCreated){const std::string text="{\"passed\":false,\"live_part_stopped\":true,\"diagnostic\":"+quoted(error.what())+"}\n";try{writeNew(output/L"failure.json",text.data(),text.size());}catch(...){} }
        std::cerr<<error.what()<<'\n';return 1;
    }
}
