// Capture-only engine: no property, routing or spatial-format setters.
#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <spatialaudioclient.h>
#include <ks.h>
#include <ksmedia.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
using Microsoft::WRL::ComPtr;
namespace fs=std::filesystem;
std::wstring Playback,Recording;
std::string ExpectedSettings;
#include <functiondiscoverykeys_devpkey.h>
#include <map>
constexpr uint32_t Rate=48000, Segment=Rate*2, Total=Segment*5;
constexpr float Peak=0.01f;
constexpr std::array<std::array<float,3>,5> Positions{{{0,0,-1},{-1,0,0},{1,0,0},{0,0,1},{0,1,0}}};
void Require(bool valid,const char* message){if(!valid)throw std::runtime_error(message);}
void Check(HRESULT hr,const char* stage){static std::set<std::string> logged;if(FAILED(hr)||logged.insert(stage).second)std::cout<<stage<<" HRESULT="<<std::hex<<static_cast<uint32_t>(hr)<<std::dec<<"\n";if(FAILED(hr))throw std::runtime_error(stage);}
struct ComScope{ComScope(){Check(CoInitializeEx(nullptr,COINIT_MULTITHREADED),"CoInitializeEx");}~ComScope(){CoUninitialize();}};
struct Handle{HANDLE value;Handle():value(CreateEventW(nullptr,FALSE,FALSE,nullptr)){Require(value!=nullptr,"CreateEvent");}~Handle(){CloseHandle(value);}Handle(const Handle&)=delete;};
struct TaskMemory{void operator()(void* value)const{CoTaskMemFree(value);}};
using FormatPointer=std::unique_ptr<WAVEFORMATEX,TaskMemory>;
std::wstring DefaultSnapshot;
void Defaults(IMMDeviceEnumerator* devices){
 std::wstring current;
 for(ERole role:{eConsole,eMultimedia,eCommunications}){ComPtr<IMMDevice> device;HRESULT result=devices->GetDefaultAudioEndpoint(eRender,role,&device);if(result==HRESULT_FROM_WIN32(ERROR_NOT_FOUND)){current+=L"none;";continue;}Check(result,"GetDefaultAudioEndpoint");LPWSTR id=nullptr;Check(device->GetId(&id),"GetId");current+=id;current+=L";";CoTaskMemFree(id);}
 if(DefaultSnapshot.empty())DefaultSnapshot=current;else Require(DefaultSnapshot==current,"Default playback device changed during capture");
}

