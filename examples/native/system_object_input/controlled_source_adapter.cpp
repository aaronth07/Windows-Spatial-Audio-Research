#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <SpatialAudioClient.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
using Microsoft::WRL::ComPtr;
struct Object;
struct Snapshot { unsigned id, type, frames; std::array<float,3> position; float volume; std::vector<float> pcm; };
struct State { bool started{}, updating{}, streamAllocated{}; unsigned frames{}, nextId{1},maxDynamic{8}; AudioObjectType staticMask{AudioObjectType_FrontLeft}; HANDLE event{}; std::vector<Object*> objects; std::vector<Snapshot> captured; };
template<class Interface> HRESULT Identify(Interface* self, REFIID iid, void** out) {
 if(!out) return E_POINTER;
 *out=nullptr;
 if(iid!=__uuidof(IUnknown)&&iid!=__uuidof(Interface)) return E_NOINTERFACE;
 *out=self; self->AddRef(); return S_OK;
}
struct Object final : ISpatialAudioObject {
 ULONG refs{1}; std::shared_ptr<State> state; unsigned id; AudioObjectType type; std::array<float,3> position{}; float volume{1}; bool active{true}, touched{}, ending{}; unsigned endFrames{}; std::vector<float> pcm;
 Object(std::shared_ptr<State> s,AudioObjectType t):state(std::move(s)),id(state->nextId++),type(t){state->objects.push_back(this);}
 ~Object(){ auto& values=state->objects; values.erase(std::remove(values.begin(),values.end(),this),values.end()); }
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out) override { if(iid==__uuidof(ISpatialAudioObjectBase)) { if(!out)return E_POINTER; *out=static_cast<ISpatialAudioObjectBase*>(this);AddRef();return S_OK;} return Identify<ISpatialAudioObject>(this,iid,out); }
 ULONG STDMETHODCALLTYPE AddRef() override{return ++refs;}
 ULONG STDMETHODCALLTYPE Release() override{auto n=--refs;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE GetBuffer(BYTE** data,UINT32* bytes) override {if(!data||!bytes)return E_POINTER;*data=nullptr;*bytes=0;if(!state->updating||!active)return E_UNEXPECTED;if(!touched)pcm.assign(state->frames,0);touched=true;*data=reinterpret_cast<BYTE*>(pcm.data());*bytes=state->frames*4;return S_OK;}
 HRESULT STDMETHODCALLTYPE SetEndOfStream(UINT32 frames) override{if(!state->updating||!active||frames>state->frames)return E_INVALIDARG;ending=true;endFrames=frames;return S_OK;}
 HRESULT STDMETHODCALLTYPE IsActive(BOOL* value) override{if(!value)return E_POINTER;*value=active;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetAudioObjectType(AudioObjectType* value) override{if(!value)return E_POINTER;*value=type;return S_OK;}
 HRESULT STDMETHODCALLTYPE SetPosition(float x,float y,float z) override{if(type!=AudioObjectType_Dynamic||!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z))return E_INVALIDARG;position={x,y,z};return S_OK;}
 HRESULT STDMETHODCALLTYPE SetVolume(float value) override{if(!std::isfinite(value)||value<0||value>1)return E_INVALIDARG;volume=value;return S_OK;}
};
struct Stream final : ISpatialAudioObjectRenderStream {
 ULONG refs{1}; std::shared_ptr<State> state;
 explicit Stream(std::shared_ptr<State> s):state(std::move(s)){state->streamAllocated=true;}
 ~Stream(){state->streamAllocated=false;}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out) override{if(iid==__uuidof(ISpatialAudioObjectRenderStreamBase)){if(!out)return E_POINTER;*out=static_cast<ISpatialAudioObjectRenderStreamBase*>(this);AddRef();return S_OK;}return Identify<ISpatialAudioObjectRenderStream>(this,iid,out);}
 ULONG STDMETHODCALLTYPE AddRef() override{return ++refs;}
 ULONG STDMETHODCALLTYPE Release() override{auto n=--refs;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE GetAvailableDynamicObjectCount(UINT32* value) override{if(!value)return E_POINTER;*value=state->maxDynamic-static_cast<UINT32>(std::count_if(state->objects.begin(),state->objects.end(),[](Object* o){return o->active&&o->type==AudioObjectType_Dynamic;}));return S_OK;}
 HRESULT STDMETHODCALLTYPE GetService(REFIID,void** out) override{if(!out)return E_POINTER;*out=nullptr;return E_NOINTERFACE;}
 HRESULT STDMETHODCALLTYPE Start() override{state->started=true;SetEvent(state->event);return S_OK;}
 HRESULT STDMETHODCALLTYPE Stop() override{if(state->updating)return E_UNEXPECTED;state->started=false;return S_OK;}
 HRESULT STDMETHODCALLTYPE Reset() override{if(state->started||state->updating)return E_UNEXPECTED;state->captured.clear();return S_OK;}
 HRESULT STDMETHODCALLTYPE BeginUpdatingAudioObjects(UINT32* available,UINT32* frames) override{if(!available||!frames)return E_POINTER;if(!state->started||state->updating)return E_UNEXPECTED;state->updating=true;*frames=state->frames;return GetAvailableDynamicObjectCount(available);}
 HRESULT STDMETHODCALLTYPE EndUpdatingAudioObjects() override{if(!state->updating)return E_UNEXPECTED;for(auto object:state->objects){if(object->active&&object->touched){unsigned frames=object->ending?object->endFrames:state->frames;state->captured.push_back({object->id,static_cast<unsigned>(object->type),frames,object->position,object->volume,std::vector<float>(object->pcm.begin(),object->pcm.begin()+frames)});object->touched=false;}if(object->ending)object->active=false;}state->updating=false;SetEvent(state->event);return S_OK;}
 HRESULT STDMETHODCALLTYPE ActivateSpatialAudioObject(AudioObjectType type,ISpatialAudioObject** out) override{if(!out)return E_POINTER;*out=nullptr;if(type!=AudioObjectType_Dynamic&&type!=AudioObjectType_FrontLeft)return E_INVALIDARG;if(type!=AudioObjectType_Dynamic&&((state->staticMask&type)==0||std::any_of(state->objects.begin(),state->objects.end(),[=](Object* o){return o->active&&o->type==type;})))return E_INVALIDARG;UINT32 available{};GetAvailableDynamicObjectCount(&available);if(type==AudioObjectType_Dynamic&&!available)return E_OUTOFMEMORY;*out=new Object(state,type);return S_OK;}
};
static bool Supported(const WAVEFORMATEX* f){return f&&f->wFormatTag==WAVE_FORMAT_IEEE_FLOAT&&f->nChannels==1&&f->nSamplesPerSec==48000&&f->wBitsPerSample==32&&f->nBlockAlign==4&&f->nAvgBytesPerSec==192000&&f->cbSize==0;}
struct Formats final : IAudioFormatEnumerator {
 ULONG refs{1};
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out) override{return Identify<IAudioFormatEnumerator>(this,iid,out);}
 ULONG STDMETHODCALLTYPE AddRef() override{return ++refs;}
 ULONG STDMETHODCALLTYPE Release() override{auto n=--refs;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE GetCount(UINT32* value) override{if(!value)return E_POINTER;*value=1;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetFormat(UINT32 index,WAVEFORMATEX** out) override{if(!out)return E_POINTER;*out=nullptr;if(index)return E_INVALIDARG;auto f=static_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));if(!f)return E_OUTOFMEMORY;*f={WAVE_FORMAT_IEEE_FLOAT,1,48000,192000,4,32,0};*out=f;return S_OK;}
};
struct Client final : ISpatialAudioClient {
 ULONG refs{1};std::shared_ptr<State> state;
 explicit Client(std::shared_ptr<State> s):state(std::move(s)){}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out) override{return Identify<ISpatialAudioClient>(this,iid,out);}
 ULONG STDMETHODCALLTYPE AddRef() override{return ++refs;}
 ULONG STDMETHODCALLTYPE Release() override{auto n=--refs;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE GetStaticObjectPosition(AudioObjectType type,float* x,float* y,float* z) override{if(!x||!y||!z)return E_POINTER;if(type!=AudioObjectType_FrontLeft)return E_INVALIDARG;*x=-1;*y=0;*z=-1;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetNativeStaticObjectTypeMask(AudioObjectType* mask) override{if(!mask)return E_POINTER;*mask=AudioObjectType_FrontLeft;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetMaxDynamicObjectCount(UINT32* value) override{if(!value)return E_POINTER;*value=8;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetSupportedAudioObjectFormatEnumerator(IAudioFormatEnumerator** out) override{if(!out)return E_POINTER;*out=new Formats;return S_OK;}
 HRESULT STDMETHODCALLTYPE GetMaxFrameCount(const WAVEFORMATEX* f,UINT32* value) override{if(!value)return E_POINTER;if(!Supported(f))return E_INVALIDARG;*value=state->frames;return S_OK;}
 HRESULT STDMETHODCALLTYPE IsAudioObjectFormatSupported(const WAVEFORMATEX* f) override{return Supported(f)?S_OK:E_INVALIDARG;}
 HRESULT STDMETHODCALLTYPE IsSpatialAudioStreamAvailable(REFIID iid,const PROPVARIANT*) override{return iid==__uuidof(ISpatialAudioObjectRenderStream)?S_OK:E_NOINTERFACE;}
 HRESULT STDMETHODCALLTYPE ActivateSpatialAudioStream(const PROPVARIANT* p,REFIID iid,void** out) override{if(!out)return E_POINTER;*out=nullptr;if(iid!=__uuidof(ISpatialAudioObjectRenderStream))return E_NOINTERFACE;if(state->streamAllocated)return E_UNEXPECTED;if(!p||p->vt!=VT_BLOB||p->blob.cbSize!=sizeof(SpatialAudioObjectRenderStreamActivationParams)||!p->blob.pBlobData)return E_INVALIDARG;auto a=reinterpret_cast<const SpatialAudioObjectRenderStreamActivationParams*>(p->blob.pBlobData);if(!Supported(a->ObjectFormat)||!a->EventHandle||a->MaxDynamicObjectCount>8||a->MinDynamicObjectCount>a->MaxDynamicObjectCount||a->NotifyObject||(a->StaticObjectTypeMask&~AudioObjectType_FrontLeft)!=0)return E_INVALIDARG;state->event=a->EventHandle;state->maxDynamic=a->MaxDynamicObjectCount;state->staticMask=a->StaticObjectTypeMask;*out=new Stream(state);return S_OK;}
};
struct Device final : IMMDevice {
 ULONG refs{1};std::shared_ptr<State> state;
 explicit Device(std::shared_ptr<State> s):state(std::move(s)){}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out) override{return Identify<IMMDevice>(this,iid,out);}
 ULONG STDMETHODCALLTYPE AddRef() override{return ++refs;}
 ULONG STDMETHODCALLTYPE Release() override{auto n=--refs;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE Activate(REFIID iid,DWORD,PROPVARIANT*,void** out) override{if(!out)return E_POINTER;*out=nullptr;if(iid!=__uuidof(ISpatialAudioClient))return E_NOINTERFACE;*out=new Client(state);return S_OK;}
 HRESULT STDMETHODCALLTYPE OpenPropertyStore(DWORD,IPropertyStore** out) override{if(!out)return E_POINTER;*out=nullptr;return E_NOTIMPL;}
 HRESULT STDMETHODCALLTYPE GetId(LPWSTR* out) override{if(!out)return E_POINTER;*out=nullptr;return E_NOTIMPL;}
 HRESULT STDMETHODCALLTYPE GetState(DWORD* out) override{if(!out)return E_POINTER;*out=DEVICE_STATE_ACTIVE;return S_OK;}
};
static void Check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static void Ok(HRESULT value){Check(SUCCEEDED(value),"Unexpected HRESULT");}
int wmain(int argc,wchar_t** argv){
 if(argc!=2){std::cerr<<"Usage: controlled_source_adapter NEW-output-directory\n";return 2;}
 try{
  std::filesystem::path output(argv[1]);Check(std::filesystem::create_directory(output),"Output exists");unsigned captures=0;
  for(unsigned frames:{1U,48U,127U,480U,1024U}){
   auto state=std::make_shared<State>();state->frames=frames;ComPtr<IMMDevice> device;device.Attach(new Device(state));ComPtr<ISpatialAudioClient> client;Ok(device->Activate(__uuidof(ISpatialAudioClient),CLSCTX_INPROC_SERVER,nullptr,reinterpret_cast<void**>(client.GetAddressOf())));
   ComPtr<IAudioFormatEnumerator> formats;Ok(client->GetSupportedAudioObjectFormatEnumerator(&formats));UINT32 count{};Ok(formats->GetCount(&count));Check(count==1,"Formats count");WAVEFORMATEX* format{};Check(formats->GetFormat(1,&format)==E_INVALIDARG&&!format,"Format index rejection");Ok(formats->GetFormat(0,&format));
   HANDLE event=CreateEventW(nullptr,FALSE,FALSE,nullptr);Check(event!=nullptr,"Event creation");SpatialAudioObjectRenderStreamActivationParams activation{format,AudioObjectType_FrontLeft,0,8,AudioCategory_GameEffects,event,nullptr};PROPVARIANT variant{};variant.vt=VT_BLOB;variant.blob.cbSize=sizeof(activation);variant.blob.pBlobData=reinterpret_cast<BYTE*>(&activation);ComPtr<ISpatialAudioObjectRenderStream> stream;Ok(client->ActivateSpatialAudioStream(&variant,__uuidof(ISpatialAudioObjectRenderStream),reinterpret_cast<void**>(stream.GetAddressOf())));CoTaskMemFree(format);
   Check(stream->EndUpdatingAudioObjects()==E_UNEXPECTED,"End outside transaction");UINT32 available{},n{};Check(stream->BeginUpdatingAudioObjects(&available,&n)==E_UNEXPECTED,"Begin before Start");Ok(stream->Start());Check(WaitForSingleObject(event,0)==WAIT_OBJECT_0,"Start event");
   ComPtr<ISpatialAudioObject> a,b;Ok(stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&a));Ok(stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&b));unsigned firstId=state->objects[0]->id,secondId=state->objects[1]->id;
   std::vector<ComPtr<ISpatialAudioObject>> extras(6);for(auto& extra:extras)Ok(stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&extra));ComPtr<ISpatialAudioObject> excess;Check(stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&excess)==E_OUTOFMEMORY&&!excess,"Dynamic object limit");extras.clear();
   ComPtr<ISpatialAudioObject> fixed;Ok(stream->ActivateSpatialAudioObject(AudioObjectType_FrontLeft,&fixed));Check(stream->ActivateSpatialAudioObject(AudioObjectType_FrontLeft,&excess)==E_INVALIDARG&&!excess,"Duplicate static channel");Check(fixed->SetPosition(0,0,0)==E_INVALIDARG,"Static position rejection");fixed.Reset();Check(stream->Reset()==E_UNEXPECTED,"Reset while started");
   for(unsigned block=0;block<3;block++){
    Ok(stream->BeginUpdatingAudioObjects(&available,&n));Check(n==frames&&available==6,"Available objects/frames");Check(stream->BeginUpdatingAudioObjects(&available,&n)==E_UNEXPECTED,"Nested Begin");
    for(unsigned index=0;index<2;index++){auto object=index?b.Get():a.Get();Ok(object->SetPosition(static_cast<float>(block+index),static_cast<float>(index),-1));Ok(object->SetVolume(index?.5f:1));BYTE* data{};UINT32 bytes{};Ok(object->GetBuffer(&data,&bytes));Check(bytes==frames*4,"Buffer length");auto pcm=reinterpret_cast<float*>(data);for(unsigned frame=0;frame<frames;frame++)pcm[frame]=static_cast<float>((index+1)*1000+block*10+frame%7)/4096;BYTE* repeated{};UINT32 repeatedBytes{};Ok(object->GetBuffer(&repeated,&repeatedBytes));Check(repeated==data&&repeatedBytes==bytes,"Repeated buffer identity");}
    if(block==2)Ok(a->SetEndOfStream(frames/2));Ok(stream->EndUpdatingAudioObjects());Check(WaitForSingleObject(event,0)==WAIT_OBJECT_0,"End event");
    for(unsigned index=0;index<2;index++){auto& snap=state->captured[state->captured.size()-2+index];Check(snap.id==(index?secondId:firstId)&&snap.position[0]==block+index&&snap.volume==(index?.5f:1),"Identity/pose/gain");unsigned extent=block==2&&!index?frames/2:frames;Check(snap.frames==extent,"EOS length");for(unsigned frame=0;frame<extent;frame++)Check(snap.pcm[frame]==static_cast<float>((index+1)*1000+block*10+frame%7)/4096,"PCM identity");++captures;}
   }
   BOOL active{};Ok(a->IsActive(&active));Check(!active,"EOS inactivity");a.Reset();ComPtr<ISpatialAudioObject> replacement;Ok(stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&replacement));Check(state->objects.back()->id>secondId,"Monotonic identity after release");Check(b->SetVolume(-1)==E_INVALIDARG,"Gain rejection");Check(b->GetBuffer(nullptr,nullptr)==E_POINTER,"Pointer rejection");Check(stream->GetService(__uuidof(IUnknown),nullptr)==E_POINTER,"Service pointer rejection");
   Ok(stream->Stop());Ok(stream->Reset());Check(state->captured.empty(),"Reset");replacement.Reset();b.Reset();stream.Reset();client.Reset();device.Reset();Check(state->objects.empty(),"Source cleanup");CloseHandle(event);
  }
  std::ofstream file(output/L"result.json");file<<"{\"passed\":true,\"frame_sizes\":[1,48,127,480,1024],\"exact_source_snapshots\":"<<captures<<",\"scope\":\"Controlled IMMDevice/ISpatialAudioClient/ISpatialAudioObjectRenderStream/ISpatialAudioObject implementation; no real endpoint, injection, forwarding to Windows, audio I/O, or game compatibility proof.\"}\n";Check(bool(file),"Result write");std::cout<<"PASS exact_source_snapshots="<<captures<<"\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<"\n";return 1;}
}
