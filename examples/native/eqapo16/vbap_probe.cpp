#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

struct Vector{double x,y,z;};
static Vector cross(Vector a,Vector b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static double dot(Vector a,Vector b){return a.x*b.x+a.y*b.y+a.z*b.z;}
template<class T>static void read(std::ifstream& f,T* p,size_t count){if(!f.read(reinterpret_cast<char*>(p),static_cast<std::streamsize>(sizeof(T)*count)))throw std::runtime_error("Truncated carrier");}
struct Face{std::array<uint32_t,3> indices;std::array<Vector,3> rows;};

int wmain(int argc,wchar_t** argv){try{
    if(argc!=4)throw std::runtime_error("Usage: vbap_probe carrier.bin output.f32 metrics.json");
    if(std::filesystem::exists(argv[2])||std::filesystem::exists(argv[3])||_wcsicmp(std::filesystem::weakly_canonical(argv[2]).c_str(),std::filesystem::weakly_canonical(argv[3]).c_str())==0)throw std::runtime_error("Use separate new output files");
    std::ifstream file(std::filesystem::path(argv[1]),std::ios::binary);if(!file)throw std::runtime_error("Carrier missing");
    std::array<uint32_t,6> h{};read(file,h.data(),h.size());
    if((h[0]!=0x31504256&&h[0]!=0x32504256)||h[1]<4||h[1]>32||h[2]>128||h[3]<1||h[3]>4096||h[4]<1||h[4]>2048||h[5]<4||h[5]>128)throw std::runtime_error("Invalid header");
    const size_t channels=h[1],objects=h[2],blocks=h[3],frames=h[4];
    const bool lifetimeIds=h[0]==0x32504256;
    const uint64_t expected=24ull+channels*12ull+h[5]*12ull+blocks*static_cast<uint64_t>(objects)*((4ull+frames)*4ull+(lifetimeIds?8ull:0ull));
    if(expected>512ull*1024*1024||std::filesystem::file_size(argv[1])!=expected)throw std::runtime_error("Invalid carrier extent");
    std::vector<Vector> speakers(channels);std::vector<float> coordinates(channels*3);read(file,coordinates.data(),coordinates.size());
    for(size_t i=0;i<channels;i++){speakers[i]={coordinates[3*i],coordinates[3*i+1],coordinates[3*i+2]};if(!std::isfinite(dot(speakers[i],speakers[i]))||std::abs(dot(speakers[i],speakers[i])-1)>1e-5)throw std::runtime_error("Speaker vectors must be finite unit directions");}
    std::vector<Face> faces(h[5]);
    for(auto& f:faces){
        read(file,f.indices.data(),3);for(auto i:f.indices)if(i>=channels)throw std::runtime_error("Triangle index");
        auto a=speakers[f.indices[0]],b=speakers[f.indices[1]],c=speakers[f.indices[2]];
        double determinant=dot(a,cross(b,c));if(std::abs(determinant)<1e-8)throw std::runtime_error("Degenerate speaker triplet");
        f.rows={cross(b,c),cross(c,a),cross(a,b)};
        for(auto& r:f.rows){r.x/=determinant;r.y/=determinant;r.z/=determinant;}
    }
    std::vector<uint64_t> identities(lifetimeIds?blocks*objects:0),previousIdentity(objects);
    if(lifetimeIds){read(file,identities.data(),identities.size());for(size_t b=0;b<blocks;b++)for(size_t i=0;i<objects;i++){if(!identities[b*objects+i])throw std::runtime_error("Missing object lifetime ID");for(size_t j=0;j<i;j++)if(identities[b*objects+i]==identities[b*objects+j])throw std::runtime_error("Duplicate object lifetime ID");}}
    std::vector<float> meta(blocks*objects*4),input(blocks*objects*frames);
    read(file,meta.data(),meta.size());read(file,input.data(),input.size());char trailing{};if(file.read(&trailing,1))throw std::runtime_error("Trailing carrier bytes");
    for(const auto* data:{&meta,&input})for(float value:*data)if(!std::isfinite(value))throw std::runtime_error("Nonfinite carrier");
    std::vector<float> output(blocks*frames*channels),target(objects*channels),previous(objects*channels),previousVolume(objects);
    std::vector<double> costs;costs.reserve(blocks);double maxPowerError=0,maxDirectionError=0;
    for(size_t block=0;block<blocks;block++){
        auto start=std::chrono::steady_clock::now();
        for(size_t object=0;object<objects;object++){
            const float* item=meta.data()+(block*objects+object)*4;
            Vector direction{item[0],item[1],item[2]};double length=std::sqrt(dot(direction,direction));
            if(length<1e-9||item[3]<0||item[3]>16)throw std::runtime_error("Invalid object position or gain");
            direction.x/=length;direction.y/=length;direction.z/=length;
            const Face* chosen=nullptr;std::array<double,3> coefficients{};double best=-1e30;
            for(const auto& face:faces){std::array<double,3> gains={dot(face.rows[0],direction),dot(face.rows[1],direction),dot(face.rows[2],direction)};double minimum=*std::min_element(gains.begin(),gains.end());if(minimum>=-1e-6&&minimum>best){best=minimum;chosen=&face;coefficients=gains;}}
            if(!chosen)throw std::runtime_error("Direction outside supplied speaker coverage");
            double power=0;for(auto& value:coefficients){value=std::max(0.,value);power+=value*value;}if(power<1e-20)throw std::runtime_error("Empty panning weights");
            std::fill_n(target.data()+object*channels,channels,0.f);Vector reconstructed{};
            for(size_t j=0;j<3;j++){auto channel=chosen->indices[j];float gain=static_cast<float>(coefficients[j]/std::sqrt(power));target[object*channels+channel]=gain;reconstructed.x+=speakers[channel].x*gain;reconstructed.y+=speakers[channel].y*gain;reconstructed.z+=speakers[channel].z*gain;}
            double norm=std::sqrt(dot(reconstructed,reconstructed));
            maxDirectionError=std::max(maxDirectionError,std::sqrt(std::pow(reconstructed.x/norm-direction.x,2)+std::pow(reconstructed.y/norm-direction.y,2)+std::pow(reconstructed.z/norm-direction.z,2)));
            if(block==0||(lifetimeIds&&identities[block*objects+object]!=previousIdentity[object])){std::copy_n(target.data()+object*channels,channels,previous.data()+object*channels);previousVolume[object]=item[3];}
            for(size_t frame=0;frame<frames;frame++){
                float alpha=static_cast<float>(frame+1)/static_cast<float>(frames);double gainPower=0;
                for(size_t channel=0;channel<channels;channel++){double gain=previous[object*channels+channel]*(1-alpha)+target[object*channels+channel]*alpha;gainPower+=gain*gain;}
                if(gainPower<1e-20)throw std::runtime_error("Empty transition weights");
                double measuredPower=0;float sample=input[(block*objects+object)*frames+frame]*(previousVolume[object]*(1-alpha)+item[3]*alpha);double inverseNorm=1/std::sqrt(gainPower);
                for(size_t channel=0;channel<channels;channel++){float gain=static_cast<float>((previous[object*channels+channel]*(1-alpha)+target[object*channels+channel]*alpha)*inverseNorm);output[(block*frames+frame)*channels+channel]+=sample*gain;measuredPower+=double(gain)*gain;}
                maxPowerError=std::max(maxPowerError,std::abs(measuredPower-1));
            }
            previousVolume[object]=item[3];
            if(lifetimeIds)previousIdentity[object]=identities[block*objects+object];
        }
        std::copy(target.begin(),target.end(),previous.begin());
        costs.push_back(std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count());
    }
    for(float value:output)if(!std::isfinite(value))throw std::runtime_error("Nonfinite output");
    std::ofstream raw(std::filesystem::path(argv[2]),std::ios::binary);raw.write(reinterpret_cast<const char*>(output.data()),static_cast<std::streamsize>(output.size()*4));if(!raw)throw std::runtime_error("Write output failed");
    std::sort(costs.begin(),costs.end());double sum=0;for(auto cost:costs)sum+=cost;
    std::ofstream record{std::filesystem::path(argv[3])};record<<std::setprecision(12)<<"{\"channels\":"<<channels<<",\"objects\":"<<objects<<",\"blocks\":"<<blocks<<",\"frames_per_block\":"<<frames<<",\"mean_us\":"<<sum/costs.size()<<",\"p99_us\":"<<costs[static_cast<size_t>(.99*(costs.size()-1))]<<",\"maximum_us\":"<<costs.back()<<",\"maximum_unit_power_error\":"<<maxPowerError<<",\"maximum_target_direction_reconstruction_error\":"<<maxDirectionError<<",\"device_stream\":false}\n";if(!record)throw std::runtime_error("Write metrics failed");return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