WAVEFORMATEX ObjectFormat(){WAVEFORMATEX f{};f.wFormatTag=WAVE_FORMAT_IEEE_FLOAT;f.nChannels=1;f.nSamplesPerSec=Rate;f.wBitsPerSample=32;f.nBlockAlign=4;f.nAvgBytesPerSec=Rate*4;return f;}
float Sample(uint32_t frame){return frame<Total&&frame%Segment==Rate/2?Peak:0.0f;}
void Fill(float* buffer,uint32_t frames,uint32_t first){for(uint32_t i=0;i<frames;i++)buffer[i]=Sample(first+i);}
void Put32(std::ofstream& file,uint32_t value){for(int i=0;i<4;i++)file.put(static_cast<char>((value>>(8*i))&255));}
void Chunk(std::ofstream& file,const char* name,const void* data,uint32_t bytes){file.write(name,4);Put32(file,bytes);if(bytes)file.write(static_cast<const char*>(data),bytes);if(bytes&1)file.put(0);}
void WriteWave(const fs::path& path,const WAVEFORMATEX* format,const std::vector<BYTE>& bytes){
 Require(!fs::exists(path),"Refusing to overwrite recording");uint32_t formatBytes=sizeof(WAVEFORMATEX)+format->cbSize;
 Require(bytes.size()<10000000&&bytes.size()%format->nBlockAlign==0,"Capture extent invalid");
 uint32_t size=4+8+formatBytes+(formatBytes&1)+8+static_cast<uint32_t>(bytes.size())+(bytes.size()&1);
 // Include fact for IEEE float/extensible WAV; retain actual negotiated fmt bytes.
 uint32_t frames=static_cast<uint32_t>(bytes.size()/format->nBlockAlign);size+=12;
 std::ofstream file(path,std::ios::binary);Require(file.good(),"Cannot create capture WAV");file.write("RIFF",4);Put32(file,size);file.write("WAVE",4);
 Chunk(file,"fmt ",format,formatBytes);file.write("fact",4);Put32(file,4);Put32(file,frames);Chunk(file,"data",bytes.data(),static_cast<uint32_t>(bytes.size()));file.close();Require(file.good(),"Capture write failed");
}
struct Streams{
 ComPtr<IAudioClient> capture;ComPtr<ISpatialAudioObjectRenderStream> render;bool captureStarted=false,renderStarted=false;
 ~Streams(){if(renderStarted){std::cout<<"render.Stop HRESULT="<<std::hex<<static_cast<uint32_t>(render->Stop())<<std::dec<<"\n";render->Reset();}if(captureStarted)std::cout<<"capture.Stop HRESULT="<<std::hex<<static_cast<uint32_t>(capture->Stop())<<std::dec<<"\n";}
};
struct AudioScheduling{
 DWORD index=0;HANDLE task=AvSetMmThreadCharacteristicsW(L"Pro Audio",&index);
 AudioScheduling(){if(task){BOOL priority=AvSetMmThreadPriority(task,AVRT_PRIORITY_HIGH);std::cout<<"mmcss_registered=1 priority_high="<<priority<<'\n';}else std::cout<<"mmcss_registered=0 win32_error="<<GetLastError()<<'\n';}
 ~AudioScheduling(){if(task)AvRevertMmThreadCharacteristics(task);}
};
struct Updating{ISpatialAudioObjectRenderStream* stream;bool open=true;~Updating(){if(open)stream->EndUpdatingAudioObjects();}void End(){open=false;Check(stream->EndUpdatingAudioObjects(),"EndUpdatingAudioObjects");}};
void Drain(IAudioCaptureClient* reader,const WAVEFORMATEX* format,std::vector<BYTE>& bytes,uint32_t& packets,uint32_t& silent){
 UINT32 frames=0;Check(reader->GetNextPacketSize(&frames),"GetNextPacketSize");
 while(frames){BYTE* data=nullptr;DWORD flags=0;UINT64 devicePosition=0,qpc=0;Check(reader->GetBuffer(&data,&frames,&flags,&devicePosition,&qpc),"capture.GetBuffer");
  bool gap=packets>0&&(flags&(AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY|AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR));
  size_t count=static_cast<size_t>(frames)*format->nBlockAlign;bool excessive=bytes.size()+count>10000000;
  if(!gap&&!excessive){size_t old=bytes.size();bytes.resize(old+count);if(flags&AUDCLNT_BUFFERFLAGS_SILENT){std::fill(bytes.begin()+old,bytes.end(),static_cast<BYTE>(0));silent++;}else if(data)std::memcpy(bytes.data()+old,data,count);else gap=true;}
  Check(reader->ReleaseBuffer(frames),"capture.ReleaseBuffer");Require(!gap&&!excessive,"Capture gap, timestamp error, pointer error or extent limit");packets++;
  Check(reader->GetNextPacketSize(&frames),"GetNextPacketSize");
 }
}
void Capabilities(ISpatialAudioClient* client){
 UINT32 maximum=0;AudioObjectType mask{};Check(client->GetMaxDynamicObjectCount(&maximum),"GetMaxDynamicObjectCount");Check(client->GetNativeStaticObjectTypeMask(&mask),"GetNativeStaticObjectTypeMask");
 std::cout<<"max_dynamic_objects="<<maximum<<" static_mask="<<static_cast<uint32_t>(mask)<<"\n";
 auto f=ObjectFormat();HRESULT hr=client->IsAudioObjectFormatSupported(&f);Check(hr,"IsAudioObjectFormatSupported(float32 mono48000)");Require(hr==S_OK,"Exact object format not supported");
 ComPtr<IAudioFormatEnumerator> formats;Check(client->GetSupportedAudioObjectFormatEnumerator(&formats),"GetSupportedAudioObjectFormatEnumerator");UINT32 count=0;Check(formats->GetCount(&count),"formats.GetCount");Require(count<=128,"Unexpected format count");
 for(UINT32 i=0;i<count;i++){WAVEFORMATEX* candidate=nullptr;Check(formats->GetFormat(i,&candidate),"formats.GetFormat");Require(candidate!=nullptr,"Null enumerated format"); // Used while enumerator remains alive; not retained.
  std::cout<<"object_format "<<i<<" tag="<<candidate->wFormatTag<<" channels="<<candidate->nChannels<<" rate="<<candidate->nSamplesPerSec<<" bits="<<candidate->wBitsPerSample<<"\n";}
 Require(maximum>=1,"No dynamic audio object available; no automatic fallback");
}
#include <roapi.h>
#include <winstring.h>
#include <bcrypt.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include <functional>
struct HText{HSTRING value=nullptr;explicit HText(const wchar_t* text){Check(WindowsCreateString(text,static_cast<UINT32>(wcslen(text)),&value),"WindowsCreateString");}~HText(){WindowsDeleteString(value);}};
template<class T>T Slot(IUnknown* p,size_t n){return reinterpret_cast<T>((*reinterpret_cast<void***>(p))[n]);}
ComPtr<IUnknown> Factory(const wchar_t* name,const wchar_t* iid){HText text(name);GUID id{};Check(IIDFromString(iid,&id),"IIDFromString");ComPtr<IUnknown> p;Check(RoGetActivationFactory(text.value,id,reinterpret_cast<void**>(p.GetAddressOf())),"RoGetActivationFactory");return p;}
std::wstring TextGetter(IUnknown* p,size_t slot){using F=HRESULT(STDMETHODCALLTYPE*)(IUnknown*,HSTRING*);HSTRING text=nullptr;Check(Slot<F>(p,slot)(p,&text),"spatial getter");UINT32 n=0;const wchar_t* raw=WindowsGetStringRawBuffer(text,&n);std::wstring value(raw,n);WindowsDeleteString(text);return value;}
std::wstring ActiveProvider;
std::wstring EndpointGuid(){auto dot=Playback.find(L"}.");Require(dot!=std::wstring::npos,"Invalid playback endpoint ID");return Playback.substr(dot+2);}
void SpatialFormat(){
 std::wstring id=L"\\\\?\\SWD#MMDEVAPI#"+Playback+L"#{e6327cad-dcec-4949-ae8a-991e976a79d2}";
 HText device(id.c_str());auto factory=Factory(L"Windows.Media.Audio.SpatialAudioDeviceConfiguration",L"{3ec37f7b-936d-4e04-9728-2827d9f758c4}");using Create=HRESULT(STDMETHODCALLTYPE*)(IUnknown*,HSTRING,IUnknown**);ComPtr<IUnknown> config;Check(Slot<Create>(factory.Get(),6)(factory.Get(),device.value,config.GetAddressOf()),"SpatialAudioDeviceConfiguration");
 auto active=TextGetter(config.Get(),9),selected=TextGetter(config.Get(),10),actual=TextGetter(config.Get(),6);
 Require(actual.find(EndpointGuid())!=std::wstring::npos,"Selected playback endpoint changed");
 Require(!active.empty()&&active!=L"{00000000-0000-0000-0000-000000000000}"&&_wcsicmp(active.c_str(),selected.c_str())==0,"Enable Spatial Sound in Windows settings for the selected playback device");
 if(ActiveProvider.empty())ActiveProvider=active;else Require(_wcsicmp(ActiveProvider.c_str(),active.c_str())==0,"Spatial Sound provider changed during capture");
}

