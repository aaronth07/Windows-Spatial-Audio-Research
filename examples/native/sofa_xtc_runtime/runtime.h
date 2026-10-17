#pragma once
#include "dsp.h"
#include <limits>
#include <stdexcept>
#include <emmintrin.h>

namespace runtime {
constexpr int MaxObjects=64;
constexpr int Bins=xtc::Fft/2+1;
struct Binding { int slot=-1; uint64_t generation=0; };
struct Mix { Binding binding; const float* input=nullptr; int first=0,second=0; float begin=0,end=0; };
struct History { int64_t block=-1; int first=-1,second=-1; };
struct Slot { bool active=false,retiring=false; int64_t id=-1,lastInput=-1; uint64_t generation=0; };
// Windows x64 guarantees SSE2. Two complex MACs per vector; no optional AVX
// instruction set, FFT convention change, coefficient change or allocation.
inline void complexMac(xtc::Complex* destination,const xtc::Complex* input,const xtc::Complex* filter) noexcept {
    auto* d=reinterpret_cast<float*>(destination);
    const auto* x=reinterpret_cast<const float*>(input);const auto* h=reinterpret_cast<const float*>(filter);
    const __m128 sign=_mm_castsi128_ps(_mm_setr_epi32(std::numeric_limits<int>::min(),0,std::numeric_limits<int>::min(),0));
    for(int k=0;k<2*(Bins-1);k+=4) {
        const __m128 a=_mm_loadu_ps(x+k),b=_mm_loadu_ps(h+k);
        const __m128 real=_mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,0,0));
        const __m128 imag=_mm_shuffle_ps(a,a,_MM_SHUFFLE(3,3,1,1));
        const __m128 swapped=_mm_shuffle_ps(b,b,_MM_SHUFFLE(2,3,0,1));
        const __m128 product=_mm_add_ps(_mm_mul_ps(real,b),_mm_xor_ps(_mm_mul_ps(imag,swapped),sign));
        _mm_storeu_ps(d+k,_mm_add_ps(_mm_loadu_ps(d+k),product));
    }
    destination[Bins-1]+=xtc::Complex(input[Bins-1].real()*filter[Bins-1].real(),0);
}

