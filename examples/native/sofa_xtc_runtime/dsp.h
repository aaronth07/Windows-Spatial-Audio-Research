#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdint>
#include <vector>

namespace xtc {
constexpr int Block = 128, Fft = 256, MaxSources = 32;
constexpr double Pi = 3.14159265358979323846;
using Complex = std::complex<float>;

// Fixed-size radix-2 transform; no plan creation or allocation in processing.
inline void fft(Complex* a, bool inverse) {
    for (int i=1,j=0;i<Fft;i++) {
        int bit=Fft>>1; for (;j&bit;bit>>=1) j^=bit; j^=bit;
        if(i<j) std::swap(a[i],a[j]);
    }
    for(int n=2;n<=Fft;n<<=1) {
        Complex step=std::polar(1.f,float((inverse?2:-2)*Pi/n));
        for(int k=0;k<Fft;k+=n) { Complex w=1; for(int j=0;j<n/2;j++) {
            Complex u=a[k+j],v=a[k+j+n/2]*w; a[k+j]=u+v;a[k+j+n/2]=u-v;w*=step;
        }}
    }
    if(inverse) for(int i=0;i<Fft;i++) a[i]/=float(Fft);
}

class MatrixConvolver {
    int partitions=1, cursor=0, fill=0, read=0;
    std::vector<Complex> spectra,history;
    std::array<std::array<float,Block>,2> input{},output{},overlap{};
    std::array<Complex,Fft> scratch{};
public:
    void configure(const float* coefficients,int taps) {
        partitions=(taps+Block-1)/Block;
        spectra.assign(size_t(4)*partitions*Fft,{});
        history.assign(size_t(2)*partitions*Fft,{});
        cursor=fill=read=0;input={};output={};overlap={};
        for(int route=0;route<4;route++)for(int p=0;p<partitions;p++) {
            scratch.fill({});for(int j=0;j<Block;j++)if(p*Block+j<taps)scratch[j]=coefficients[(p*Block+j)*4+route];
            fft(scratch.data(),false);std::copy(scratch.begin(),scratch.end(),spectra.begin()+(route*partitions+p)*Fft);
        }
    }
    void process(float left,float right,float& outL,float& outR) {
        outL=output[0][read];outR=output[1][read];read=(read+1)%Block;
        input[0][fill]=left;input[1][fill]=right;
        if(++fill<Block)return;fill=0;
        for(int ear=0;ear<2;ear++) {
            scratch.fill({});for(int j=0;j<Block;j++)scratch[j]=input[ear][j];fft(scratch.data(),false);
            std::copy(scratch.begin(),scratch.end(),history.begin()+(ear*partitions+cursor)*Fft);
        }
        for(int speaker=0;speaker<2;speaker++) {
            scratch.fill({});
            for(int ear=0;ear<2;ear++)for(int p=0;p<partitions;p++) {
                int h=(cursor-p+partitions)%partitions;
                const auto* x=&history[(ear*partitions+h)*Fft];const auto* g=&spectra[((speaker*2+ear)*partitions+p)*Fft];
                for(int k=0;k<Fft;k++)scratch[k]+=x[k]*g[k];
            }
            fft(scratch.data(),true);
            for(int j=0;j<Block;j++){output[speaker][j]=scratch[j].real()+overlap[speaker][j];overlap[speaker][j]=scratch[j+Block].real();}
        }
        cursor=(cursor+1)%partitions;
    }
};

// Producer and consumer each own a buffer; the exchange slot is never edited.
template<class T> class TripleBuffer {
    T buffers[3]{};
    std::atomic<unsigned> middle{1};
    unsigned back=2,front=0;
public:
    void publish(const T& value){buffers[back]=value;back=middle.exchange(back|4,std::memory_order_acq_rel)&3;}
    const T& read(){if(middle.load(std::memory_order_acquire)&4)front=middle.exchange(front,std::memory_order_acq_rel)&3;return buffers[front];}
};

struct Source { float x=0,y=0,z=-1,gain=1,send=.3f;int channel=0,enabled=1,solo=0; };
struct Parameters {
    float master=.25f;int mute=0,bypass=0,inputMode=0,sourceCount=2;
    float wet=0,early=0,decay=1.2f,damping=.5f,roomSize=5,predelayMs=15,width=1,ambience=0;
    float headX=0,headY=0,headZ=0,qx=0,qy=0,qz=0,qw=1;
    Source sources[MaxSources]{};
};

class Renderer {
    int rate=48000,position=0;
    std::vector<float> delay;
    std::array<std::vector<float>,8> reverb;
    std::array<int,8> reverbPos{};
    std::array<float,8> dampState{};
    std::array<float,8> feedback{};
    std::vector<float> predelay;
    int predelayPos=0,parameterCounter=0;
    float gain=.0f,bypassMix=0;
    std::vector<float> bypassDelay;
    int bypassPos=0;
    float profileGain=1;
public:
    MatrixConvolver matrix;
    void configure(int sampleRate,const float* filters,int taps,int designDelay,float attenuation) {
        rate=sampleRate;matrix.configure(filters,taps);profileGain=attenuation;
        delay.assign(size_t(MaxSources)*rate*2,0);position=0;gain=0;bypassMix=0;
        bypassDelay.assign(size_t(std::max(1,designDelay+Block))*2,0);bypassPos=0;
        constexpr float seconds[8]={.0297f,.0371f,.0411f,.0437f,.0531f,.0593f,.0617f,.0719f};
        for(int i=0;i<8;i++)reverb[i].assign(int(rate*seconds[i]),0);
        reverbPos={};dampState={};
        predelay.assign(rate/2+1,0);predelayPos=0;parameterCounter=0;
    }
    float delayed(int channel,float samples) const {
        samples=std::clamp(samples,0.f,float(rate*2-2));int integer=int(samples);float fraction=samples-integer;
        int a=(position-integer+rate*2)%(rate*2),b=(a-1+rate*2)%(rate*2);
        return delay[size_t(channel)*rate*2+a]*(1-fraction)+delay[size_t(channel)*rate*2+b]*fraction;
    }
    void process(const float* channels,int count,const Parameters& p,float& outL,float& outR) {
        for(int ch=0;ch<std::min(count,MaxSources);ch++)delay[size_t(ch)*rate*2+position]=channels[ch];
        float l=0,r=0,send=0;
        if(p.inputMode==0 || p.inputMode==1) { l=count>0?channels[0]:0;r=count>1?channels[1]:l;send=(l+r)*.5f; }
        else {
            bool solo=false;for(int i=0;i<p.sourceCount;i++)solo|=p.sources[i].solo!=0;
            for(int i=0;i<p.sourceCount;i++) {
                const Source& s=p.sources[i];if(!s.enabled||(solo&&!s.solo)||s.channel<0||s.channel>=count)continue;
                float x=s.x-p.headX,y=s.y-p.headY,z=s.z-p.headZ;
                // Inverse Hamilton rotation: room vector to head-local coordinates.
                float tx=2*(-p.qy*z+p.qz*y),ty=2*(-p.qz*x+p.qx*z),tz=2*(-p.qx*y+p.qy*x);
                float rx=x+p.qw*tx+(-p.qy*tz+p.qz*ty),rz=z+p.qw*tz+(-p.qx*ty+p.qy*tx);
                float distance=std::sqrt(x*x+y*y+z*z),azimuth=std::atan2(rx,-rz),pan=std::sin(azimuth);
                float common=std::min(distance,20.f)*rate/343.f;
                float itd=.00065f*rate*pan;
                float volume=s.gain/std::max(1.f,distance);
                l+=delayed(s.channel,common+std::max(0.f,itd))*std::sqrt(.5f*(1-pan))*volume;
                r+=delayed(s.channel,common+std::max(0.f,-itd))*std::sqrt(.5f*(1+pan))*volume;
                send+=delayed(s.channel,common)*volume*s.send;
                if(p.early>0) {
                    float echo=std::clamp(p.roomSize,1.f,30.f)*2*rate/343.f;
                    l+=delayed(s.channel,common+echo)*volume*p.early*.27f;
                    r+=delayed(s.channel,common+echo*1.13f)*volume*p.early*.27f;
                }
            }
            float norm=1/std::sqrt(float(std::max(1,p.sourceCount)));l*=norm;r*=norm;send*=norm;
        }
        if(p.ambience>0 && count>=2) {
            float side=(delayed(0,rate*.019f)-delayed(1,rate*.027f))*.5f*p.ambience;
            l+=side*p.width;r-=side*p.width;send+=side;
        }
        predelay[predelayPos]=send;
        int pre=std::clamp(int(p.predelayMs*rate/1000),0,int(predelay.size())-1);
        send=predelay[(predelayPos-pre+int(predelay.size()))%predelay.size()];
        predelayPos=(predelayPos+1)%int(predelay.size());
        if(parameterCounter==0)for(int i=0;i<8;i++)feedback[i]=std::pow(.001f,float(reverb[i].size())/(rate*std::max(.1f,p.decay)));
        parameterCounter=(parameterCounter+1)%Block;
        float revL=0,revR=0;
        for(int i=0;i<8;i++) {
            float v=reverb[i][reverbPos[i]];
            dampState[i]=v*(1-std::clamp(p.damping,0.f,.98f))+dampState[i]*std::clamp(p.damping,0.f,.98f);
            reverb[i][reverbPos[i]]=send*.15f+dampState[i]*feedback[i];
            if(i%2)revR+=v;else revL+=v;
            reverbPos[i]=(reverbPos[i]+1)%int(reverb[i].size());
        }
        l+=revL*p.wet;r+=revR*p.wet;position=(position+1)%(rate*2);
        float xl,xr;matrix.process(l,r,xl,xr);
        float bl=bypassDelay[bypassPos*2],br=bypassDelay[bypassPos*2+1];
        bypassDelay[bypassPos*2]=l;bypassDelay[bypassPos*2+1]=r;bypassPos=(bypassPos+1)%int(bypassDelay.size()/2);
        // Both banks maintain input history. Ten-ms equal-amplitude transition.
        float step=1.f/(rate*.01f);bypassMix+=std::clamp(float(p.bypass)-bypassMix,-step,step);
        gain+=( (p.mute?0:p.master)-gain)*step;
        outL=((1-bypassMix)*xl+bypassMix*bl)*profileGain*gain;
        outR=((1-bypassMix)*xr+bypassMix*br)*profileGain*gain;
    }
};
}