std::string Sha(const std::vector<BYTE>& data){BCRYPT_ALG_HANDLE alg=nullptr;Require(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)==0,"SHA provider");std::array<BYTE,32> digest{};NTSTATUS status=BCryptHash(alg,nullptr,0,const_cast<BYTE*>(data.data()),static_cast<ULONG>(data.size()),digest.data(),static_cast<ULONG>(digest.size()));BCryptCloseAlgorithmProvider(alg,0);Require(status==0,"SHA hash");std::ostringstream out;for(BYTE value:digest)out<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<unsigned>(value);return out.str();}
void RegistryBytes(HKEY key,const std::wstring& path,std::map<std::wstring,std::vector<BYTE>>& values,unsigned depth){
 Require(depth<12&&values.size()<4096,"Endpoint settings exceed snapshot bounds");
 DWORD count=0,maxName=0,maxData=0,children=0,maxChild=0;Require(RegQueryInfoKeyW(key,nullptr,nullptr,nullptr,&children,&maxChild,nullptr,&count,&maxName,&maxData,nullptr,nullptr)==ERROR_SUCCESS,"Query endpoint settings");
 Require(count<4096&&children<4096&&maxData<=8388608&&maxName<32768&&maxChild<32768,"Endpoint settings exceed snapshot bounds");
 for(DWORD i=0;i<count;i++){std::vector<wchar_t> name(static_cast<size_t>(maxName)+2);std::vector<BYTE> data(static_cast<size_t>(maxData)+4);DWORD n=maxName+1,size=maxData,type=0;Require(RegEnumValueW(key,i,name.data(),&n,nullptr,&type,data.data()+4,&size)==ERROR_SUCCESS,"Read endpoint settings");std::memcpy(data.data(),&type,4);data.resize(static_cast<size_t>(size)+4);values[path+L"/"+std::wstring(name.data(),n)]=std::move(data);}
 for(DWORD i=0;i<children;i++){std::vector<wchar_t> name(static_cast<size_t>(maxChild)+2);DWORD n=maxChild+1;Require(RegEnumKeyExW(key,i,name.data(),&n,nullptr,nullptr,nullptr,nullptr)==ERROR_SUCCESS,"Read endpoint settings key");HKEY child=nullptr;Require(RegOpenKeyExW(key,name.data(),0,KEY_READ|KEY_WOW64_64KEY,&child)==ERROR_SUCCESS,"Open endpoint settings key");try{RegistryBytes(child,path+L"/"+std::wstring(name.data(),n),values,depth+1);}catch(...){RegCloseKey(child);throw;}RegCloseKey(child);}
}
std::string SettingsFingerprint(){
 std::wstring path=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MMDevices\\Audio\\Render\\"+EndpointGuid()+L"\\FxProperties";
 HKEY key=nullptr;LSTATUS status=RegOpenKeyExW(HKEY_LOCAL_MACHINE,path.c_str(),0,KEY_READ|KEY_WOW64_64KEY,&key);Require(status==ERROR_SUCCESS||status==ERROR_FILE_NOT_FOUND,"Endpoint settings unavailable");
 std::map<std::wstring,std::vector<BYTE>> values;if(status==ERROR_SUCCESS){try{RegistryBytes(key,L"",values,0);}catch(...){RegCloseKey(key);throw;}RegCloseKey(key);}
 std::vector<BYTE> data;for(const auto& value:values){Require(data.size()+value.first.size()*2+value.second.size()+8<=16777216,"Endpoint settings snapshot too large");auto name=reinterpret_cast<const BYTE*>(value.first.data());uint32_t n=static_cast<uint32_t>(value.first.size()*sizeof(wchar_t));auto length=reinterpret_cast<const BYTE*>(&n);data.insert(data.end(),length,length+4);data.insert(data.end(),name,name+n);n=static_cast<uint32_t>(value.second.size());length=reinterpret_cast<const BYTE*>(&n);data.insert(data.end(),length,length+4);data.insert(data.end(),value.second.begin(),value.second.end());}return Sha(data);
}
std::string Json(const std::wstring& text){int n=WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);std::string utf8(n,'\0');WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),utf8.data(),n,nullptr,nullptr);std::string out="\"";for(unsigned char c:utf8){if(c=='"'||c=='\\'){out+='\\';out+=static_cast<char>(c);}else if(c<32){const char* hex="0123456789abcdef";out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=static_cast<char>(c);}return out+'"';}
void Devices(IMMDeviceEnumerator* devices){
 for(EDataFlow flow:{eRender,eCapture}){ComPtr<IMMDeviceCollection> collection;Check(devices->EnumAudioEndpoints(flow,DEVICE_STATE_ACTIVE,&collection),"EnumAudioEndpoints");UINT count=0;Check(collection->GetCount(&count),"GetCount");for(UINT i=0;i<count;i++){ComPtr<IMMDevice> device;Check(collection->Item(i,&device),"Item");LPWSTR id=nullptr;Check(device->GetId(&id),"GetId");std::wstring identifier=id;CoTaskMemFree(id);ComPtr<IPropertyStore> store;Check(device->OpenPropertyStore(STGM_READ,&store),"OpenPropertyStore");PROPVARIANT name{};HRESULT hr=store->GetValue(PKEY_Device_FriendlyName,&name);std::wstring friendly=SUCCEEDED(hr)&&name.vt==VT_LPWSTR?name.pwszVal:L"Audio device";PropVariantClear(&name);std::cout<<"{\"event\":\"device\",\"flow\":\""<<(flow==eRender?"render":"capture")<<"\",\"id\":"<<Json(identifier)<<",\"name\":"<<Json(friendly)<<"}"<<std::endl;}}
}

