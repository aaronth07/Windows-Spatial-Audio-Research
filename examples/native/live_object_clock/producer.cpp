#include "protocol.h"
#include <mmdeviceapi.h>
#include <spatialaudioclient.h>
#include <audioclient.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <cstring>
using Microsoft::WRL::ComPtr;
namespace{
void label(char* target,size_t capacity,const char* name){const size_t n=std::min(std::strlen(name),capacity-1);std::memcpy(target,name,n);target[n]=0;}
HRESULT stage(StreamResult& result,HRESULT hr,const char* name){result.result=hr;label(result.stage,sizeof(result.stage),name);return hr;}
HRESULT stage(ProducerResult& result,HRESULT hr,const char* name){result.result=hr;label(result.stage,sizeof(result.stage),name);return hr;}
struct Apartment{bool active=false;~Apartment(){if(active)CoUninitialize();}};
struct Session{
    ComPtr<ISpatialAudioObjectRenderStream> stream;std::array<ComPtr<ISpatialAudioObject>,TapObjects> objects;
    ComPtr<IAudioClock> clock;ComPtr<IAudioClock2> clock2;
    HANDLE event=nullptr;bool started=false,updating=false,done=false,reusePending=false,reused=false;
    uint32_t offset=0,object0Offset=0;StreamResult* result=nullptr;
    ProducerResult* producerResult=nullptr;uint32_t streamIndex=0;uint64_t updateOrdinal=0,submittedUpdateFrames=0;
    void recordClock(ClockPhase phase)noexcept{
        if(producerResult->clockCount>=TapMaxClockRecords){producerResult->clockLogResult=E_BOUNDS;return;}
        ClockRecord record;record.stream=streamIndex;record.phase=phase;record.update=updateOrdinal;record.submittedUpdateFrames=submittedUpdateFrames;
        LARGE_INTEGER qpc{};QueryPerformanceCounter(&qpc);record.hostQpcBefore=qpc.QuadPart;
        record.positionResult=clock?clock->GetPosition(&record.position,&record.positionQpc100ns):result->clockServiceResult;
        record.deviceResult=clock2?clock2->GetDevicePosition(&record.devicePosition,&record.deviceQpc100ns):result->clock2ServiceResult;
        QueryPerformanceCounter(&qpc);record.hostQpcAfter=qpc.QuadPart;
        producerResult->clocks[producerResult->clockCount++]=record;
    }
    void close()noexcept{
        if(stream){if(updating){stream->EndUpdatingAudioObjects();updating=false;}
            if(started){result->stopResult=stream->Stop();started=false;}result->resetResult=stream->Reset();}
        clock2.Reset();clock.Reset();if(result)result->clockInterfacesReleased=1;
        for(auto& object:objects)object.Reset();stream.Reset();if(event){CloseHandle(event);event=nullptr;}
    }
    ~Session(){close();}
};
float sample(uint32_t stream,uint32_t source,uint32_t frame){
    const uint32_t value=(frame+17*source+100003*stream)*1664525u+1013904223u;
    return (float(value>>9)/8388608.f-.5f)*.02f;
}
HRESULT update(Session& session,uint32_t streamIndex,SourceLog& reference){
    auto& result=*session.result;UINT32 available=0,frames=0;
    HRESULT hr=session.stream->BeginUpdatingAudioObjects(&available,&frames);
    if(FAILED(stage(result,hr,"BeginUpdating")))return hr;session.updating=true;session.updateOrdinal++;session.recordClock(AfterBegin);
    if(!frames||frames>result.maxFrames||frames>TapMaxFrameCount)return stage(result,E_BOUNDS,"Update extent");
    if(session.offset==TapTargetFrames){
        uint32_t inactive=0;for(auto& object:session.objects){BOOL active=TRUE;hr=object->IsActive(&active);if(FAILED(stage(result,hr,"EOS IsActive")))return hr;inactive+=active==FALSE;}
        hr=session.stream->EndUpdatingAudioObjects();session.updating=false;if(SUCCEEDED(hr))session.submittedUpdateFrames+=frames;session.recordClock(AfterEnd);if(FAILED(stage(result,hr,"EOS EndUpdating")))return hr;
        result.drainUpdates++;result.eosInactiveObjects=inactive;
        if(inactive==TapObjects){session.done=true;session.close();if(FAILED(result.stopResult))return stage(result,result.stopResult,"Stop");if(FAILED(result.resetResult))return stage(result,result.resetResult,"Reset");return stage(result,S_OK,"complete");}
        return result.drainUpdates>=4?stage(result,HRESULT_FROM_WIN32(ERROR_TIMEOUT),"EOS objects remained active"):S_OK;
    }
    if(!session.objects[1]){
        if(available<TapObjects)return stage(result,HRESULT_FROM_WIN32(ERROR_BUSY),"Initial object availability");
        for(auto& object:session.objects){hr=session.stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&object);if(FAILED(stage(result,hr,"Activate initial object")))return hr;}
    }
    if(session.reusePending){
        BOOL active=TRUE;hr=session.objects[0]->IsActive(&active);if(FAILED(stage(result,hr,"Reuse IsActive")))return hr;
        if(active==FALSE){session.objects[0].Reset();hr=session.stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&session.objects[0]);
            if(FAILED(stage(result,hr,"Recreate object")))return hr;session.reusePending=false;session.reused=true;result.recreatedObjects++;}
    }
    const uint32_t valid=std::min(frames,TapTargetFrames-session.offset),recordStart=reference.count;
    for(uint32_t source=0;source<TapObjects;source++){
        if(source==0&&session.reusePending)continue;
        SourceRecord record;record.streamId=streamIndex+1;record.update=result.updates+1;record.sourceIndex=source;
        record.generation=source==0&&session.reused?1:0;record.frames=frames;record.validFrames=valid;record.type=AudioObjectType_Dynamic;
        const uint32_t sourceOffset=source==0?session.object0Offset:session.offset;
        const bool midpoint=source==0&&!session.reused&&sourceOffset+valid>=TapReuseFrame;
        if(midpoint)record.validFrames=TapReuseFrame-sourceOffset;
        record.eos=midpoint||session.offset+valid==TapTargetFrames;
        record.position[0]=(source?.75f:-.75f)+.05f*float(streamIndex)+.1f*(float(sourceOffset)/float(TapTargetFrames));
        record.position[1]=source?.15f:-.15f;record.position[2]=-1.f-.25f*float(streamIndex);
        record.volume=source?.00075f:.0005f;
        hr=session.objects[source]->SetPosition(record.position[0],record.position[1],record.position[2]);if(FAILED(stage(result,hr,"SetPosition")))return hr;
        hr=session.objects[source]->SetVolume(record.volume);if(FAILED(stage(result,hr,"SetVolume")))return hr;
        BYTE* bytes=nullptr;UINT32 extent=0;hr=session.objects[source]->GetBuffer(&bytes,&extent);if(FAILED(stage(result,hr,"GetBuffer")))return hr;
        if(!bytes||extent!=frames*sizeof(float))return stage(result,E_UNEXPECTED,"Object buffer extent");
        auto* pcm=reinterpret_cast<float*>(bytes);for(uint32_t i=0;i<frames;i++)pcm[i]=i<record.validFrames?sample(streamIndex,source,sourceOffset+i):0;
        if(!appendRecord(reference,record,pcm))return stage(result,E_BOUNDS,"Reference capacity");
        if(record.eos){hr=session.objects[source]->SetEndOfStream(record.validFrames);if(FAILED(stage(result,hr,"SetEndOfStream")))return hr;}
        if(source==0){session.object0Offset+=record.validFrames;if(midpoint)session.reusePending=true;}
    }
    hr=session.stream->EndUpdatingAudioObjects();session.updating=false;
    if(SUCCEEDED(hr))session.submittedUpdateFrames+=frames;session.recordClock(AfterEnd);
    for(uint32_t i=recordStart;i<reference.count;i++)reference.records[i].endResult=hr;
    if(FAILED(stage(result,hr,"EndUpdating")))return hr;
    session.offset+=valid;result.validSourceFrames=session.offset;result.updates++;return S_OK;
}
}
// An ordinary SDK producer: no wrapper IDs, hooks or capture storage knowledge.
extern "C" __declspec(dllexport) HRESULT WINAPI RunSpatialProducer(const wchar_t* endpoint,SourceLog* references,ProducerResult* result){
    if(!endpoint||!*endpoint||!references||!result)return E_POINTER;
    *result={};LARGE_INTEGER qpcFrequency{};if(!QueryPerformanceFrequency(&qpcFrequency))return stage(*result,E_FAIL,"QPC frequency");result->hostQpcFrequency=qpcFrequency.QuadPart;
    Apartment apartment;HRESULT hr=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
    if(FAILED(stage(*result,hr,"CoInitializeEx")))return hr;apartment.active=true;
    ComPtr<IMMDeviceEnumerator> enumerator;ComPtr<IMMDevice> device;ComPtr<ISpatialAudioClient> client;
    hr=CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator));if(FAILED(stage(*result,hr,"CoCreateInstance")))return hr;
    hr=enumerator->GetDevice(endpoint,&device);if(FAILED(stage(*result,hr,"GetDevice")))return hr;
    hr=device->Activate(__uuidof(ISpatialAudioClient),CLSCTX_INPROC_SERVER,nullptr,&client);if(FAILED(stage(*result,hr,"Activate client")))return hr;
    hr=client->GetMaxDynamicObjectCount(&result->maxDynamic);if(FAILED(stage(*result,hr,"Maximum dynamic objects")))return hr;
    if(result->maxDynamic<TapObjects*TapStreams)return stage(*result,HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED),"Insufficient dynamic capacity");
    WAVEFORMATEX format{WAVE_FORMAT_IEEE_FLOAT,1,TapRate,TapRate*4,4,32,0};result->rate=TapRate;
    hr=client->IsAudioObjectFormatSupported(&format);if(hr!=S_OK)return stage(*result,FAILED(hr)?hr:AUDCLNT_E_UNSUPPORTED_FORMAT,"Float48k mono unsupported");
    std::array<Session,TapStreams> sessions;
    for(uint32_t i=0;i<TapStreams;i++){
        auto& session=sessions[i];session.result=&result->streams[i];session.producerResult=result;session.streamIndex=i;
        hr=client->GetMaxFrameCount(&format,&session.result->maxFrames);if(FAILED(stage(*result,hr,"GetMaxFrameCount")))return hr;
        if(!session.result->maxFrames||session.result->maxFrames>TapMaxFrameCount)return stage(*result,E_BOUNDS,"Maximum frames bound");
        session.event=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!session.event)return stage(*result,HRESULT_FROM_WIN32(GetLastError()),"CreateEvent");
        SpatialAudioObjectRenderStreamActivationParams activation{&format,AudioObjectType_None,TapObjects,TapObjects,AudioCategory_GameEffects,session.event,nullptr};
        PROPVARIANT parameters{};parameters.vt=VT_BLOB;parameters.blob.cbSize=sizeof(activation);parameters.blob.pBlobData=reinterpret_cast<BYTE*>(&activation);
        hr=client->ActivateSpatialAudioStream(&parameters,__uuidof(ISpatialAudioObjectRenderStream),&session.stream);if(FAILED(stage(*result,hr,"Activate actual stream")))return hr;
        session.result->clockServiceResult=session.stream->GetService(__uuidof(IAudioClock),reinterpret_cast<void**>(session.clock.GetAddressOf()));
        session.result->clock2ServiceResult=session.stream->GetService(__uuidof(IAudioClock2),reinterpret_cast<void**>(session.clock2.GetAddressOf()));
        if(session.clock)session.result->frequencyResult=session.clock->GetFrequency(&session.result->clockFrequency);
        else session.result->frequencyResult=session.result->clockServiceResult;
        session.recordClock(BeforeStart);
    }
    for(auto& session:sessions){hr=session.stream->Start();if(FAILED(stage(*result,hr,"Start")))return hr;session.started=true;session.recordClock(AfterStart);}
    const ULONGLONG start=GetTickCount64();
    for(;;){
        HANDLE events[TapStreams];uint32_t indices[TapStreams],count=0;
        for(uint32_t i=0;i<TapStreams;i++)if(!sessions[i].done){events[count]=sessions[i].event;indices[count++]=i;}
        if(!count)break;if(GetTickCount64()-start>1900)return stage(*result,HRESULT_FROM_WIN32(ERROR_TIMEOUT),"Global 1.9 second bound");
        result->lastWait=WaitForMultipleObjects(count,events,FALSE,200);
        if(result->lastWait>=WAIT_OBJECT_0+count)return stage(*result,HRESULT_FROM_WIN32(result->lastWait==WAIT_TIMEOUT?ERROR_TIMEOUT:GetLastError()),"WaitForMultipleObjects");
        const uint32_t index=indices[result->lastWait-WAIT_OBJECT_0];hr=update(sessions[index],index,references[index]);
        if(FAILED(hr))return stage(*result,hr,result->streams[index].stage);
    }
    for(auto& stream:result->streams)if(stream.recreatedObjects!=1)return stage(*result,E_UNEXPECTED,"Object reuse not completed");
    return stage(*result,S_OK,"complete");
}
