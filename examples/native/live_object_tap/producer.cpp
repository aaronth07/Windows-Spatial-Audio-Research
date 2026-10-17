#include "protocol.h"
#include <mmdeviceapi.h>
#include <spatialaudioclient.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <cstring>

using Microsoft::WRL::ComPtr;
namespace {
struct Apartment {bool active=false;~Apartment(){if(active)CoUninitialize();}};
struct Session {
    ComPtr<ISpatialAudioObjectRenderStream> stream;
    std::array<ComPtr<ISpatialAudioObject>,TapObjects> objects;
    HANDLE event=nullptr;bool started=false,updating=false;ProducerResult* result=nullptr;
    void close() noexcept {
        if(stream){
            if(updating){stream->EndUpdatingAudioObjects();updating=false;}
            if(started){result->stopResult=stream->Stop();started=false;}
            result->resetResult=stream->Reset();
        }
        for(auto& object:objects)object.Reset();stream.Reset();
        if(event){CloseHandle(event);event=nullptr;}
    }
    ~Session(){close();}
};
HRESULT stage(ProducerResult& r,HRESULT hr,const char* name) noexcept {
    r.result=hr;const size_t length=std::min(std::strlen(name),sizeof(r.stage)-1);
    std::memcpy(r.stage,name,length);r.stage[length]=0;return hr;
}
float sample(uint32_t object,uint32_t frame) noexcept {
    const uint32_t value=(frame+17*object)*1664525u+1013904223u;
    return (float(value>>9)/8388608.f-.5f)*.02f;
}
}