struct Direction{double az=0,el=0,r=1;std::array<float,3> xyz{};uint64_t first=0,end=0;bool complete=false;};
std::vector<Direction> ReadGrid(const fs::path& path){std::ifstream in(path);Require(in.good(),"Cannot read grid CSV");std::vector<Direction> grid;std::string line;while(std::getline(in,line)){if(line.empty()||line[0]=='#')continue;std::replace(line.begin(),line.end(),',',' ');std::istringstream row(line);Direction d;Require(static_cast<bool>(row>>d.az>>d.el>>d.r>>d.xyz[0]>>d.xyz[1]>>d.xyz[2]),"Invalid six-column grid");Require(std::isfinite(d.az)&&std::isfinite(d.el)&&std::isfinite(d.r)&&d.el>=-90&&d.el<=90&&d.r>0&&d.r<=100,"Invalid direction");double norm=0;for(float value:d.xyz){Require(std::isfinite(value),"Invalid native coordinates");norm+=static_cast<double>(value)*value;}Require(std::abs(std::sqrt(norm)-d.r)<1e-4,"Coordinate/radius mismatch");grid.push_back(d);}Require(!grid.empty()&&grid.size()<=10000,"Grid must contain 1..10000 directions");return grid;}
std::vector<float> ReadStimulus(const fs::path& path){std::ifstream in(path,std::ios::binary|std::ios::ate);Require(in.good(),"Cannot read mono stimulus");auto size=in.tellg();Require(size>0&&size<Rate*20*4&&size%4==0,"Invalid stimulus extent");std::vector<float> x(static_cast<size_t>(size)/4);in.seekg(0);in.read(reinterpret_cast<char*>(x.data()),size);Require(in.good(),"Read stimulus failed");for(float a:x)Require(std::isfinite(a)&&std::abs(a)<=0.1f,"Stimulus must be finite and bounded at0.1");return x;}
uint64_t Qpc(){LARGE_INTEGER t{},f{};QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return static_cast<uint64_t>(static_cast<long double>(t.QuadPart)*10000000/f.QuadPart);}
struct StereoWriter{
 std::ofstream file;uint64_t frames=0;explicit StereoWriter(const fs::path& p){Require(!fs::exists(p),"Refusing to overwrite raw WAV");file.open(p,std::ios::binary);Require(file.good(),"Cannot create raw WAV");file.write("RIFF",4);Put32(file,0);file.write("WAVEfmt ",8);Put32(file,18);WAVEFORMATEX f{};f.wFormatTag=3;f.nChannels=2;f.nSamplesPerSec=Rate;f.nAvgBytesPerSec=Rate*8;f.nBlockAlign=8;f.wBitsPerSample=32;file.write(reinterpret_cast<const char*>(&f),18);file.write("fact",4);Put32(file,4);Put32(file,0);file.write("data",4);Put32(file,0);}
 void Add(const BYTE* data,UINT32 count,bool silent){Require(frames+count<(0xffffffffull-58)/8,"WAV 4GiB limit reached");if(silent){std::vector<float> zeros(static_cast<size_t>(count)*2);file.write(reinterpret_cast<const char*>(zeros.data()),static_cast<std::streamsize>(zeros.size()*4));}else{Require(data!=nullptr,"Null capture data");const float* samples=reinterpret_cast<const float*>(data);for(size_t i=0;i<static_cast<size_t>(count)*2;i++)Require(std::isfinite(samples[i])&&std::abs(samples[i])<0.98f,"Capture clipped or nonfinite; own stream stopped");file.write(reinterpret_cast<const char*>(data),static_cast<std::streamsize>(count)*8);}frames+=count;Require(file.good(),"Capture disk write failed");}
 void Finish(){if(!file.is_open())return;file.seekp(4);Put32(file,static_cast<uint32_t>(frames*8+50));file.seekp(46);Put32(file,static_cast<uint32_t>(frames));file.seekp(54);Put32(file,static_cast<uint32_t>(frames*8));file.close();}
 ~StereoWriter(){Finish();}
};
struct ReleaseCaptureBuffer{IAudioCaptureClient* reader;UINT32 frames;bool open=true;~ReleaseCaptureBuffer(){if(open)reader->ReleaseBuffer(frames);}};
struct CaptureState{uint64_t firstQpc=0,firstDevice=0,lastDevice=0,lastQpc=0,nextDevice=0;uint32_t packets=0,silent=0;};
void DrainFile(IAudioCaptureClient* reader,StereoWriter& wave,CaptureState& state,std::ofstream& timestamps){UINT32 count=0;Check(reader->GetNextPacketSize(&count),"GetNextPacketSize");while(count){BYTE* data=nullptr;DWORD flags=0;UINT64 device=0,qpc=0;Check(reader->GetBuffer(&data,&count,&flags,&device,&qpc),"capture.GetBuffer");ReleaseCaptureBuffer held{reader,count};bool gap=(state.packets>0&&(flags&AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY))||(flags&AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR)||(state.packets>0&&device!=state.nextDevice);if(!gap){if(!state.packets){state.firstQpc=qpc;state.firstDevice=device;}timestamps<<wave.frames<<','<<device<<','<<qpc<<','<<count<<','<<flags<<'\n';wave.Add(data,count,(flags&AUDCLNT_BUFFERFLAGS_SILENT)!=0);state.nextDevice=device+count;state.lastDevice=device;state.lastQpc=qpc;state.packets++;if(flags&AUDCLNT_BUFFERFLAGS_SILENT)state.silent++;}if(gap)std::cerr<<"capture_gap flags="<<flags<<" expected_device_position="<<state.nextDevice<<" actual_device_position="<<device<<" qpc100ns="<<qpc<<" frames="<<count<<"\n";held.open=false;Check(reader->ReleaseBuffer(count),"capture.ReleaseBuffer");Require(!gap,"Capture discontinuity, clock jump or timestamp error");Check(reader->GetNextPacketSize(&count),"GetNextPacketSize");}}
std::string Control(const fs::path& session){std::ifstream file(session/L"control.txt");std::string state;std::getline(file,state);return state;}
void CaptureSphere(IMMDeviceEnumerator* devices,ISpatialAudioClient* client,const fs::path& session,const fs::path& gridPath,const fs::path& inputPath,bool loopback){
 struct ExclusiveCapture{HANDLE value=CreateMutexW(nullptr,FALSE,L"Local\\WindowsSpatialSound2SOFA_Capture");bool held=false;ExclusiveCapture(){Require(value!=nullptr,"Create capture mutex");DWORD result=WaitForSingleObject(value,0);held=result==WAIT_OBJECT_0||result==WAIT_ABANDONED;if(!held){CloseHandle(value);value=nullptr;throw std::runtime_error("Another capture is active");}}~ExclusiveCapture(){if(held)ReleaseMutex(value);if(value)CloseHandle(value);}} exclusive;
 Require(fs::is_directory(session),"Session folder missing");auto grid=ReadGrid(gridPath);auto stimulus=ReadStimulus(inputPath);std::string fingerprint=SettingsFingerprint();Require(fingerprint==ExpectedSettings,"Endpoint settings changed before capture");Defaults(devices);SpatialFormat();Capabilities(client);
 Handle renderEvent,captureEvent;Streams streams;ComPtr<IMMDevice> device;Check(devices->GetDevice((loopback?Playback:Recording).c_str(),&device),"GetDevice(capture)");Check(device->Activate(__uuidof(IAudioClient),CLSCTX_INPROC_SERVER,nullptr,reinterpret_cast<void**>(streams.capture.GetAddressOf())),"Activate capture client");WAVEFORMATEX* raw=nullptr;Check(streams.capture->GetMixFormat(&raw),"GetMixFormat");FormatPointer format(raw);
 Require(format!=nullptr,"Missing capture format");if(format->wFormatTag==WAVE_FORMAT_EXTENSIBLE)Require(format->cbSize>=22,"Invalid extensible capture format");bool floating=format&&format->wFormatTag==WAVE_FORMAT_IEEE_FLOAT;if(format&&format->wFormatTag==WAVE_FORMAT_EXTENSIBLE){auto ext=reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(format.get());floating=IsEqualGUID(ext->SubFormat,KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)!=0;Require(ext->dwChannelMask==3,"Capture channel mask must be left/right");}
 Require(floating&&format->nChannels==2&&format->nSamplesPerSec==Rate&&format->wBitsPerSample==32&&format->nBlockAlign==8,"Capture must negotiate stereo float32/48k without format changes");DWORD flags=AUDCLNT_STREAMFLAGS_EVENTCALLBACK|(loopback?AUDCLNT_STREAMFLAGS_LOOPBACK:0);
 Check(streams.capture->Initialize(AUDCLNT_SHAREMODE_SHARED,flags,1000000,0,format.get(),nullptr),"capture.Initialize");Check(streams.capture->SetEventHandle(captureEvent.value),"capture.SetEventHandle");ComPtr<IAudioCaptureClient> reader;Check(streams.capture->GetService(__uuidof(IAudioCaptureClient),reinterpret_cast<void**>(reader.GetAddressOf())),"capture.GetService");
 auto f=ObjectFormat();SpatialAudioObjectRenderStreamActivationParams args{};args.ObjectFormat=&f;args.StaticObjectTypeMask=AudioObjectType_None;args.MinDynamicObjectCount=1;args.MaxDynamicObjectCount=1;args.Category=AudioCategory_SoundEffects;args.EventHandle=renderEvent.value;
 PROPVARIANT activation{};activation.vt=VT_BLOB;activation.blob.cbSize=sizeof(args);activation.blob.pBlobData=reinterpret_cast<BYTE*>(&args);Check(client->ActivateSpatialAudioStream(&activation,__uuidof(ISpatialAudioObjectRenderStream),reinterpret_cast<void**>(streams.render.GetAddressOf())),"ActivateSpatialAudioStream(dynamic1)");
 StereoWriter wave(session/L"raw_binaural.wav");std::ofstream timestamps(session/L"capture_timestamps.csv"),renderTimes(session/L"render_timestamps.csv");Require(timestamps.good()&&renderTimes.good(),"Cannot create timestamp logs");timestamps<<"capture_frame,device_position,qpc100ns,frames,flags\n";renderTimes<<"source_frame,qpc100ns\n";
 AudioScheduling scheduling;Check(streams.capture->Start(),"capture.Start");streams.captureStarted=true;Check(streams.render->Start(),"render.Start");streams.renderStarted=true;ComPtr<ISpatialAudioObject> object;CaptureState capture;uint64_t sent=0,firstRenderQpc=0;size_t index=0,offset=0;bool started=false,stopped=false,paused=false;ULONGLONG began=GetTickCount64(),lastRender=began,lastControl=0;std::string command="run";HANDLE events[]{renderEvent.value,captureEvent.value};
 bool captureOk=false,metadataSaved=false;auto saveCapture=[&](){wave.Finish(); std::ofstream meta(session/L"capture.json");meta<<"{\"success\":"<<(captureOk?"true":"false")<<",\"sample_rate\":48000,\"channels\":2,\"capture_bits\":32,\"capture_format_tag\":"<<format->wFormatTag<<",\"capture_mode\":\""<<(loopback?"loopback":"paired")<<"\",\"renderer_settings_sha256\":\""<<fingerprint<<"\",\"capture_frames\":"<<wave.frames<<",\"source_frames\":"<<sent<<",\"first_render_submission_qpc100ns\":"<<firstRenderQpc<<",\"first_capture_qpc100ns\":"<<capture.firstQpc<<",\"first_capture_device_position\":"<<capture.firstDevice<<",\"last_capture_device_position\":"<<capture.lastDevice<<",\"last_capture_qpc100ns\":"<<capture.lastQpc<<",\"packets\":"<<capture.packets<<",\"stopped\":"<<(stopped?"true":"false")<<",\"directions\":[";
 for(size_t i=0;i<grid.size();i++){const auto& d=grid[i];if(i)meta<<',';meta<<"{\"azimuth\":"<<std::setprecision(12)<<d.az<<",\"elevation\":"<<d.el<<",\"radius\":"<<d.r<<",\"xyz\":["<<d.xyz[0]<<','<<d.xyz[1]<<','<<d.xyz[2]<<"],\"start_source_frame\":"<<d.first<<",\"end_source_frame\":"<<d.end<<",\"complete\":"<<(d.complete?"true":"false")<<'}';}meta<<"]}";meta.close();Require(meta.good(),"Capture metadata write failed");metadataSaved=true;};struct SaveCapture{std::function<void()> write;~SaveCapture(){try{write();}catch(...){}}} save{[&](){if(!metadataSaved)saveCapture();}};
 while(index<grid.size()){
  Require(GetTickCount64()-began<7200000&&GetTickCount64()-lastRender<2000,"Measurement deadline/event timeout");if(GetTickCount64()-lastControl>100){command=Control(session);lastControl=GetTickCount64();}if(command=="stop"){stopped=true;break;}
  DWORD event=WaitForMultipleObjects(2,events,FALSE,100);Require(event!=WAIT_FAILED,"Wait events");DrainFile(reader.Get(),wave,capture,timestamps);
  if(event==WAIT_OBJECT_0){lastRender=GetTickCount64();UINT32 available=0,frames=0;Check(streams.render->BeginUpdatingAudioObjects(&available,&frames),"BeginUpdatingAudioObjects");Updating update{streams.render.Get()};Require(frames>0&&frames<=Rate/4,"Unexpected spatial quantum");
   if(!object){Require(available>=1,"Dynamic object unavailable");Check(streams.render->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&object),"ActivateSpatialAudioObject(dynamic)");Check(object->SetVolume(1),"object.SetVolume");}
   uint64_t now=Qpc();if(!firstRenderQpc)firstRenderQpc=now;renderTimes<<sent<<','<<now<<'\n';
   BYTE* buffer=nullptr;UINT32 length=0;Check(object->GetBuffer(&buffer,&length),"object.GetBuffer");Require(buffer&&length==frames*4,"Object buffer extent changed");float* output=reinterpret_cast<float*>(buffer);std::fill(output,output+frames,0.0f);
   if(!started&&command=="pause"){paused=true;}
   else{
    if(!started){if(paused){Require(SettingsFingerprint()==fingerprint,"Profile changed while paused");Defaults(devices);SpatialFormat();paused=false;}auto& d=grid[index];d.first=sent;Check(object->SetPosition(d.xyz[0],d.xyz[1],d.xyz[2]),"object.SetPosition");started=true;offset=0;std::cout<<"{\"event\":\"direction\",\"index\":"<<index<<",\"total\":"<<grid.size()<<",\"azimuth\":"<<d.az<<",\"elevation\":"<<d.el<<"}"<<std::endl;}
    size_t count=std::min(static_cast<size_t>(frames),stimulus.size()-offset);std::copy_n(stimulus.data()+offset,count,output);offset+=count;
    if(offset==stimulus.size()){grid[index].end=sent+frames;grid[index].complete=true;index++;started=false;std::cout<<"{\"event\":\"completed\",\"completed\":"<<index<<",\"total\":"<<grid.size()<<"}"<<std::endl;if(index==grid.size())Check(object->SetEndOfStream(frames),"object.SetEndOfStream");}
   }
   sent+=frames;update.End();
  }
 }
 ULONGLONG until=GetTickCount64()+500;while(GetTickCount64()<until){WaitForSingleObject(captureEvent.value,25);DrainFile(reader.Get(),wave,capture,timestamps);}Check(streams.render->Stop(),"render.Stop");streams.renderStarted=false;Check(streams.render->Reset(),"render.Reset own stream");Check(streams.capture->Stop(),"capture.Stop");streams.captureStarted=false;wave.Finish();timestamps.close();renderTimes.close();Defaults(devices);SpatialFormat();Require(SettingsFingerprint()==fingerprint,"Profile changed during measurement");
 captureOk=true;saveCapture();std::cout<<"{\"event\":\"finished\",\"completed\":"<<index<<",\"total\":"<<grid.size()<<"}"<<std::endl;
}
int wmain(int argc,wchar_t** argv){try{
 Require(argc>=2,"Use --devices, --inspect playback recording, or --record session grid stimulus paired|loopback playback recording");ComScope com;HRESULT initialized=RoInitialize(RO_INIT_MULTITHREADED);Check(initialized,"RoInitialize");struct RoScope{~RoScope(){RoUninitialize();}} ro;ComPtr<IMMDeviceEnumerator> devices;Check(CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&devices)),"MMDeviceEnumerator");
 std::wstring command=argv[1];if(command==L"--devices"){Require(argc==2,"Invalid device arguments");Devices(devices.Get());return 0;}
 bool inspect=command==L"--inspect";Require((inspect&&argc==4)||(command==L"--record"&&argc==8),"Invalid capture arguments");Playback=argv[inspect?2:6];Recording=argv[inspect?3:7];if(Recording==L"-")Recording.clear();
 if(Playback==L"default"){ComPtr<IMMDevice> device;Check(devices->GetDefaultAudioEndpoint(eRender,eConsole,&device),"GetDefaultAudioEndpoint");LPWSTR id=nullptr;Check(device->GetId(&id),"GetId");Playback=id;CoTaskMemFree(id);}
 Defaults(devices.Get());SpatialFormat();ExpectedSettings=SettingsFingerprint();
 if(inspect){std::cout<<"{\"event\":\"inspect\",\"provider\":\"Windows Spatial Sound\",\"provider_guid\":"<<Json(ActiveProvider)<<",\"renderer_settings_sha256\":\""<<ExpectedSettings<<"\",\"sample_rate\":48000,\"playback_endpoint\":"<<Json(Playback)<<",\"recording_endpoint\":"<<Json(Recording)<<"}"<<std::endl;return 0;}
 std::wstring mode=argv[5];Require(mode==L"paired"||mode==L"loopback","Invalid capture mode");Require(mode!=L"paired"||!Recording.empty(),"Select a recording endpoint or use playback loopback");ComPtr<IMMDevice> playback;Check(devices->GetDevice(Playback.c_str(),&playback),"GetDevice playback");ComPtr<ISpatialAudioClient> client;Check(playback->Activate(__uuidof(ISpatialAudioClient),CLSCTX_INPROC_SERVER,nullptr,reinterpret_cast<void**>(client.GetAddressOf())),"Activate ISpatialAudioClient");CaptureSphere(devices.Get(),client.Get(),argv[2],argv[3],argv[4],mode==L"loopback");return 0;
 }catch(const std::exception& e){std::cerr<<"FAILED: "<<e.what()<<'\n';return 1;}}
