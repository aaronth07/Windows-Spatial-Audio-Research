#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <activation.h>
#include <winstring.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mftransform.h>
#include <mferror.h>
#include <wrl.h>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include "pinned_binary.h"
#include "packed_pcm12.h"
using Microsoft::WRL::ComPtr;
using Clock=std::chrono::steady_clock;
static double Us(Clock::duration d){return std::chrono::duration<double,std::micro>(d).count();}
static void Require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void Check(HRESULT hr,const char* message){Require(SUCCEEDED(hr),message);}
static std::string Stats(std::vector<double> v){
 Require(!v.empty(),"Empty timing distribution");std::sort(v.begin(),v.end());double sum=0;for(auto d:v)sum+=d;
 auto p=[&](double q){return v[static_cast<size_t>(std::ceil(q*static_cast<double>(v.size())))-1];};
 std::ostringstream s;s<<"{\"count\":"<<v.size()<<",\"mean_us\":"<<sum/static_cast<double>(v.size())<<",\"p99_us\":"<<p(.99)<<",\"p999_us\":"<<p(.999)<<",\"max_us\":"<<v.back()<<"}";return s.str();
}
class GuardedBuffer final:public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,IMFMediaBuffer>{
 std::vector<BYTE> bytes_;DWORD capacity_{},length_{};
 public:
 void Initialize(DWORD n){capacity_=n;bytes_.assign(static_cast<size_t>(n)+64,0xcd);}
 bool Intact()const{for(size_t i=capacity_;i<bytes_.size();i++)if(bytes_[i]!=0xcd)return false;return true;}
 STDMETHODIMP Lock(BYTE** data,DWORD* maximum,DWORD* length)override{if(!data)return E_POINTER;*data=bytes_.data();if(maximum)*maximum=capacity_;if(length)*length=length_;return S_OK;}
 STDMETHODIMP Unlock()override{return S_OK;}
 STDMETHODIMP GetCurrentLength(DWORD* n)override{if(!n)return E_POINTER;*n=length_;return S_OK;}
 STDMETHODIMP SetCurrentLength(DWORD n)override{if(n>capacity_)return E_INVALIDARG;length_=n;return S_OK;}
 STDMETHODIMP GetMaxLength(DWORD* n)override{if(!n)return E_POINTER;*n=capacity_;return S_OK;}
};
int wmain(int argc,wchar_t** argv){
 if(argc!=7){std::cerr<<"Usage: pcm12_stream decoder runtime-directory media consumer-frames(128|480) new-result-prefix reference-float-directory\n";return 2;}
 const std::wstring requested=argv[4];if(requested!=L"128"&&requested!=L"480")return 2;
 try{
  const auto begin=Clock::now();const unsigned period=static_cast<unsigned>(std::stoul(requested));
  const std::filesystem::path prefix(argv[5]);auto csvPath=prefix;csvPath+=".csv";auto jsonPath=prefix;jsonPath+=".json";
  Require(!std::filesystem::exists(csvPath)&&!std::filesystem::exists(jsonPath),"Result exists");
  RequirePinnedBinary(argv[1],"0fb48a12aef354016992e5fc2a34ea7ef4852711a5f48a66bdff07661285c4ca");
  std::array<std::vector<float>,10> reference;
  for(unsigned c=0;c<10;c++){std::ifstream file(std::filesystem::path(argv[6])/(L"buffer_"+std::to_wstring(c)+L".f32"),std::ios::binary|std::ios::ate);Require(bool(file)&&file.tellg()==2883584*4,"Reference extent");reference[c].resize(2883584);file.seekg(0);file.read(reinterpret_cast<char*>(reference[c].data()),2883584*4);Require(bool(file),"Reference read");}
  Check(CoInitializeEx(nullptr,COINIT_MULTITHREADED),"COM");Check(MFStartup(MF_VERSION,MFSTARTUP_LITE),"MFStartup");
  ComPtr<IMFAttributes> settings;Check(MFCreateAttributes(&settings,1),"Reader settings");Check(settings->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS,TRUE),"Disable converters");
  ComPtr<IMFSourceReader> reader;Check(MFCreateSourceReaderFromURL(argv[3],settings.Get(),&reader),"Source reader");
  constexpr DWORD audio=static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
  Check(reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS),FALSE),"Disable streams");Check(reader->SetStreamSelection(audio,TRUE),"Select audio");
  ComPtr<IMFMediaType> input;Check(reader->GetNativeMediaType(audio,0,&input),"Native input");Check(reader->SetCurrentMediaType(audio,nullptr,input.Get()),"Select native input");
  auto cookie=AddDllDirectory(argv[2]);Require(cookie!=nullptr,"Runtime directory");
  const auto dll=LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32|LOAD_LIBRARY_SEARCH_USER_DIRS);Require(dll!=nullptr,"Decoder load");
  // No metadata endpoint redirect or spatial object APIs are used by this executable.
  using GetFactory=HRESULT(WINAPI*)(HSTRING,IActivationFactory**);auto get=reinterpret_cast<GetFactory>(GetProcAddress(dll,"DllGetActivationFactory"));Require(get!=nullptr,"Factory export");
  HSTRING name{};const std::wstring className=L"DTSXDecoder.DTSXDecoder";Check(WindowsCreateString(className.c_str(),static_cast<UINT32>(className.size()),&name),"Class name");ComPtr<IActivationFactory> factory;auto factoryHr=get(name,&factory);WindowsDeleteString(name);Check(factoryHr,"Factory");
  ComPtr<IInspectable> instance;Check(factory->ActivateInstance(&instance),"Instance");ComPtr<IMFTransform> decoder;Check(instance.As(&decoder),"MFT");Check(decoder->SetInputType(0,input.Get(),0),"Input type");
  ComPtr<IMFMediaType> output;for(DWORD i=0;i<16;i++){ComPtr<IMFMediaType> type;auto hr=decoder->GetOutputAvailableType(0,i,&type);if(hr==MF_E_NO_MORE_TYPES)break;Check(hr,"Output enumeration");GUID sub{};Check(type->GetGUID(MF_MT_SUBTYPE,&sub),"Output subtype");if(sub==MFAudioFormat_PCM){output=type;break;}}
  Require(output!=nullptr,"PCM output missing");Check(output->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,12),"Channels");Check(output->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,36),"Block alignment");Check(output->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND,1728000),"Byte rate");Check(output->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK,0x2d63f),"Requested mask");Check(decoder->SetOutputType(0,output.Get(),0),"Set PCM12");
  ComPtr<IMFMediaType> current;Check(decoder->GetOutputCurrentType(0,&current),"Current output");UINT32 channels{},rate{},bits{};Check(current->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS,&channels),"Current channels");Check(current->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,&rate),"Current rate");Check(current->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,&bits),"Current bits");Require(channels==12&&rate==48000&&bits==24,"Unexpected negotiated PCM");
  MFT_OUTPUT_STREAM_INFO info{};Check(decoder->GetOutputStreamInfo(0,&info),"Output info");Require(info.cbSize>=49152&&info.cbSize<=1048576,"Unsafe output allocation extent");
  Check(decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING,0),"Begin");Check(decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM,0),"Start");
  PackedPcm12Ring ring(4096);std::array<float,4096*16> expected{};std::vector<float> out(static_cast<size_t>(period)*16);std::vector<double> decodeTimes,pushTimes,popTimes;decodeTimes.reserve(3000);pushTimes.reserve(3000);popTimes.reserve(24000);
  std::array<double,12> energies{},refEnergy{},cross{},maxDirect{};constexpr unsigned sourceOrder[12]={1,2,0,5,3,4,3,4,6,7,8,9};uint64_t pushed=0,popped=0,padding=0,maxQueue=0,minQueue=4096;unsigned inputs=0,outputs=0,drainOutputs=0,readCalls=0;bool sourceEof=false,decoderEof=false,drainSent=false;double firstPcmUs=0;
  std::ostringstream csv;csv<<"packet,first_source_frame,decode_work_us,convert_push_us,queue_before,queue_after,input_packets,after_source_eof\n";
  const double setupUs=Us(Clock::now()-begin);
  auto nextPacket=[&](){
   const auto start=Clock::now();
   for(unsigned attempts=0;attempts<10000;attempts++){
    ComPtr<IMFSample> sample;Check(MFCreateSample(&sample),"Sample allocation");auto guarded=Microsoft::WRL::Make<GuardedBuffer>();guarded->Initialize(info.cbSize);ComPtr<IMFMediaBuffer> buffer;Check(guarded.As(&buffer),"Buffer interface");Check(sample->AddBuffer(buffer.Get()),"Add buffer");
    MFT_OUTPUT_DATA_BUFFER data{};data.pSample=sample.Get();DWORD status{};const auto hr=decoder->ProcessOutput(0,1,&data,&status);if(data.pEvents)data.pEvents->Release();
    if(hr==MF_E_TRANSFORM_NEED_MORE_INPUT){
     if(sourceEof){decoderEof=true;ring.EndInput();return false;}
     DWORD stream{},flags{};LONGLONG timestamp{};ComPtr<IMFSample> next;Check(reader->ReadSample(audio,0,&stream,&flags,&timestamp,&next),"Read source packet");readCalls++;
     Require(!(flags&(MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED|MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED|MF_SOURCE_READERF_ERROR)),"Source format/error change");
     if(next){LONGLONG packetTime{};Check(next->GetSampleTime(&packetTime),"Input timestamp");const auto wanted=static_cast<LONGLONG>(inputs)*1024*10000000/48000;Require(std::llabs(packetTime-wanted)<=1,"Input timestamp gap");Check(decoder->ProcessInput(0,next.Get(),0),"Process input");inputs++;}
     if(flags&MF_SOURCE_READERF_ENDOFSTREAM){sourceEof=true;Check(decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM,0),"End of source");Check(decoder->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN,0),"Drain");drainSent=true;}
     continue;
    }
    Check(hr,"Process output (format changes require new stream)");Require(data.pSample==sample.Get()&&guarded->Intact(),"Owned buffer or guard violation");DWORD count{};Check(sample->GetBufferCount(&count),"Output count");Require(count==1,"Unexpected PCM planes");
    LONGLONG timestamp{},duration{};Check(sample->GetSampleTime(&timestamp),"Output timestamp");Check(sample->GetSampleDuration(&duration),"Output duration");Require(std::llabs(timestamp-static_cast<LONGLONG>(pushed)*10000000/48000)<=1,"Output timestamp gap");Require(duration==213333||duration==213334,"Output duration");
    BYTE* bytes{};DWORD maximum{},length{};Check(buffer->Lock(&bytes,&maximum,&length),"Lock output");Require(maximum==info.cbSize&&length==36864,"Output extent");
    const double decodeUs=Us(Clock::now()-start);const auto queueBefore=ring.Queued();auto pushStart=Clock::now();Require(ring.Push(bytes,length,1024,pushed)==PackedPcm12Ring::Result::Ok,"Ring push");const auto pushUs=Us(Clock::now()-pushStart);
    Require(pushed+1024<=2883584,"Extra source frames");
    for(unsigned f=0;f<1024;f++)for(unsigned c=0;c<16;c++){const float value=c<12?PackedPcm12Ring::Decode24(bytes+(f*12+c)*3):0.f;expected[((pushed+f)%4096)*16+c]=value;if(c<12){const double original=reference[sourceOrder[c]][static_cast<size_t>(pushed+f)];Require(std::isfinite(original),"Reference finite validation");energies[c]+=double(value)*value;refEnergy[c]+=original*original;cross[c]+=original*value;maxDirect[c]=std::max(maxDirect[c],std::abs(original-value));}}
    Check(buffer->Unlock(),"Unlock output");if(!outputs)firstPcmUs=Us(Clock::now()-begin);if(sourceEof)drainOutputs++;
    csv<<outputs<<','<<pushed<<','<<decodeUs<<','<<pushUs<<','<<queueBefore<<','<<ring.Queued()<<','<<inputs<<','<<(sourceEof?1:0)<<'\n';decodeTimes.push_back(decodeUs);pushTimes.push_back(pushUs);outputs++;pushed+=1024;maxQueue=std::max(maxQueue,ring.Queued());return true;
   }throw std::runtime_error("Decoder progress limit");
  };
  // Offline demand model: a file worker refills to >=2048 frames before the
  // simulated device read. This waits outside Pop and is not a device callback.
  while(!decoderEof||!ring.Drained()){
   while(!decoderEof&&ring.Queued()<2048)nextPacket();
   if(ring.Drained())break;
   uint64_t first{};auto start=Clock::now();const auto valid=ring.Pop(out.data(),period,&first);popTimes.push_back(Us(Clock::now()-start));Require(first==popped&&valid>0,"Read timestamp/starvation");
   for(unsigned f=0;f<period;f++)for(unsigned c=0;c<16;c++){const float expectedSample=f<valid?expected[((popped+f)%4096)*16+c]:0.f;Require(std::memcmp(&expectedSample,&out[static_cast<size_t>(f)*16+c],4)==0,"PCM transport mismatch");}
   popped+=valid;padding+=period-valid;minQueue=std::min(minQueue,ring.Queued());
  }
  Require(drainSent&&sourceEof&&decoderEof&&ring.Drained(),"EOF/drain invariant");Require(inputs==2816&&outputs==2816&&pushed==2883584&&popped==pushed,"Complete fixture conservation");
  Check(decoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING,0),"End streaming");RemoveDllDirectory(cookie);
  std::ostringstream json;json<<"{\"all_passed\":true,\"metadata_endpoint_or_redirect_used\":false,\"sample_rate\":48000,\"input_channels\":12,\"transport_channels\":16,\"consumer_frames\":"<<period<<",\"target_refill_frames\":2048,\"capacity_frames\":4096,\"input_packets\":"<<inputs<<",\"output_packets\":"<<outputs<<",\"read_calls\":"<<readCalls<<",\"drain_outputs\":"<<drainOutputs<<",\"source_eof\":true,\"drain_completed\":true,\"pushed_frames\":"<<pushed<<",\"popped_source_frames\":"<<popped<<",\"eof_padding_frames\":"<<padding<<",\"max_queued_frames\":"<<maxQueue<<",\"min_post_pop_queue_frames\":"<<minQueue<<",\"setup_us\":"<<setupUs<<",\"first_pcm_including_setup_us\":"<<firstPcmUs<<",\"decode_work\":"<<Stats(decodeTimes)<<",\"convert_push\":"<<Stats(pushTimes)<<",\"pop\":"<<Stats(popTimes)<<",\"channel_energy\":[";
  for(unsigned c=0;c<12;c++){if(c)json<<',';json<<energies[c];}json<<"],\"full_reference_comparison\":[";
  constexpr double expectedGain[12]={1,1,1,1,.484208,.484208,.732812,.732812,1,1,1,1};
  for(unsigned c=0;c<12;c++){const auto correlation=cross[c]/std::sqrt(refEnergy[c]*energies[c]);const auto gain=cross[c]/refEnergy[c];const auto fittedRms=std::sqrt(std::max(0.0,energies[c]-cross[c]*cross[c]/refEnergy[c])/2883584);
   std::cout<<"REFERENCE column="<<c<<" gain="<<gain<<" correlation="<<correlation<<" direct_max="<<maxDirect[c]<<" fitted_rms="<<fittedRms<<'\n';
   Require(std::isfinite(correlation)&&correlation>.9999&&std::abs(gain-expectedGain[c])<.001,"Full reference signal identity");Require(fittedRms<.00001,"Full reference fitted residual");if(c)json<<',';json<<"{\"column\":"<<c<<",\"native_reference_index\":"<<sourceOrder[c]<<",\"gain\":"<<gain<<",\"correlation\":"<<correlation<<",\"direct_max_abs_error\":"<<maxDirect[c]<<",\"fitted_rms_error\":"<<fittedRms<<"}";}json<<"]}";
  const auto csvString=csv.str(),jsonString=json.str();WriteNew(csvPath.wstring(),csvString.data(),static_cast<DWORD>(csvString.size()));WriteNew(jsonPath.wstring(),jsonString.data(),static_cast<DWORD>(jsonString.size()));std::cout<<jsonString<<'\n';return 0;
 }catch(const std::exception& error){std::cerr<<"ERROR "<<error.what()<<'\n';return 1;}
}