// This ordinary producer uses only documented SDK calls. It has no knowledge
// of the host's wrappers, capture IDs, import edit or capture-buffer addresses.
extern "C" __declspec(dllexport) HRESULT WINAPI RunSpatialProducer(const wchar_t* endpoint,SourceLog* reference,ProducerResult* result) {
    if(!endpoint||!*endpoint||!reference||!result)return E_POINTER;
    *result={};Apartment apartment;HRESULT hr=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
    if(FAILED(stage(*result,hr,"CoInitializeEx")))return hr;apartment.active=true;
    ComPtr<IMMDeviceEnumerator> enumerator;ComPtr<IMMDevice> device;ComPtr<ISpatialAudioClient> client;
    hr=CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator));
    if(FAILED(stage(*result,hr,"CoCreateInstance")))return hr;
    hr=enumerator->GetDevice(endpoint,&device);if(FAILED(stage(*result,hr,"GetDevice")))return hr;
    hr=device->Activate(__uuidof(ISpatialAudioClient),CLSCTX_INPROC_SERVER,nullptr,&client);
    if(FAILED(stage(*result,hr,"Activate spatial client")))return hr;
    hr=client->GetMaxDynamicObjectCount(&result->maxDynamic);if(FAILED(stage(*result,hr,"GetMaxDynamicObjectCount")))return hr;
    if(result->maxDynamic<TapObjects)return stage(*result,HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED),"Fewer than two dynamic objects");
    WAVEFORMATEX format{WAVE_FORMAT_IEEE_FLOAT,1,TapRate,TapRate*4,4,32,0};result->rate=TapRate;
    hr=client->IsAudioObjectFormatSupported(&format);if(hr!=S_OK)return stage(*result,FAILED(hr)?hr:AUDCLNT_E_UNSUPPORTED_FORMAT,"Float48k mono unsupported");
    hr=client->GetMaxFrameCount(&format,&result->maxFrames);if(FAILED(stage(*result,hr,"GetMaxFrameCount")))return hr;
    if(!result->maxFrames||result->maxFrames>TapMaxFrameCount)return stage(*result,E_BOUNDS,"Maximum frame count exceeds bound");
    Session session;session.result=result;session.event=CreateEventW(nullptr,FALSE,FALSE,nullptr);
    if(!session.event)return stage(*result,HRESULT_FROM_WIN32(GetLastError()),"Create event");
    SpatialAudioObjectRenderStreamActivationParams activation{&format,AudioObjectType_None,TapObjects,TapObjects,AudioCategory_GameEffects,session.event,nullptr};
    PROPVARIANT parameters{};parameters.vt=VT_BLOB;parameters.blob.cbSize=sizeof(activation);parameters.blob.pBlobData=reinterpret_cast<BYTE*>(&activation);
    hr=client->ActivateSpatialAudioStream(&parameters,__uuidof(ISpatialAudioObjectRenderStream),&session.stream);
    if(FAILED(stage(*result,hr,"Activate real spatial stream")))return hr;
    hr=session.stream->Start();if(FAILED(stage(*result,hr,"Start")))return hr;session.started=true;
    uint32_t offset=0;
    while(offset<TapTargetFrames){
        result->lastWait=WaitForSingleObject(session.event,200);
        if(result->lastWait!=WAIT_OBJECT_0)return stage(*result,HRESULT_FROM_WIN32(result->lastWait==WAIT_TIMEOUT?ERROR_TIMEOUT:GetLastError()),"Wait event");
        UINT32 available=0,frames=0;hr=session.stream->BeginUpdatingAudioObjects(&available,&frames);
        if(FAILED(stage(*result,hr,"BeginUpdatingAudioObjects")))return hr;session.updating=true;
        if(!frames||frames>result->maxFrames||frames>TapMaxFrameCount)return stage(*result,E_BOUNDS,"Invalid update frame count");
        if(!session.objects[0]){
            if(available<TapObjects)return stage(*result,HRESULT_FROM_WIN32(ERROR_BUSY),"Insufficient available dynamic objects");
            for(auto& object:session.objects){hr=session.stream->ActivateSpatialAudioObject(AudioObjectType_Dynamic,&object);if(FAILED(stage(*result,hr,"Activate dynamic object")))return hr;}
        }
        const uint32_t valid=std::min(frames,TapTargetFrames-offset);const bool eos=offset+valid==TapTargetFrames;
        const uint32_t recordStart=reference->count;
        for(uint32_t object=0;object<TapObjects;object++){
            SourceRecord record;record.update=result->updates+1;record.sourceIndex=object;record.frames=frames;record.validFrames=valid;
            record.eos=eos;record.type=AudioObjectType_Dynamic;
            record.position[0]=(object?.75f:-.75f)+.1f*(float(offset)/float(TapTargetFrames));
            record.position[1]=object?.15f:-.15f;record.position[2]=-1;
            record.volume=object?.00075f:.0005f;
            hr=session.objects[object]->SetPosition(record.position[0],record.position[1],record.position[2]);if(FAILED(stage(*result,hr,"SetPosition")))return hr;
            hr=session.objects[object]->SetVolume(record.volume);if(FAILED(stage(*result,hr,"SetVolume")))return hr;
            BYTE* bytes=nullptr;UINT32 size=0;hr=session.objects[object]->GetBuffer(&bytes,&size);if(FAILED(stage(*result,hr,"GetBuffer")))return hr;
            if(!bytes||size!=frames*sizeof(float))return stage(*result,E_UNEXPECTED,"Actual object buffer extent");
            auto* pcm=reinterpret_cast<float*>(bytes);
            for(uint32_t i=0;i<frames;i++)pcm[i]=i<valid?sample(object,offset+i):0;
            if(!appendRecord(*reference,record,pcm))return stage(*result,E_BOUNDS,"Reference capture capacity");
            if(eos){hr=session.objects[object]->SetEndOfStream(valid);if(FAILED(stage(*result,hr,"SetEndOfStream")))return hr;}
        }
        hr=session.stream->EndUpdatingAudioObjects();session.updating=false;
        for(uint32_t i=recordStart;i<reference->count;i++)reference->records[i].endResult=hr;
        if(FAILED(stage(*result,hr,"EndUpdatingAudioObjects")))return hr;
        offset+=valid;result->validSourceFrames=offset;result->updates++;
    }
    for(uint32_t attempt=0;attempt<4;attempt++){
        result->lastWait=WaitForSingleObject(session.event,200);
        if(result->lastWait!=WAIT_OBJECT_0)return stage(*result,HRESULT_FROM_WIN32(ERROR_TIMEOUT),"EOS drain event");
        UINT32 available=0,frames=0;hr=session.stream->BeginUpdatingAudioObjects(&available,&frames);
        if(FAILED(stage(*result,hr,"EOS BeginUpdating")))return hr;session.updating=true;
        uint32_t inactive=0;for(auto& object:session.objects){BOOL active=TRUE;hr=object->IsActive(&active);if(FAILED(stage(*result,hr,"IsActive after EOS")))return hr;inactive+=active==FALSE;}
        hr=session.stream->EndUpdatingAudioObjects();session.updating=false;if(FAILED(stage(*result,hr,"EOS EndUpdating")))return hr;
        result->drainUpdates++;result->eosInactiveObjects=inactive;if(inactive==TapObjects)break;
    }
    if(result->eosInactiveObjects!=TapObjects)return stage(*result,HRESULT_FROM_WIN32(ERROR_TIMEOUT),"EOS objects remained active");
    session.close();if(FAILED(result->stopResult))return stage(*result,result->stopResult,"Stop");
    if(FAILED(result->resetResult))return stage(*result,result->resetResult,"Reset");
    return stage(*result,S_OK,"complete");
}
