#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <unknwn.h>
#include <string>
#include <mutex>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <atomic>
#include <cstdlib>
#include <new>
#include <array>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "../sofa_xtc_runtime/adapter.h"
static std::atomic<unsigned long long> allocationCount{};
void* operator new(size_t bytes){allocationCount.fetch_add(1,std::memory_order_relaxed);if(void* value=std::malloc(bytes?bytes:1))return value;throw std::bad_alloc();}
void* operator new[](size_t bytes){return ::operator new(bytes);}
void operator delete(void* value) noexcept{std::free(value);}
void operator delete[](void* value) noexcept{std::free(value);}
void operator delete(void* value,size_t) noexcept{std::free(value);}
void operator delete[](void* value,size_t) noexcept{std::free(value);}
static const GUID ClassId={0x1eb4a2d1,0x0aec,0x4dfa,{0xa0,0x2c,0xc9,0x44,0xcb,0x6f,0x74,0xe2}};
static std::wstring cpuPath;
static std::mutex pathMutex;
static HMODULE dataHelperModule{};
extern "C" { void* NativeDataHelper{}; }
static ULONG creations{};
static int processingMode{};
static ULONG diagnosticCalls{}, diagnosticErrors{}, liveDsps{};
static ULONG sofaCalls{},sofaErrors{};
static unsigned long long renderAllocations{};
static std::vector<float> sofaFilters,sofaDirections;
static UINT32 sofaBankCount{},sofaTaps{},sofaCapacity{};
struct SofaObservation { UINT32 call,index,bank,frames,pcmHash; float gain,position[3]; };
static std::array<SofaObservation,4096> sofaObservations{};
static UINT32 sofaObservationCount{};
struct SofaState {
 runtime::BlockAdapter adapter;
 std::array<runtime::Binding,runtime::MaxObjects> bindings{};
 std::array<int,runtime::MaxObjects> previous{};
 std::array<runtime::SourceBlock,runtime::MaxObjects> blocks{};
 std::array<int,runtime::MaxObjects> selected{};
 UINT32 sources;
 explicit SofaState(UINT32 count):sources(count){
  adapter.renderer.configure(sofaFilters.data(),static_cast<int>(sofaBankCount),static_cast<int>(sofaTaps),static_cast<int>(sofaCapacity),1.f);previous.fill(-1);
  for(UINT32 i=0;i<count;i++){bindings[i]=adapter.renderer.start(i);if(!adapter.renderer.valid(bindings[i]))throw std::runtime_error("Binding failure");}
 }
};
static const GUID FactoryId={0xb91f3db7,0x302c,0x4ae3,{0x83,0xde,0x77,0x35,0xf2,0x4e,0xf4,0x62}};
struct Source { float position[3],gain; BYTE unmapped[88]; const float* samples; UINT32 frames,tail; };
static_assert(sizeof(Source)==120&&offsetof(Source,samples)==104&&offsetof(Source,frames)==112);
using CreateDsp=HRESULT(WINAPI*)(IUnknown*,IUnknown*,IUnknown*,UINT32,IUnknown**);
using Process=HRESULT(WINAPI*)(IUnknown*,const Source*,float*,UINT32,UINT32);
extern "C" void Forward3(); extern "C" void Forward4(); extern "C" void Forward6(); extern "C" void Forward7(); extern "C" void Forward8();
class DspFactory;
struct Dsp { void** table; IUnknown* inner; ULONG references{1}; DspFactory* owner; std::unique_ptr<SofaState> sofa; };
static_assert(offsetof(Dsp,inner)==8);
static HRESULT WINAPI DspQuery(Dsp* self,REFIID iid,void** out);
static ULONG WINAPI DspAdd(Dsp* self){return InterlockedIncrement(&self->references);}
static ULONG WINAPI DspRelease(Dsp* self);
static HRESULT SofaRender(Dsp* self,const Source* sources,float* output,UINT32 frames,UINT32 count){
 InterlockedIncrement(&sofaCalls);
 auto& state=*self->sofa;
 if(!sources||!output||count!=state.sources||!frames||frames%xtc::Block||frames>8192){InterlockedIncrement(&sofaErrors);return E_INVALIDARG;}
 const auto before=allocationCount.load(std::memory_order_relaxed);
 for(UINT32 i=0;i<count;i++){
  auto& source=sources[i];if(!source.samples||source.frames!=frames||!std::isfinite(source.gain)){InterlockedIncrement(&sofaErrors);return E_INVALIDARG;}
  state.selected[i]=runtime::nearestDirection(sofaDirections.data(),static_cast<int>(sofaBankCount),source.position);
  if(state.selected[i]<0){InterlockedIncrement(&sofaErrors);return E_INVALIDARG;}
  if(sofaObservationCount<sofaObservations.size()){
   UINT32 hash=2166136261u;auto bytes=reinterpret_cast<const unsigned char*>(source.samples);for(size_t n=0;n<size_t(frames)*4;n++)hash=(hash^bytes[n])*16777619u;
   sofaObservations[sofaObservationCount++]={sofaCalls,i,static_cast<UINT32>(state.selected[i]),frames,hash,source.gain,{source.position[0],source.position[1],source.position[2]}};
  }
 }
 for(UINT32 offset=0;offset<frames;offset+=xtc::Block){
  for(UINT32 i=0;i<count;i++){
   const int old=state.previous[i]<0?state.selected[i]:state.previous[i];
   const float begin=float(offset)/float(frames-1),end=float(offset+xtc::Block-1)/float(frames-1);
   // Only offset-12 native scalar gain is mapped. The following 16 bytes
   // are not interpreted as a gain ramp or second position.
   state.blocks[i]={state.bindings[i],sources[i].samples+offset,xtc::Block,old,state.selected[i],sources[i].gain,sources[i].gain,begin,end};
  }
  if(!state.adapter.render(state.blocks.data(),static_cast<int>(count),output+offset*2)){InterlockedIncrement(&sofaErrors);return E_INVALIDARG;}
 }
 for(UINT32 i=0;i<count;i++)state.previous[i]=state.selected[i];
 renderAllocations+=allocationCount.load(std::memory_order_relaxed)-before;
 return S_OK;
}
static HRESULT WINAPI Render(Dsp* self,const Source* sources,float* output,UINT32 frames,UINT32 count){
 if(processingMode==2)return SofaRender(self,sources,output,frames,count);
 if(processingMode==0)return reinterpret_cast<Process>((*reinterpret_cast<void***>(self->inner))[5])(self->inner,sources,output,frames,count);
 InterlockedIncrement(&diagnosticCalls);
 if(!sources||!output||count!=2||!frames||frames>8192){InterlockedIncrement(&diagnosticErrors);return E_INVALIDARG;}
 for(unsigned index=0;index<2;index++){auto& source=sources[index];if(!source.samples||source.frames!=frames||source.position[0]!=(index?1.0f:-1.0f)||source.position[1]!=0||source.position[2]!=0||source.gain!=1.0f){InterlockedIncrement(&diagnosticErrors);return E_INVALIDARG;}}
 for(unsigned frame=0;frame<frames;frame++){output[2*frame]=sources[0].samples[frame];output[2*frame+1]=sources[1].samples[frame];}return S_OK;
}
static void* DspMethods[]={reinterpret_cast<void*>(&DspQuery),reinterpret_cast<void*>(&DspAdd),reinterpret_cast<void*>(&DspRelease),reinterpret_cast<void*>(&Forward3),reinterpret_cast<void*>(&Forward4),reinterpret_cast<void*>(&Render),reinterpret_cast<void*>(&Forward6),reinterpret_cast<void*>(&Forward7),reinterpret_cast<void*>(&Forward8)};
class DspFactory final:public IUnknown{
 ULONG references{1}; IUnknown* inner; HMODULE cpu;
 public:
 DspFactory(IUnknown* value,HMODULE module):inner(value),cpu(module){}
 ~DspFactory(){inner->Release();FreeLibrary(cpu);}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(iid!=IID_IUnknown&&iid!=FactoryId)return E_NOINTERFACE;*out=static_cast<IUnknown*>(this);AddRef();return S_OK;}
 ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&references);}
 ULONG STDMETHODCALLTYPE Release()override{ULONG value=InterlockedDecrement(&references);if(!value)delete this;return value;}
 virtual HRESULT STDMETHODCALLTYPE Create(IUnknown* a,IUnknown* b,UINT32 sources,IUnknown** out){if(!out)return E_POINTER;*out=nullptr;if(processingMode==2&&(sources<1||sources>sofaCapacity))return E_INVALIDARG;IUnknown* native{};HRESULT hr=reinterpret_cast<CreateDsp>((*reinterpret_cast<void***>(inner))[3])(inner,a,b,sources,&native);if(FAILED(hr))return hr;try{std::unique_ptr<SofaState> prepared;if(processingMode==2)prepared=std::make_unique<SofaState>(sources);auto dsp=new Dsp{DspMethods,native,1,this,std::move(prepared)};AddRef();InterlockedIncrement(&liveDsps);*out=reinterpret_cast<IUnknown*>(dsp);return S_OK;}catch(...){native->Release();return E_FAIL;}}
};
static HRESULT WINAPI DspQuery(Dsp* self,REFIID iid,void** out){
 if(!out)return E_POINTER;*out=nullptr;
 if(iid==IID_IUnknown){*out=self;DspAdd(self);return S_OK;}
 void* native{};HRESULT hr=self->inner->QueryInterface(iid,&native);if(FAILED(hr))return hr;
 if(native==self->inner){static_cast<IUnknown*>(native)->Release();*out=self;DspAdd(self);return S_OK;}
 // Secondary native interfaces require an explicit proxy to preserve COM
 // identity and module ownership. This bounded test does not advertise them.
 static_cast<IUnknown*>(native)->Release();return E_NOINTERFACE;
}
static ULONG WINAPI DspRelease(Dsp* self){ULONG value=InterlockedDecrement(&self->references);if(!value){self->inner->Release();self->owner->Release();InterlockedDecrement(&liveDsps);delete self;}return value;}
class Factory final:public IClassFactory{
 ULONG references{1};
 public:
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(iid!=IID_IUnknown&&iid!=IID_IClassFactory)return E_NOINTERFACE;*out=static_cast<IClassFactory*>(this);AddRef();return S_OK;}
 ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&references);}
 ULONG STDMETHODCALLTYPE Release()override{ULONG value=InterlockedDecrement(&references);if(!value)delete this;return value;}
 HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer,REFIID iid,void** out)override{
  if(!out)return E_POINTER;*out=nullptr;if(outer)return CLASS_E_NOAGGREGATION;
  std::lock_guard<std::mutex> lock(pathMutex);if(cpuPath.empty())return E_UNEXPECTED;
  HMODULE cpu=LoadLibraryExW(cpuPath.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);if(!cpu)return HRESULT_FROM_WIN32(GetLastError());
  auto create=reinterpret_cast<HRESULT(WINAPI*)(unsigned,IUnknown**)>(GetProcAddress(cpu,"CreateHrtfDspFactory"));
  if(!create){FreeLibrary(cpu);return E_NOINTERFACE;}IUnknown* factory{};HRESULT hr=create(0,&factory);if(FAILED(hr)){FreeLibrary(cpu);return hr;}
  DspFactory* replacement{};try{replacement=new DspFactory(factory,cpu);}catch(const std::bad_alloc&){factory->Release();FreeLibrary(cpu);return E_OUTOFMEMORY;}hr=replacement->QueryInterface(iid,out);replacement->Release();if(SUCCEEDED(hr))InterlockedIncrement(&creations);return hr;
 }
 HRESULT STDMETHODCALLTYPE LockServer(BOOL)override{return S_OK;}
};
extern "C" __declspec(dllexport) HRESULT WINAPI ConfigureCpuPath(const wchar_t* path){
 if(!path||!*path)return E_INVALIDARG;
 try{std::lock_guard<std::mutex> lock(pathMutex);if(!cpuPath.empty())return cpuPath==path?S_OK:E_UNEXPECTED;
  std::wstring prepared(path);auto module=LoadLibraryExW(prepared.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);if(!module)return HRESULT_FROM_WIN32(GetLastError());
  auto helper=GetProcAddress(module,"CreateHrtfDataHelper");if(!helper){FreeLibrary(module);return E_NOINTERFACE;}
  cpuPath=std::move(prepared);dataHelperModule=module;InterlockedExchangePointer(&NativeDataHelper,reinterpret_cast<void*>(helper));return S_OK;
 }catch(const std::bad_alloc&){return E_OUTOFMEMORY;}catch(...){return E_FAIL;}
}
extern "C" __declspec(dllexport) ULONG WINAPI GetCreationCount(){return creations;}
extern "C" __declspec(dllexport) HRESULT WINAPI ConfigureDiagnostic(BOOL value){if(liveDsps)return E_UNEXPECTED;processingMode=value?1:0;return S_OK;}
extern "C" __declspec(dllexport) ULONG WINAPI GetDiagnosticCalls(){return diagnosticCalls;}
extern "C" __declspec(dllexport) ULONG WINAPI GetDiagnosticErrors(){return diagnosticErrors;}
extern "C" __declspec(dllexport) ULONG WINAPI GetLiveDspCount(){return liveDsps;}
extern "C" __declspec(dllexport) HRESULT WINAPI ConfigureSofa(const float* filters,UINT32 banks,UINT32 taps,const float* directions,UINT32 maxSources){
 if(liveDsps)return E_UNEXPECTED;
 if(!filters||!directions||!banks||banks>2048||taps!=4096||!maxSources||maxSources>64)return E_INVALIDARG;
 try{std::vector<float> newFilters(filters,filters+size_t(banks)*taps*2),newDirections(directions,directions+size_t(banks)*3);
 for(float value:newFilters)if(!std::isfinite(value))return E_INVALIDARG;
 for(UINT32 i=0;i<banks;i++){float* d=&newDirections[size_t(i)*3];const float norm=d[0]*d[0]+d[1]*d[1]+d[2]*d[2];if(!std::isfinite(d[0])||!std::isfinite(d[1])||!std::isfinite(d[2])||!std::isfinite(norm)||norm<.1f)return E_INVALIDARG;const float inverse=1.f/std::sqrt(norm);for(int axis=0;axis<3;axis++)d[axis]*=inverse;}
 sofaFilters=std::move(newFilters);sofaDirections=std::move(newDirections);sofaBankCount=banks;sofaTaps=taps;sofaCapacity=maxSources;sofaCalls=sofaErrors=sofaObservationCount=0;renderAllocations=0;processingMode=2;return S_OK;}catch(...){return E_FAIL;}
}
extern "C" __declspec(dllexport) ULONG WINAPI GetSofaCalls(){return sofaCalls;}
extern "C" __declspec(dllexport) ULONG WINAPI GetSofaErrors(){return sofaErrors;}
extern "C" __declspec(dllexport) unsigned long long WINAPI GetRenderAllocations(){return renderAllocations;}
extern "C" __declspec(dllexport) UINT32 WINAPI GetSofaObservations(SofaObservation* out,UINT32 capacity){if(!out)return sofaObservationCount;UINT32 count=std::min(capacity,sofaObservationCount);std::copy_n(sofaObservations.data(),count,out);return count;}
#include "factory_initialization.h"
extern "C" __declspec(dllexport) HRESULT WINAPI CreateHrtfDspFactory(UINT32 mode,IUnknown** out){if(!out)return E_POINTER;*out=nullptr;if(mode)return E_INVALIDARG;Factory factory;return factory.CreateInstance(nullptr,FactoryId,reinterpret_cast<void**>(out));}
STDAPI DllGetClassObject(REFCLSID cls,REFIID iid,void** out){if(!out)return E_POINTER;*out=nullptr;if(cls!=ClassId)return CLASS_E_CLASSNOTAVAILABLE;Factory* factory{};try{factory=new Factory;}catch(const std::bad_alloc&){return E_OUTOFMEMORY;}HRESULT hr=factory->QueryInterface(iid,out);factory->Release();return hr;}
STDAPI DllCanUnloadNow(){return S_FALSE;}