// Input-time filters: every input block retains its prepared filter IDs and
// sample crossfade. Old inputs continue through their original full FIRs.
class Renderer {
    int taps=0,parts=0,banks=0,capacity=0;
    int64_t block=0;
    float gain=1;
    uint64_t nextGeneration=1;
    std::vector<xtc::Complex> filters,history;
    std::vector<History> metadata;
    std::array<Slot,MaxObjects> slots{};
    std::array<std::array<xtc::Complex,xtc::Fft>,2> scratch{},sum{};
    std::array<std::array<float,xtc::Block>,2> overlap{},pending{};
public:
    // Coefficients: [bank,tap,output], two outputs, no normalization here.
    void configure(const float* ir,int bankCount,int length,int maxObjects,float commonGain) {
        if(!ir||bankCount<1||bankCount>2048||length<1||length>32768||maxObjects<1||maxObjects>MaxObjects||
           !std::isfinite(commonGain)||commonGain<0||commonGain>1) throw std::invalid_argument("Invalid renderer configuration");
        taps=length;parts=(taps+xtc::Block-1)/xtc::Block;banks=bankCount;capacity=maxObjects;gain=commonGain;
        if(size_t(banks)*parts*2*Bins*sizeof(xtc::Complex)>512u*1024u*1024u)throw std::invalid_argument("Coefficient storage exceeds 512 MiB");
        filters.assign(size_t(banks)*parts*2*Bins,{});
        history.assign(size_t(capacity)*parts*2*Bins,{});
        metadata.assign(size_t(capacity)*parts,{});
        for(int bank=0;bank<banks;bank++)for(int part=0;part<parts;part++)for(int output=0;output<2;output++) {
            scratch[0].fill({});
            for(int t=0;t<xtc::Block && part*xtc::Block+t<taps;t++) {
                const float value=ir[(size_t(bank)*taps+part*xtc::Block+t)*2+output];
                if(!std::isfinite(value)) throw std::invalid_argument("Nonfinite filter");
                scratch[0][t]=value;
            }
            xtc::fft(scratch[0].data(),false);
            std::copy(scratch[0].begin(),scratch[0].begin()+Bins,filters.begin()+((size_t(bank)*parts+part)*2+output)*Bins);
        }
        slots={};overlap={};pending={};block=0;
    }
    Binding start(int64_t id) noexcept {
        if(id<0)return {};
        for(int i=0;i<capacity;i++)if(slots[i].active && slots[i].id==id)return {};
        for(int i=0;i<capacity;i++)if(!slots[i].active&&!slots[i].retiring) {
            slots[i]={true,false,id,-1,nextGeneration++};
            for(int p=0;p<parts;p++)metadata[size_t(i)*parts+p]={};
            return {i,slots[i].generation};
        }
        return {};
    }
    bool valid(Binding b) const noexcept {
        return b.slot>=0&&b.slot<capacity&&slots[b.slot].active&&slots[b.slot].generation==b.generation;
    }
    bool matches(Binding b,int64_t id) const noexcept {return valid(b)&&slots[b.slot].id==id;}
    bool stop(Binding b) noexcept {
        if(!valid(b))return false;
        slots[b.slot].active=false;slots[b.slot].retiring=slots[b.slot].lastInput>=0;
        return true;
    }
    int busy() const noexcept {int n=0;for(int i=0;i<capacity;i++)n+=slots[i].active||slots[i].retiring;return n;}
    bool drained() const noexcept {
        if(busy())return false;
        for(int ear=0;ear<2;ear++)for(int j=0;j<xtc::Block;j++)if(pending[ear][j]!=0||overlap[ear][j]!=0)return false;
        return true;
    }
    size_t coefficientBytes() const noexcept {return filters.size()*sizeof(xtc::Complex);}
    size_t historyBytes() const noexcept {return history.size()*sizeof(xtc::Complex)+metadata.size()*sizeof(History);}
    // All error checks precede state changes. Invalid calls leave the timeline intact.
    bool render(const Mix* mixes,int count,float* output) noexcept {
        if(parts<1||!output||count<0||count>capacity||(count&&!mixes))return false;
        std::array<const Mix*,MaxObjects> bySlot{};
        for(int i=0;i<count;i++) {
            const auto& m=mixes[i];
            if(!valid(m.binding)||bySlot[m.binding.slot]||!m.input||m.first<0||m.first>=banks||m.second<0||m.second>=banks||
               !std::isfinite(m.begin)||!std::isfinite(m.end)||m.begin<0||m.begin>1||m.end<0||m.end>1)return false;
            for(int j=0;j<xtc::Block;j++)if(!std::isfinite(m.input[j]))return false;
            bySlot[m.binding.slot]=&m;
        }
        for(int j=0;j<xtc::Block;j++)for(int ear=0;ear<2;ear++)output[j*2+ear]=pending[ear][j]*gain;
        sum[0].fill({});sum[1].fill({});
        const int cursor=int(block%parts);
        for(int object=0;object<capacity;object++) {
            auto& slot=slots[object];if(!slot.active&&!slot.retiring)continue;
            auto& current=metadata[size_t(object)*parts+cursor];current={block,-1,-1};
            if(const Mix* m=bySlot[object]) {
                current.first=m->first;current.second=m->second;
                scratch[0].fill({});scratch[1].fill({});
                const bool same=m->first==m->second;
                for(int j=0;j<xtc::Block;j++) {
                    const float alpha=m->begin+(m->end-m->begin)*(float(j)/float(xtc::Block-1));
                    scratch[0][j]=m->input[j]*(same?1.f:1-alpha);
                    if(!same)scratch[1][j]=m->input[j]*alpha;
                }
                if(same)current.second=-1;
                for(int component=0;component<(same?1:2);component++) {
                    xtc::fft(scratch[component].data(),false);
                    std::copy(scratch[component].begin(),scratch[component].begin()+Bins,history.begin()+((size_t(object)*parts+cursor)*2+component)*Bins);
                }
                slot.lastInput=block;
            }
            for(int p=0;p<parts;p++) {
                const int index=(cursor-p+parts)%parts;
                const auto& old=metadata[size_t(object)*parts+index];if(old.block!=block-p)continue;
                const int ids[]={old.first,old.second};
                for(int component=0;component<2;component++)if(ids[component]>=0) {
                    const auto* x=&history[((size_t(object)*parts+index)*2+component)*Bins];
                    for(int ear=0;ear<2;ear++) {
                        const auto* h=&filters[((size_t(ids[component])*parts+p)*2+ear)*Bins];
                        complexMac(sum[ear].data(),x,h);
                    }
                }
            }
            if(slot.retiring&&block-slot.lastInput>=parts)slot.retiring=false;
        }
        for(int ear=0;ear<2;ear++) {
            sum[ear][0]=xtc::Complex(sum[ear][0].real(),0);
            sum[ear][Bins-1]=xtc::Complex(sum[ear][Bins-1].real(),0);
            for(int k=Bins;k<xtc::Fft;k++)sum[ear][k]=std::conj(sum[ear][xtc::Fft-k]);
            xtc::fft(sum[ear].data(),true);
            for(int j=0;j<xtc::Block;j++) {
                pending[ear][j]=sum[ear][j].real()+overlap[ear][j];
                overlap[ear][j]=sum[ear][j+xtc::Block].real();
            }
        }
        block++;return true;
    }
};
}
