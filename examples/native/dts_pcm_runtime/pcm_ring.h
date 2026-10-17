#pragma once
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

// One producer and one consumer. Construction/reset require both to be stopped.
// Frame timestamps are absolute sample indices within one stream generation.
template<unsigned Channels> class BasicPcmRing final {
 public:
  static constexpr unsigned InputChannels=Channels,OutputChannels=16,SampleRate=48000;
  static_assert(Channels>0&&Channels<=OutputChannels,"Input channel count must fit transport");
  using Matrix=std::array<std::array<float,InputChannels>,OutputChannels>;
  enum class Result { Ok, Full, TimestampDiscontinuity, InvalidInput, EndOfInput };
  static Matrix SpeakerOrder(){
   Matrix m{};constexpr unsigned order[10]={1,2,0,5,3,4,6,7,8,9};
   if constexpr(Channels==10){for(unsigned c=0;c<10;c++)m[c][order[c]]=1;}
   else{for(unsigned c=0;c<Channels;c++)m[c][c]=1;}return m;
  }
  explicit BasicPcmRing(unsigned capacity, const Matrix& matrix=SpeakerOrder()):capacity_(capacity),storage_(CheckedCapacity(capacity)*OutputChannels){
   for(unsigned o=0;o<OutputChannels;o++)for(unsigned i=0;i<InputChannels;i++){
    if(!std::isfinite(matrix[o][i]))throw std::invalid_argument("Nonfinite channel matrix");
    if(matrix[o][i]!=0)routes_[o].push_back({i,matrix[o][i]});
   }
  }
  Result Push(const float* const* planar,unsigned frames,uint64_t firstFrame){
   if(ended_.load(std::memory_order_acquire))return Result::EndOfInput;
   if(!planar||frames==0||frames>capacity_)return Result::InvalidInput;
   if(firstFrame!=nextFrame_)return Result::TimestampDiscontinuity;
   if(firstFrame>std::numeric_limits<uint64_t>::max()-frames)return Result::InvalidInput;
   for(unsigned i=0;i<InputChannels;i++)if(!planar[i])return Result::InvalidInput;
   const auto write=write_.load(std::memory_order_relaxed),read=read_.load(std::memory_order_acquire);
   if(write-read>capacity_||frames>capacity_-(write-read))return Result::Full;
   // Validate before publication, so failure cannot publish partial PCM.
   for(unsigned c=0;c<InputChannels;c++)for(unsigned f=0;f<frames;f++)if(!std::isfinite(planar[c][f]))return Result::InvalidInput;
   for(unsigned f=0;f<frames;f++){
    float* dst=storage_.data()+static_cast<size_t>((write+f)%capacity_)*OutputChannels;
    for(unsigned c=0;c<OutputChannels;c++){
     float value=0;for(const auto& route:routes_[c])value+=planar[route.input][f]*route.gain;
     if(!std::isfinite(value))return Result::InvalidInput;dst[c]=value;
    }
   }
   nextFrame_+=frames;write_.store(write+frames,std::memory_order_release);return Result::Ok;
  }
  // Always fills requested output extent; missing source frames become zero.
  // Return value identifies valid source frames, including an EOF partial tail.
  unsigned Pop(float* interleaved,unsigned requested,uint64_t* firstFrame=nullptr){
   if(!interleaved||requested==0||requested>capacity_)throw std::invalid_argument("Invalid output extent");
   const auto read=read_.load(std::memory_order_relaxed),write=write_.load(std::memory_order_acquire);
   const unsigned available=static_cast<unsigned>(std::min<uint64_t>(requested,write-read));
   const unsigned offset=static_cast<unsigned>(read%capacity_),first=std::min(available,capacity_-offset);
   if(first)std::memcpy(interleaved,storage_.data()+static_cast<size_t>(offset)*OutputChannels,static_cast<size_t>(first)*OutputChannels*sizeof(float));
   if(available>first)std::memcpy(interleaved+static_cast<size_t>(first)*OutputChannels,storage_.data(),static_cast<size_t>(available-first)*OutputChannels*sizeof(float));
   std::fill(interleaved+static_cast<size_t>(available)*OutputChannels,interleaved+static_cast<size_t>(requested)*OutputChannels,0.f);
   if(firstFrame)*firstFrame=baseFrame_+read;read_.store(read+available,std::memory_order_release);return available;
  }
  uint64_t Queued()const {const auto read=read_.load(std::memory_order_acquire);return write_.load(std::memory_order_acquire)-read;}
  unsigned Capacity()const{return capacity_;}
  void EndInput(){ended_.store(true,std::memory_order_release);}
  bool Drained()const{return ended_.load(std::memory_order_acquire)&&Queued()==0;}
  // Explicit discontinuity/source-change barrier; caller reports discarded frames.
  uint64_t Reset(uint64_t firstFrame=0){const auto discarded=Queued();read_.store(0);write_.store(0);nextFrame_=baseFrame_=firstFrame;ended_.store(false);return discarded;}
 private:
  static size_t CheckedCapacity(unsigned n){if(n<1024||n>1048576)throw std::invalid_argument("Ring capacity outside 1024..1048576 frames");return n;}
  struct Route {unsigned input;float gain;};
  unsigned capacity_;std::vector<float> storage_;std::array<std::vector<Route>,OutputChannels> routes_;
  static_assert(std::atomic<uint64_t>::is_always_lock_free,"64-bit lock-free indices required");
  std::atomic<uint64_t> write_{0},read_{0};
  uint64_t nextFrame_=0,baseFrame_=0;std::atomic<bool> ended_{false};
};
using PcmRing=BasicPcmRing<10>;
using Pcm12Ring=BasicPcmRing<12>;
