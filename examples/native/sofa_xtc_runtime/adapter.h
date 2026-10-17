#pragma once
#include "runtime.h"

namespace runtime {
// Directions are finite unit XYZ triplets prepared once, in the same coordinate
// system as position. This lookup allocates nothing and is bounded by bankCount.
inline int nearestDirection(const float* directions,int bankCount,const float* position) noexcept {
    if(!directions||!position||bankCount<1||bankCount>2048||
       !std::isfinite(position[0])||!std::isfinite(position[1])||!std::isfinite(position[2]))return -1;
    const float scale=std::max({std::abs(position[0]),std::abs(position[1]),std::abs(position[2])});
    if(scale<1e-20f)return -1;
    const float x=position[0]/scale,y=position[1]/scale,z=position[2]/scale;
    float maximum=-std::numeric_limits<float>::infinity();int result=-1;
    for(int i=0;i<bankCount;i++) {
        const float dot=x*directions[i*3]+y*directions[i*3+1]+z*directions[i*3+2];
        if(dot>maximum){maximum=dot;result=i;}
    }
    return result;
}
// One already-identified object's span within one 128-frame renderer block.
// Gains refer to its first/last valid sample and are applied exactly once.
struct SourceBlock {
    Binding binding;
    const float* samples=nullptr;
    int frames=0,first=0,second=0;
    float startGain=1,endGain=1,begin=0,end=0;
};
class BlockAdapter {
    std::array<std::array<float,xtc::Block>,MaxObjects> gained{};
    std::array<Mix,MaxObjects> mixes{};
public:
    Renderer renderer;
    bool render(const SourceBlock* sources,int count,float* output) noexcept {
        if(count<0||count>MaxObjects||(count&&!sources)||!output)return false;
        for(int i=0;i<count;i++) {
            const auto& source=sources[i];
            if(source.frames<0||source.frames>xtc::Block||(source.frames&&!source.samples)||
               !std::isfinite(source.startGain)||!std::isfinite(source.endGain))return false;
            for(int j=0;j<xtc::Block;j++) {
                float value=0;
                if(j<source.frames) {
                    const float mix=source.frames>1?float(j)/float(source.frames-1):0;
                    const float gain=source.startGain+(source.endGain-source.startGain)*mix;
                    value=source.samples[j]*gain;if(!std::isfinite(value))return false;
                }
                gained[i][j]=value;
            }
            mixes[i]={source.binding,gained[i].data(),source.first,source.second,source.begin,source.end};
        }
        return renderer.render(mixes.data(),count,output);
    }
};
}
