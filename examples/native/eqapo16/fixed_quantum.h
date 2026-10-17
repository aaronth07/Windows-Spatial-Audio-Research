#pragma once
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

// Coalesce producer packets into one fixed APO quantum. Storage is allocated at
// construction; push invokes a caller-supplied processor without allocating.
class FixedQuantum {
    size_t channels,quantum,used=0;
    std::vector<float> pending;
    static size_t extent(size_t count,size_t frames){if(!count||!frames||count>std::numeric_limits<size_t>::max()/frames)throw std::invalid_argument("Invalid fixed quantum extent");return count*frames;}
public:
    FixedQuantum(size_t count,size_t frames):channels(count),quantum(frames),pending(extent(count,frames)){}
    size_t remaining()const{return used;}
    template<class Render>void push(const float* samples,size_t frames,Render&& render){
        if(frames&&!samples)throw std::invalid_argument("Missing source samples");
        while(frames){
            size_t take=std::min(frames,quantum-used);
            std::copy_n(samples,take*channels,pending.data()+used*channels);
            samples+=take*channels;used+=take;frames-=take;
            if(used==quantum){render(pending.data(),quantum);used=0;}
        }
    }
};
