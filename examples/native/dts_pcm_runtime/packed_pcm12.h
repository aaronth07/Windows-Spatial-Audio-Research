#pragma once
#include "pcm_ring.h"

// Build-specific observed PCM ordering: FL,FR,C,LFE, surround pair A L/R,
// surround pair B L/R,TFL,TFR,TBL,TBR. No standard mask is inferred here.
class PackedPcm12Ring final {
 public:
  using Result=Pcm12Ring::Result;
  explicit PackedPcm12Ring(unsigned capacity):ring_(capacity){}
  static float Decode24(const unsigned char* p){
   const unsigned raw=unsigned(p[0])|(unsigned(p[1])<<8)|(unsigned(p[2])<<16);
   const int value=raw>=0x800000?static_cast<int>(raw)-0x1000000:static_cast<int>(raw);
   return static_cast<float>(value)*(1.f/8388608.f);
  }
  Result Push(const unsigned char* bytes,size_t length,unsigned frames,uint64_t firstFrame){
   if(!bytes||frames==0||frames>1024||length!=static_cast<size_t>(frames)*36)return Result::InvalidInput;
   std::array<const float*,12> pointers{};
   for(unsigned c=0;c<12;c++){pointers[c]=planar_[c].data();for(unsigned f=0;f<frames;f++)planar_[c][f]=Decode24(bytes+(static_cast<size_t>(f)*12+c)*3);}
   return ring_.Push(pointers.data(),frames,firstFrame);
  }
  unsigned Pop(float* output,unsigned frames,uint64_t* firstFrame=nullptr){return ring_.Pop(output,frames,firstFrame);}
  uint64_t Queued()const{return ring_.Queued();}
  void EndInput(){ring_.EndInput();}
  bool Drained()const{return ring_.Drained();}
  uint64_t Reset(uint64_t firstFrame=0){return ring_.Reset(firstFrame);}
 private:
  Pcm12Ring ring_;std::array<std::array<float,1024>,12> planar_{};
};
