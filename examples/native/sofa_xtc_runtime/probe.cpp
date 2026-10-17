#include "adapter.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <new>
#include <numeric>
#include <filesystem>
#include <cwctype>
#include <thread>

thread_local bool tracking=false;
thread_local size_t allocations=0,deallocations=0;
void* operator new(size_t n) {if(tracking)allocations++;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {if(tracking&&p)deallocations++;std::free(p);}
void* operator new[](size_t n) {return ::operator new(n);}
void operator delete[](void* p) noexcept {::operator delete(p);}
void operator delete(void* p,size_t) noexcept {::operator delete(p);}
void operator delete[](void* p,size_t) noexcept {::operator delete(p);}

template<class T> void readExact(std::ifstream& f,T* p,size_t n) {
    if(!f.read(reinterpret_cast<char*>(p),std::streamsize(n*sizeof(T))))throw std::runtime_error("Truncated file");
}
void require(bool v,const char* message){if(!v)throw std::runtime_error(message);}
void newOutput(const char* path) {require(!std::filesystem::exists(path),"Output already exists");}
struct Banks {int count,taps,xtcTaps;float gain;std::vector<float> ir,xtc;};
Banks loadBanks(const char* path) {
    std::ifstream f(path,std::ios::binary);require(bool(f),"Cannot open banks");
    uint32_t h[4]{};readExact(f,h,4);require(h[0]==0x324b4e42&&h[1]>=1&&h[1]<=2048&&h[2]>=1&&h[2]<=32768&&h[3]<=32768,"Invalid banks");
    require(size_t(h[1])*((h[2]+127)/128)*2*runtime::Bins*sizeof(xtc::Complex)<=512u*1024u*1024u,"Coefficient storage exceeds 512 MiB");
    Banks b{int(h[1]),int(h[2]),int(h[3]),1,{},{}};readExact(f,&b.gain,1);
    require(std::isfinite(b.gain)&&b.gain>=0&&b.gain<=1,"Invalid common gain");
    b.ir.resize(size_t(b.count)*b.taps*2);readExact(f,b.ir.data(),b.ir.size());
    b.xtc.resize(size_t(b.xtcTaps)*4);readExact(f,b.xtc.data(),b.xtc.size());
    for(float v:b.xtc)require(std::isfinite(v),"Nonfinite XTC coefficient");
    char extra;require(!f.read(&extra,1),"Trailing bank data");return b;
}
void selftest() {
    runtime::Renderer r;std::array<float,256> output{};std::array<float,128> x{};x[0]=1;
    require(!r.render(nullptr,0,output.data()),"Unconfigured render accepted");
    float ir[258]{};ir[0]=1;ir[257]=.5f;r.configure(ir,1,129,1,1);
    tracking=true;const auto a=r.start(42);const auto duplicate=r.start(42);const auto full=r.start(43);tracking=false;
    require(a.slot==0&&duplicate.slot<0&&full.slot<0,"Capacity or duplicate ID handling failed");
    runtime::Mix mix{a,x.data(),0,0,0,0};
    tracking=true;const bool first=r.render(&mix,1,output.data());const bool stopped=r.stop(a);const auto blocked=r.start(42);tracking=false;
    require(first&&stopped&&blocked.slot<0&&!r.valid(a),"Retiring slot accepted early reuse");
    tracking=true;const bool drain1=r.render(nullptr,0,output.data());const bool drain2=r.render(nullptr,0,output.data());
    const auto b=r.start(42);const bool stale=r.stop(a);tracking=false;
    require(drain1&&drain2&&b.slot==0&&b.generation!=a.generation&&!stale,"Generation or drained reuse failed");
    runtime::Mix bad{a,x.data(),0,0,0,0};require(!r.render(&bad,1,output.data()),"Stale audio token accepted");
    bad.binding=b;bad.first=1;require(!r.render(&bad,1,output.data()),"Invalid bank accepted");
    bad.first=0;bad.begin=-.1f;require(!r.render(&bad,1,output.data()),"Invalid fade accepted");
    bad.begin=0;x[0]=std::numeric_limits<float>::quiet_NaN();require(!r.render(&bad,1,output.data()),"Nonfinite audio accepted");
    x[0]=1;runtime::Mix twice[2]{{b,x.data(),0,0,0,0},{b,x.data(),0,0,0,0}};
    require(!r.render(twice,2,output.data()),"Excess mixes accepted");
    runtime::Renderer reference,checked;float simple[]={1,.5f};reference.configure(simple,1,1,2,1);checked.configure(simple,1,1,2,1);
    const auto refBinding=reference.start(7),checkBinding=checked.start(7);
    runtime::Mix refMix{refBinding,x.data(),0,0,0,0},checkMix{checkBinding,x.data(),0,0,0,0};
    runtime::Mix duplicates[2]{checkMix,checkMix};require(!checked.render(duplicates,2,output.data()),"Duplicate slot mixes accepted");
    std::array<float,256> other{};
    for(int block=0;block<4;block++) {
        tracking=true;const bool refOk=reference.render(&refMix,1,output.data()),checkOk=checked.render(&checkMix,1,other.data());tracking=false;
        require(refOk&&checkOk&&output==other,"Rejected call changed renderer timeline");
    }
    checked.configure(simple,1,1,2,1);const auto renewed=checked.start(7);
    require(renewed.generation!=checkBinding.generation&&!checked.valid(checkBinding),"Reconfiguration revived stale token");
    require(checked.start(-1).slot<0,"Negative object ID accepted");
    require(checked.matches(renewed,7)&&!checked.matches(renewed,8),"Object ID lookup mismatch");
    runtime::Renderer ordered,reordered;ordered.configure(simple,1,1,2,1);reordered.configure(simple,1,1,2,1);
    const auto o1=ordered.start(1),o2=ordered.start(2),r1=reordered.start(1),r2=reordered.start(2);
    runtime::Mix forward[2]{{o1,x.data(),0,0,0,0},{o2,x.data(),0,0,0,0}};
    runtime::Mix reverse[2]{{r2,x.data(),0,0,0,0},{r1,x.data(),0,0,0,0}};
    for(int block=0;block<4;block++) {
        tracking=true;const bool aOk=ordered.render(forward,2,output.data()),bOk=reordered.render(reverse,2,other.data());tracking=false;
        require(aOk&&bOk&&output==other,"Mix order changed stable object routing");
    }
    runtime::Renderer tail;std::array<float,512> lastTap{};lastTap[510]=1;tail.configure(lastTap.data(),1,256,1,1);
    std::array<float,128> lastSample{};lastSample[127]=1;const auto tailBinding=tail.start(99);
    runtime::Mix tailMix{tailBinding,lastSample.data(),0,0,0,0};
    tracking=true;bool tailOk=tail.render(&tailMix,1,output.data())&&tail.stop(tailBinding);
    tailOk&=tail.render(nullptr,0,output.data())&&tail.render(nullptr,0,output.data());tracking=false;
    require(tailOk&&tail.busy()==0&&!tail.drained(),"Slot drain incorrectly discarded pending pipeline tail");
    tracking=true;tailOk=tail.render(nullptr,0,output.data());tracking=false;
    if(!tailOk||!tail.drained()||std::abs(output[252]-1)>=4e-5)
        throw std::runtime_error("Final pending tail: value="+std::to_string(output[252])+", drained="+std::to_string(tail.drained()));
    runtime::BlockAdapter adapter;adapter.renderer.configure(simple,1,1,2,1);const auto adapterBinding=adapter.renderer.start(55);
    std::array<float,128> ones{};ones.fill(1);
    runtime::SourceBlock source{adapterBinding,ones.data(),17,0,0,.25f,.75f,0,0};
    auto invalidSource=source;invalidSource.endGain=std::numeric_limits<float>::quiet_NaN();
    require(!adapter.render(&invalidSource,1,output.data()),"Nonfinite source gain accepted");
    invalidSource=source;invalidSource.frames=129;require(!adapter.render(&invalidSource,1,output.data()),"Oversized source block accepted");
    tracking=true;bool adapterOk=adapter.render(&source,1,output.data())&&adapter.renderer.stop(adapterBinding);
    adapterOk&=adapter.render(nullptr,0,output.data());tracking=false;
    require(adapterOk,"Gain/partial block adapter rejected valid input");
    for(int j=0;j<128;j++) {
        const float expected=j<17?.25f+.5f*float(j)/16:0;
        require(std::abs(output[j*2]-expected)<4e-5&&std::abs(output[j*2+1]-.5f*expected)<4e-5,"Source gain ramp or partial-block zero fill differs");
    }
    const float directions[]={0,0,-1,-1,0,0,1,0,0};const float front[]={0,0,-2},right[]={3,0,0},zero[]={0,0,0};
    tracking=true;const int frontBank=runtime::nearestDirection(directions,3,front),rightBank=runtime::nearestDirection(directions,3,right),badBank=runtime::nearestDirection(directions,3,zero);tracking=false;
    require(frontBank==0&&rightBank==2&&badBank==-1,"Nearest-direction lookup or Windows coordinates failed");
    require(allocations==0&&deallocations==0,"Callback or lifecycle allocated");
    std::cout<<"{\"selftest_passed\":true,\"callback_allocations\":"<<allocations<<",\"callback_deallocations\":"<<deallocations<<"}\n";
}
struct Control {int64_t id;int32_t action,first,second;float begin,end;};
void fixture(const Banks& banks,const char* inputPath,const char* outputPath,const char* jsonPath) {
    newOutput(outputPath);newOutput(jsonPath);
    auto outputKey=std::filesystem::weakly_canonical(outputPath).wstring(),jsonKey=std::filesystem::weakly_canonical(jsonPath).wstring();
    for(auto& c:outputKey)c=wchar_t(std::towlower(c));for(auto& c:jsonKey)c=wchar_t(std::towlower(c));
    require(outputKey!=jsonKey,"Output paths alias");
    std::ifstream input(inputPath,std::ios::binary);require(bool(input),"Cannot open fixture");uint32_t h[4]{};readExact(input,h,4);
    require(h[0]==0x31585452&&h[1]>=1&&h[1]<=64&&h[2]>=1&&h[2]<=10000&&h[3]>=h[1]&&h[3]<=64,"Invalid fixture");
    const int channels=int(h[1]),blocks=int(h[2]),capacity=int(h[3]);
    std::vector<Control> controls(size_t(blocks)*channels);
    std::vector<float> pcm(size_t(blocks)*channels*128),out(size_t(blocks)*256);
    // Packed control records are read field by field; C++ struct padding is irrelevant.
    for(auto& c:controls){readExact(input,&c.id,1);readExact(input,&c.action,1);readExact(input,&c.first,1);readExact(input,&c.second,1);readExact(input,&c.begin,1);readExact(input,&c.end,1);}
    readExact(input,pcm.data(),pcm.size());char extra;require(!input.read(&extra,1),"Trailing fixture data");
    runtime::Renderer renderer;renderer.configure(banks.ir.data(),banks.count,banks.taps,capacity,banks.xtcTaps?1:banks.gain);
    xtc::MatrixConvolver matrix;if(banks.xtcTaps)matrix.configure(banks.xtc.data(),banks.xtcTaps);
    std::array<runtime::Binding,64> bindings{};std::array<runtime::Mix,64> mixes{};
    for(int block=0;block<blocks;block++) {
        bool good=true;int count=0;tracking=true;
        for(int channel=0;channel<channels;channel++) {
            const auto& c=controls[size_t(block)*channels+channel];
            if(c.action==1){bindings[channel]=renderer.start(c.id);good&=bindings[channel].slot>=0;}
            else if(c.action==2){good&=renderer.matches(bindings[channel],c.id);good&=renderer.stop(bindings[channel]);bindings[channel]={};}
            else if(c.action!=0&&c.action!=3)good=false;
            if(c.action==1||c.action==3){good&=renderer.matches(bindings[channel],c.id);mixes[count++]={bindings[channel],&pcm[(size_t(block)*channels+channel)*128],c.first,c.second,c.begin,c.end};}
        }
        auto* destination=&out[size_t(block)*256];good&=renderer.render(mixes.data(),count,destination);
        if(banks.xtcTaps)for(int j=0;j<128;j++) {
            float l,r;matrix.process(destination[j*2],destination[j*2+1],l,r);destination[j*2]=l*banks.gain;destination[j*2+1]=r*banks.gain;
        }
        tracking=false;
        require(good,"Fixture lifecycle/render rejected");
    }
    require(allocations==0&&deallocations==0,"Fixture callback allocated");
    std::ofstream output(outputPath,std::ios::binary);output.write(reinterpret_cast<const char*>(out.data()),std::streamsize(out.size()*sizeof(float)));require(bool(output),"Output write failed");
    std::ofstream result(jsonPath);result<<"{\"passed\":true,\"callback_allocations\":"<<allocations<<",\"callback_deallocations\":"<<deallocations
        <<",\"coefficient_bytes\":"<<renderer.coefficientBytes()<<",\"history_bytes\":"<<renderer.historyBytes()
        <<",\"shared_xtc_spectra_history_bytes\":"<<(banks.xtcTaps?size_t((banks.xtcTaps+127)/128)*6*256*sizeof(xtc::Complex):0)
        <<",\"final_busy_slots\":"<<renderer.busy()<<",\"object_renderer_drained\":"<<(renderer.drained()?"true":"false")<<"}\n";
    require(bool(result),"Metrics write failed");
}
void benchmark(const Banks& banks,int objects,int blocks,bool paced,const char* path) {
    newOutput(path);
    require(objects>=1&&objects<=64&&blocks>=100&&blocks<=100000,"Invalid benchmark dimensions");
    runtime::Renderer renderer;renderer.configure(banks.ir.data(),banks.count,banks.taps,objects,banks.xtcTaps?1:banks.gain);
    xtc::MatrixConvolver matrix;if(banks.xtcTaps)matrix.configure(banks.xtc.data(),banks.xtcTaps);
    std::array<runtime::Binding,64> bindings{};std::array<runtime::Mix,64> mixes{};
    std::array<std::array<float,128>,64> input{};std::array<float,256> output{};
    for(int i=0;i<objects;i++)for(int j=0;j<128;j++)input[i][j]=.001f*std::sin(float(.01*(i+1)*(j+1)));
    std::vector<double> times;times.reserve(size_t(blocks));int overruns=0,wakeLate=0;double peak=0;
    const double budget=128000000./48000.;auto trialStart=std::chrono::steady_clock::now();
    for(int block=0;block<blocks;block++) {
        const auto due=trialStart+std::chrono::nanoseconds(int64_t(double(block)*128000000000./48000.));
        if(paced)std::this_thread::sleep_until(due);
        auto begin=std::chrono::steady_clock::now();if(paced&&std::chrono::duration<double,std::micro>(begin-due).count()>budget)wakeLate++;
        bool good=true;tracking=true;
        for(int i=0;i<objects;i++) {
            if(block==0){bindings[i]=renderer.start(1000+i);good&=bindings[i].slot>=0;}
            const int phase=(block/4+i*11)%banks.count;const int next=(phase+1)%banks.count;
            mixes[i]={bindings[i],input[i].data(),phase,next,0,1};
        }
        good&=renderer.render(mixes.data(),objects,output.data());
        if(banks.xtcTaps)for(int j=0;j<128;j++) {
            float l,r;matrix.process(output[j*2],output[j*2+1],l,r);output[j*2]=l*banks.gain;output[j*2+1]=r*banks.gain;
        }
        tracking=false;
        const double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count();
        require(good,"Benchmark render failed");times.push_back(us);overruns+=us>budget;
        for(float value:output){require(std::isfinite(value),"Nonfinite benchmark output");peak=std::max(peak,double(std::abs(value)));}
    }
    const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-trialStart).count();
    require(allocations==0&&deallocations==0,"Benchmark callback allocated");
    const double sum=std::accumulate(times.begin(),times.end(),0.);std::sort(times.begin(),times.end());
    std::ofstream out(path);out<<std::setprecision(12)<<"{\"objects\":"<<objects<<",\"taps\":"<<banks.taps<<",\"shared_xtc_taps\":"<<banks.xtcTaps<<",\"blocks\":"<<blocks
        <<",\"paced\":"<<(paced?"true":"false")<<",\"mean_us\":"<<sum/blocks<<",\"p99_us\":"<<times[size_t(.99*(blocks-1))]
        <<",\"max_us\":"<<times.back()<<",\"processing_budget_us\":"<<budget<<",\"processing_budget_exceedances\":"<<overruns
        <<",\"wake_lateness_over_one_quantum\":"<<wakeLate<<",\"elapsed_seconds\":"<<elapsed<<",\"peak\":"<<peak
        <<",\"coefficient_bytes\":"<<renderer.coefficientBytes()<<",\"history_bytes\":"<<renderer.historyBytes()
        <<",\"shared_xtc_spectra_history_bytes\":"<<(banks.xtcTaps?size_t((banks.xtcTaps+127)/128)*6*256*sizeof(xtc::Complex):0)
        <<",\"callback_allocations\":"<<allocations<<",\"callback_deallocations\":"<<deallocations<<"}\n";require(bool(out),"Benchmark write failed");
}
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--selftest"){selftest();return 0;}
    require(argc>=3,"Use --selftest, --fixture banks input output metrics, or --bench banks objects blocks paced metrics");
    auto banks=loadBanks(argv[2]);
    if(std::string(argv[1])=="--fixture"&&argc==6)fixture(banks,argv[3],argv[4],argv[5]);
    else if(std::string(argv[1])=="--bench"&&argc==7)benchmark(banks,std::stoi(argv[3]),std::stoi(argv[4]),std::stoi(argv[5])!=0,argv[6]);
    else throw std::runtime_error("Invalid arguments");return 0;
}catch(const std::exception& e){tracking=false;std::cerr<<e.what()<<'\n';return 1;}}
