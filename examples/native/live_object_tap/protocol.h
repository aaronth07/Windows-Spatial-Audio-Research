#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>

constexpr uint32_t TapRate=48000,TapObjects=2,TapTargetFrames=48000;
constexpr uint32_t TapMaxFrameCount=8192,TapMaxRecords=1024,TapMaxSamples=131072;
struct SourceRecord {
    uint64_t update=0,id=0;
    uint32_t sourceIndex=0,frames=0,validFrames=0,eos=0,type=0;
    float position[3]{},volume=1;
    uint64_t sampleOffset=0;
    HRESULT endResult=E_PENDING;
};
struct SourceLog {
    SourceRecord* records=nullptr;uint32_t capacity=0,count=0;
    float* samples=nullptr;uint32_t sampleCapacity=0,sampleCount=0;
};
struct ProducerResult {
    HRESULT result=E_PENDING,stopResult=S_FALSE,resetResult=S_FALSE;
    char stage[64]{};
    uint32_t rate=0,maxFrames=0,maxDynamic=0,updates=0,drainUpdates=0;
    uint32_t validSourceFrames=0,eosInactiveObjects=0;
    DWORD lastWait=0;
};
using RunProducer=HRESULT(WINAPI*)(const wchar_t*,SourceLog*,ProducerResult*);
inline bool appendRecord(SourceLog& log,const SourceRecord& record,const float* pcm) noexcept {
    if(!log.records||!log.samples||!pcm||log.count>=log.capacity||log.sampleCount>log.sampleCapacity||record.frames>log.sampleCapacity-log.sampleCount)return false;
    auto r=record;r.sampleOffset=log.sampleCount;
    for(uint32_t i=0;i<record.frames;i++)log.samples[log.sampleCount+i]=pcm[i];
    log.sampleCount+=record.frames;log.records[log.count++]=r;return true;
}
