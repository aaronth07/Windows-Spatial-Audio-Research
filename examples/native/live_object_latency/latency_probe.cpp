#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <spatialaudioclient.h>
#include <audioclient.h>
#include <ksmedia.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <new>
#include <random>
#include <sstream>
#include <string>
#include <vector>
using Microsoft::WRL::ComPtr;namespace fs=std::filesystem;
constexpr uint32_t Rate=48000,NoiseFrames=9600,SourceFrames=33600,MaxCaptureFrames=72000,MaxRecords=1024;
constexpr uint32_t Seeds[2]{0x243f6a88u,0xb7e15162u};constexpr float Gain=.001f;
thread_local bool tracking=false;thread_local uint64_t allocations=0,deallocations=0;
void* operator new(size_t n){if(tracking)allocations++;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{if(tracking&&p)deallocations++;std::free(p);}
void* operator new[](size_t n){return ::operator new(n);}void operator delete[](void* p)noexcept{::operator delete(p);}
void operator delete(void* p,size_t)noexcept{::operator delete(p);}void operator delete[](void* p,size_t)noexcept{::operator delete(p);}
struct Failure:std::exception{HRESULT hr;const char* label;Failure(HRESULT h,const char* s):hr(h),label(s){}const char* what()const noexcept override{return label;}};
static void check(HRESULT hr,const char* label){if(FAILED(hr))throw Failure(hr,label);}
static void require(bool condition,const char* label){if(!condition)throw Failure(E_UNEXPECTED,label);}
static int64_t qpc(){LARGE_INTEGER value{};require(QueryPerformanceCounter(&value)!=FALSE,"QueryPerformanceCounter");return value.QuadPart;}
static std::string quoted(const std::string& text){std::string result="\"";for(unsigned char c:text){if(c=='\\'||c=='\"'){result+='\\';result+=char(c);}else result+=c<32?'?':char(c);}return result+'\"';}
static std::string utf8(const std::wstring& value){const int n=WideCharToMultiByte(CP_UTF8,0,value.c_str(),int(value.size()),nullptr,0,nullptr,nullptr);std::string text(size_t(n),' ');if(n)WideCharToMultiByte(CP_UTF8,0,value.c_str(),int(value.size()),text.data(),n,nullptr,nullptr);return text;}
static void writeNew(const fs::path& path,const void* data,size_t bytes){require(bytes<=MAXDWORD,"Output extent");HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);require(file!=INVALID_HANDLE_VALUE,"Output exists or cannot be created");DWORD written=0;const BOOL ok=WriteFile(file,data,DWORD(bytes),&written,nullptr);const BOOL closed=CloseHandle(file);require(ok&&written==bytes&&closed,"Output write");}
static void writeText(const fs::path& path,const std::string& value){writeNew(path,value.data(),value.size());}
struct Endpoint{ComPtr<IMMDevice> device;std::wstring id,name;};
static Endpoint endpoint(){
    ComPtr<IMMDeviceEnumerator> enumerator;ComPtr<IMMDevice> device;
    check(CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator)),"Endpoint enumerator");
    check(enumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&device),"Default render endpoint");
    LPWSTR id=nullptr;check(device->GetId(&id),"Endpoint identity");
    Endpoint result;result.id=id;result.name=L"default render endpoint";CoTaskMemFree(id);
    result.device=device;
    return result;
}
struct Packet{uint32_t frames=0,frameOffset=0;DWORD flags=0;uint64_t device=0,qpc100ns=0;int64_t hostBefore=0,hostAfter=0;HRESULT getResult=E_PENDING,releaseResult=E_PENDING;};
struct Capture{
    ComPtr<IAudioClient> client;ComPtr<IAudioCaptureClient> reader;HANDLE event=nullptr;WAVEFORMATEX* format=nullptr;bool started=false;
    std::vector<float> samples;std::array<Packet,MaxRecords> packets;uint32_t packetCount=0,frameCount=0,bufferFrames=0;HRESULT stopResult=S_FALSE,resetResult=S_FALSE;
    void initialize(IMMDevice* device){
        check(device->Activate(__uuidof(IAudioClient),CLSCTX_INPROC_SERVER,nullptr,&client),"Loopback client");check(client->GetMixFormat(&format),"Loopback mix format");
        const bool floating=format->wFormatTag==WAVE_FORMAT_IEEE_FLOAT||(format->wFormatTag==WAVE_FORMAT_EXTENSIBLE&&format->cbSize>=22&&reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format)->SubFormat==KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        require(floating&&format->nSamplesPerSec==Rate&&format->nChannels==2&&format->wBitsPerSample==32&&format->nBlockAlign==8,"Loopback requires existing stereo float32/48k mix");
        samples.resize(size_t(MaxCaptureFrames)*2);
        check(client->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_LOOPBACK|AUDCLNT_STREAMFLAGS_EVENTCALLBACK,1000000,0,format,nullptr),"Loopback Initialize");
        event=CreateEventW(nullptr,FALSE,FALSE,nullptr);require(event!=nullptr,"Loopback event");check(client->SetEventHandle(event),"Loopback SetEventHandle");
        check(client->GetBufferSize(&bufferFrames),"Loopback buffer size");check(client->GetService(IID_PPV_ARGS(&reader)),"Loopback capture service");
    }
    void drain(){
        UINT32 next=0;check(reader->GetNextPacketSize(&next),"GetNextPacketSize");while(next){
            Packet record;BYTE* data=nullptr;record.hostBefore=qpc();record.getResult=reader->GetBuffer(&data,&record.frames,&record.flags,&record.device,&record.qpc100ns);record.hostAfter=qpc();check(record.getResult,"Loopback GetBuffer");
            if(!record.frames)break;
            if(packetCount>=MaxRecords||record.frames>MaxCaptureFrames-frameCount){reader->ReleaseBuffer(record.frames);throw Failure(E_BOUNDS,"Loopback capacity");}
            if(!(record.flags&AUDCLNT_BUFFERFLAGS_SILENT)&&!data){reader->ReleaseBuffer(record.frames);throw Failure(E_POINTER,"Loopback data");}
            record.frameOffset=frameCount;auto* output=samples.data()+size_t(frameCount)*2;
            if(record.flags&AUDCLNT_BUFFERFLAGS_SILENT)std::fill_n(output,size_t(record.frames)*2,0.f);else std::memcpy(output,data,size_t(record.frames)*8);
            record.releaseResult=reader->ReleaseBuffer(record.frames);packets[packetCount++]=record;frameCount+=record.frames;check(record.releaseResult,"Loopback ReleaseBuffer");check(reader->GetNextPacketSize(&next),"GetNextPacketSize");
        }
    }
    void close()noexcept{if(client){if(started){stopResult=client->Stop();started=false;}resetResult=client->Reset();}reader.Reset();client.Reset();if(event){CloseHandle(event);event=nullptr;}if(format){CoTaskMemFree(format);format=nullptr;}}
    ~Capture(){close();}
};
struct ClockRecord{uint32_t phase=0,update=0;uint64_t position=0,positionQpc=0,device=0,deviceQpc=0;int64_t hostBefore=0,hostAfter=0;HRESULT positionResult=E_PENDING,deviceResult=E_PENDING;};
struct UpdateRecord{uint32_t update=0,frameOffset=0,frames=0,valid=0,eos=0;int64_t beginBefore=0,beginAfter=0,endBefore=0,endAfter=0;HRESULT endResult=E_PENDING;};
struct Spatial{
    ComPtr<ISpatialAudioObjectRenderStream> stream;ComPtr<ISpatialAudioObject> object;ComPtr<IAudioClock> clock;ComPtr<IAudioClock2> clock2;HANDLE event=nullptr;
    bool started=false,updating=false,done=false;uint32_t maxFrames=0,offset=0,updateCount=0,clockCount=0,drainUpdates=0,inactive=0;uint64_t frequency=0;
    HRESULT clockService=E_PENDING,clock2Service=E_PENDING,frequencyResult=E_PENDING,stopResult=S_FALSE,resetResult=S_FALSE;
    std::array<float,NoiseFrames> pn;std::array<UpdateRecord,MaxRecords> updates;std::array<ClockRecord,MaxRecords> clocks;
    void prepare(uint32_t seed){std::mt19937 random(seed);for(auto& value:pn)value=(float(random()>>9)/8388608.f-.5f)*.02f;}
    void recordClock(uint32_t phase,uint32_t update){require(clockCount<MaxRecords,"Clock capacity");ClockRecord r;r.phase=phase;r.update=update;r.hostBefore=qpc();r.positionResult=clock?clock->GetPosition(&r.position,&r.positionQpc):clockService;r.deviceResult=clock2?clock2->GetDevicePosition(&r.device,&r.deviceQpc):clock2Service;r.hostAfter=qpc();clocks[clockCount++]=r;}
    void initialize(ISpatialAudioClient* client){
        WAVEFORMATEX format{WAVE_FORMAT_IEEE_FLOAT,1,Rate,Rate*4,4,32,0};const HRESULT supported=client->IsAudioObjectFormatSupported(&format);require(supported==S_OK,"Spatial float48k support");
        check(client->GetMaxFrameCount(&format,&maxFrames),"Spatial max frames");require(maxFrames>0&&maxFrames<=8192,"Spatial frame bound");event=CreateEventW(nullptr,FALSE,FALSE,nullptr);require(event!=nullptr,"Spatial event");
        SpatialAudioObjectRenderStreamActivationParams activation{&format,AudioObjectType_None,1,1,AudioCategory_GameEffects,event,nullptr};PROPVARIANT parameters{};parameters.vt=VT_BLOB;parameters.blob.cbSize=sizeof(activation);parameters.blob.pBlobData=reinterpret_cast<BYTE*>(&activation);
        check(client->ActivateSpatialAudioStream(&parameters,__uuidof(ISpatialAudioObjectRenderStream),&stream),"Activate spatial stream");clockService=stream->GetService(__uuidof(IAudioClock),reinterpret_cast<void**>(clock.GetAddressOf()));clock2Service=stream->GetService(__uuidof(IAudioClock2),reinterpret_cast<void**>(clock2.GetAddressOf()));frequencyResult=clock?clock->GetFrequency(&frequency):clockService;recordClock(0,0);
    }
    void update(){
        UpdateRecord r;r.update=updateCount+1;r.frameOffset=offset;r.beginBefore=qpc();UINT32 available=0,frames=0;check(stream->BeginUpdatingAudioObjects(&available,&frames),"Spatial Begin");updating=true;r.beginAfter=qpc();require(frames>0&&frames<=maxFrames,"Spatial update frames");r.frames=frames;recordClock(2,r.update);
        if(offset==SourceFrames){BOOL active=TRUE;check(object->IsActive(&active),"EOS IsActive");inactive=active==FALSE;drainUpdates++;r.endBefore=qpc();r.endResult=stream->EndUpdatingAudioObjects();updating=false;r.endAfter=qpc();check(r.endResult,"EOS End");recordClock(3,r.update);require(updateCount<MaxRecords,"Update capacity");updates[updateCount++]=r;
            if(inactive){done=true;close();}else require(drainUpdates<4,"EOS inactive timeout");return;}
        if(!object){require(available>=1,"Spatial availability");check(stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&object),"Activate spatial object");}
        check(object->SetPosition(0,0,-1),"SetPosition");check(object->SetVolume(Gain),"SetVolume");BYTE* data=nullptr;UINT32 bytes=0;check(object->GetBuffer(&data,&bytes),"Object GetBuffer");require(data&&bytes==frames*4,"Object extent");r.valid=std::min(frames,SourceFrames-offset);r.eos=offset+r.valid==SourceFrames;
        auto* output=reinterpret_cast<float*>(data);for(uint32_t i=0;i<frames;i++)output[i]=i<r.valid&&offset+i<NoiseFrames?pn[offset+i]:0.f;
        if(r.eos)check(object->SetEndOfStream(r.valid),"SetEndOfStream");r.endBefore=qpc();r.endResult=stream->EndUpdatingAudioObjects();updating=false;r.endAfter=qpc();check(r.endResult,"Spatial End");recordClock(3,r.update);require(updateCount<MaxRecords,"Update capacity");updates[updateCount++]=r;offset+=r.valid;
    }
    void close()noexcept{if(stream){if(updating){stream->EndUpdatingAudioObjects();updating=false;}if(started){stopResult=stream->Stop();started=false;}resetResult=stream->Reset();}clock2.Reset();clock.Reset();object.Reset();stream.Reset();if(event){CloseHandle(event);event=nullptr;}}
    ~Spatial(){close();}
};
static void writeClocks(const fs::path& output,const std::array<Spatial,2>& sources){std::ostringstream json;json<<"{\"records\":[";bool first=true;for(uint32_t i=0;i<2;i++)for(uint32_t j=0;j<sources[i].clockCount;j++){const auto& r=sources[i].clocks[j];if(!first)json<<',';first=false;json<<"{\"stream\":"<<i<<",\"phase\":"<<r.phase<<",\"update\":"<<r.update<<",\"position_hresult\":"<<uint32_t(r.positionResult)<<",\"position\":"<<r.position<<",\"qpc_100ns\":"<<r.positionQpc<<",\"device_hresult\":"<<uint32_t(r.deviceResult)<<",\"device_position\":"<<r.device<<",\"device_qpc_100ns\":"<<r.deviceQpc<<",\"host_before\":"<<r.hostBefore<<",\"host_after\":"<<r.hostAfter<<'}';}json<<"]}\n";writeText(output/L"clocks.json",json.str());}
static void writeUpdates(const fs::path& output,const std::array<Spatial,2>& sources){std::ostringstream json;json<<"{\"records\":[";bool first=true;for(uint32_t i=0;i<2;i++)for(uint32_t j=0;j<sources[i].updateCount;j++){const auto& r=sources[i].updates[j];if(!first)json<<',';first=false;json<<"{\"stream\":"<<i<<",\"update\":"<<r.update<<",\"source_frame_offset\":"<<r.frameOffset<<",\"frames\":"<<r.frames<<",\"valid_frames\":"<<r.valid<<",\"eos\":"<<r.eos<<",\"begin_before\":"<<r.beginBefore<<",\"begin_after\":"<<r.beginAfter<<",\"end_before\":"<<r.endBefore<<",\"end_after\":"<<r.endAfter<<",\"end_hresult\":"<<uint32_t(r.endResult)<<'}';}json<<"]}\n";writeText(output/L"updates.json",json.str());}
static void writePackets(const fs::path& output,const Capture& capture){std::ostringstream json;json<<"{\"records\":[";for(uint32_t i=0;i<capture.packetCount;i++){const auto& p=capture.packets[i];if(i)json<<',';json<<"{\"frame_offset\":"<<p.frameOffset<<",\"frames\":"<<p.frames<<",\"flags\":"<<p.flags<<",\"device_position\":"<<p.device<<",\"qpc_100ns\":"<<p.qpc100ns<<",\"host_before\":"<<p.hostBefore<<",\"host_after\":"<<p.hostAfter<<",\"get_hresult\":"<<uint32_t(p.getResult)<<",\"release_hresult\":"<<uint32_t(p.releaseResult)<<'}';}json<<"]}\n";writeText(output/L"packets.json",json.str());}
static int selfTest(){Spatial source;source.prepare(Seeds[0]);std::mt19937 known(5489u);const uint32_t expected[5]{3499211612u,581869302u,3890346734u,3586334585u,545404204u};for(auto value:expected)require(known()==value,"MT19937 known sequence");require(std::all_of(source.pn.begin(),source.pn.end(),[](float x){return std::isfinite(x)&&std::abs(x)<=.01f;}),"PN finite bound");Spatial second;second.prepare(Seeds[1]);require(source.pn!=second.pn,"Independent seeds");double power=0,cross=0,power2=0;for(size_t i=0;i<NoiseFrames;i++){power+=source.pn[i]*source.pn[i];power2+=second.pn[i]*second.pn[i];cross+=source.pn[i]*second.pn[i];}require(power>0&&power2>0&&std::abs(cross/std::sqrt(power*power2))<.1,"PN correlation");require(SourceFrames-NoiseFrames==24000&&MaxCaptureFrames==72000,"Duration bounds");std::cout<<"{\"passed\":true,\"native_offline_cases\":5,\"audio_streams_started\":0}\n";return 0;}
int wmain(int argc,wchar_t** argv){
    if(argc==2&&std::wstring(argv[1])==L"--self-test"){try{return selfTest();}catch(const std::exception& e){std::cerr<<e.what();return 1;}}
    if(argc!=2){std::cerr<<"Use latency_probe NEW-output-directory or --self-test\n";return 2;}
    const fs::path output=fs::absolute(argv[1]);bool made=false,apartment=false,success=false;
    try{require(fs::create_directory(output),"Output directory exists");made=true;check(CoInitializeEx(nullptr,COINIT_MULTITHREADED),"CoInitializeEx");apartment=true;
        {const auto selected=endpoint();Capture capture;capture.initialize(selected.device.Get());ComPtr<ISpatialAudioClient> client;check(selected.device->Activate(__uuidof(ISpatialAudioClient),CLSCTX_INPROC_SERVER,nullptr,&client),"Spatial client");UINT32 maximum=0;check(client->GetMaxDynamicObjectCount(&maximum),"Dynamic capacity");require(maximum>=2,"Two dynamic objects required");
            std::array<Spatial,2> sources;for(uint32_t i=0;i<2;i++){sources[i].prepare(Seeds[i]);sources[i].initialize(client.Get());}
            LARGE_INTEGER frequency{};require(QueryPerformanceFrequency(&frequency)!=FALSE,"QPC frequency");check(capture.client->Start(),"Loopback Start");capture.started=true;const int64_t start=qpc();for(auto& source:sources){check(source.stream->Start(),"Spatial Start");source.started=true;source.recordClock(1,0);}
            tracking=true;while((qpc()-start)*1000/frequency.QuadPart<1200){capture.drain();HANDLE events[3]{capture.event};uint32_t mapping[3]{2,0,0},count=1;for(uint32_t i=0;i<2;i++)if(!sources[i].done){events[count]=sources[i].event;mapping[count++]=i;}
                const DWORD wait=WaitForMultipleObjects(count,events,FALSE,20);if(wait==WAIT_TIMEOUT)continue;require(wait<WAIT_OBJECT_0+count,"Wait events");const uint32_t index=mapping[wait-WAIT_OBJECT_0];if(index<2)sources[index].update();}
            capture.drain();tracking=false;const double elapsed=double(qpc()-start)/double(frequency.QuadPart);bool complete=true;for(auto& source:sources){complete&=source.done&&source.inactive&&source.offset==SourceFrames;source.close();complete&=SUCCEEDED(source.stopResult)&&SUCCEEDED(source.resetResult);}capture.stopResult=capture.client->Stop();capture.started=false;capture.resetResult=capture.client->Reset();
            bool finite=std::all_of(capture.samples.begin(),capture.samples.begin()+size_t(capture.frameCount)*2,[](float x){return std::isfinite(x);});const bool passed=complete&&finite&&capture.packetCount>0&&capture.frameCount>NoiseFrames&&SUCCEEDED(capture.stopResult)&&SUCCEEDED(capture.resetResult)&&allocations==0&&deallocations==0&&elapsed<1.5;
            writeNew(output/L"loopback.f32",capture.samples.data(),size_t(capture.frameCount)*8);for(uint32_t i=0;i<2;i++)writeNew(output/(L"source"+std::to_wstring(i)+L".f32"),sources[i].pn.data(),NoiseFrames*4);writeClocks(output,sources);writeUpdates(output,sources);writePackets(output,capture);
            std::ostringstream json;json<<std::setprecision(12)<<"{\"native_passed\":"<<(passed?"true":"false")<<",\"endpoint_name\":"<<quoted(utf8(selected.name))<<",\"endpoint_id\":"<<quoted("omitted")<<",\"rate\":48000,\"channels\":2,\"noise_frames\":9600,\"source_frames\":33600,\"gain\":"<<Gain<<",\"windows_position\":[0,0,-1],\"host_qpc_frequency\":"<<frequency.QuadPart<<",\"loopback_start_host_qpc\":"<<start<<",\"loopback_buffer_frames\":"<<capture.bufferFrames<<",\"captured_frames\":"<<capture.frameCount<<",\"packets\":"<<capture.packetCount<<",\"capture_stop_hresult\":"<<uint32_t(capture.stopResult)<<",\"capture_reset_hresult\":"<<uint32_t(capture.resetResult)<<",\"finite\":"<<(finite?"true":"false")<<",\"host_loop_operator_new\":"<<allocations<<",\"host_loop_operator_delete\":"<<deallocations<<",\"elapsed_seconds\":"<<elapsed<<",\"streams\":[";
            for(uint32_t i=0;i<2;i++){const auto& s=sources[i];if(i)json<<',';json<<"{\"stream\":"<<i<<",\"seed\":"<<Seeds[i]<<",\"clock_service_hresult\":"<<uint32_t(s.clockService)<<",\"clock2_service_hresult\":"<<uint32_t(s.clock2Service)<<",\"frequency_hresult\":"<<uint32_t(s.frequencyResult)<<",\"clock_frequency\":"<<s.frequency<<",\"source_frames\":"<<s.offset<<",\"update_count\":"<<s.updateCount<<",\"inactive_after_eos\":"<<s.inactive<<",\"stop_hresult\":"<<uint32_t(s.stopResult)<<",\"reset_hresult\":"<<uint32_t(s.resetResult)<<'}';}json<<"]}\n";writeText(output/L"result.json",json.str());std::cout<<json.str();success=passed;
        }CoUninitialize();apartment=false;return success?0:1;
    }catch(const Failure& e){tracking=false;if(apartment)CoUninitialize();if(made){std::ostringstream json;json<<"{\"native_passed\":false,\"live_part_stopped\":true,\"hresult\":"<<uint32_t(e.hr)<<",\"stage\":"<<quoted(e.what())<<"}\n";try{writeText(output/L"failure.json",json.str());}catch(...){} }std::cerr<<e.what()<<" HRESULT="<<std::hex<<uint32_t(e.hr)<<'\n';return 1;
    }catch(const std::exception& e){tracking=false;if(apartment)CoUninitialize();std::cerr<<e.what()<<'\n';return 1;}
}
