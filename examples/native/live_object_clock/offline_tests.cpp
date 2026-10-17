#define wmain tap_host_main
#include "tap_host.cpp"
#undef wmain
#include <thread>

static void** importSlot(HMODULE module) {
    auto* base=reinterpret_cast<BYTE*>(module);
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);
    const auto directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if(!directory.VirtualAddress)return nullptr;
    auto* descriptor=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+directory.VirtualAddress);
    for(;descriptor->Name;descriptor++)if(descriptor->OriginalFirstThunk){
        auto* names=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+descriptor->OriginalFirstThunk);
        auto* values=reinterpret_cast<IMAGE_THUNK_DATA64*>(base+descriptor->FirstThunk);
        for(;names->u1.AddressOfData;names++,values++)if(!IMAGE_SNAP_BY_ORDINAL64(names->u1.Ordinal)){
            const auto* name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData)->Name;
            if(!std::strcmp(name,"CoCreateInstance"))return reinterpret_cast<void**>(&values->u1.Function);
        }
    }
    return nullptr;
}
int wmain(int argc,wchar_t** argv){
    try{
        require(argc==2,"producer DLL required");unsigned cases=0;
        SourceRecord records[2];float storage[4]{},pcm[4]{1,2,3,4};SourceRecord record;record.frames=4;
        SourceLog log{records,2,0,storage,4,0};
        require(appendRecord(log,record,pcm)&&log.count==1&&log.sampleCount==4&&storage[3]==4,"exact capacity");cases++;
        require(!appendRecord(log,record,pcm)&&log.count==1&&log.sampleCount==4,"full sample capacity");cases++;
        log.sampleCount=UINT32_MAX;require(!appendRecord(log,record,pcm),"invalid extent overflow");cases++;
        log.sampleCount=0;log.count=2;require(!appendRecord(log,record,pcm),"record capacity");cases++;
        log.count=0;require(!appendRecord(log,record,nullptr),"null PCM");cases++;
        log.samples=nullptr;require(!appendRecord(log,record,pcm),"null storage");cases++;
        // Independent per-stream logs are written concurrently without a shared append index.
        std::array<std::array<SourceRecord,100>,2> independentRecords;
        std::array<std::array<float,400>,2> independentSamples;
        SourceLog logs[2];bool appendSuccess[2]{true,true};
        for(uint32_t i=0;i<2;i++)logs[i]={independentRecords[i].data(),100,0,independentSamples[i].data(),400,0};
        auto appendStream=[&](uint32_t index){for(uint32_t i=0;i<100;i++){SourceRecord r;r.frames=4;r.streamId=index+1;r.update=i+1;appendSuccess[index]&=appendRecord(logs[index],r,pcm);}};
        std::thread a(appendStream,0),b(appendStream,1);a.join();b.join();
        require(appendSuccess[0]&&appendSuccess[1]&&logs[0].count==100&&logs[1].count==100&&logs[0].sampleCount==400&&logs[1].sampleCount==400,"parallel independent log extents");cases++;
        require(logs[0].records[99].streamId==1&&logs[1].records[99].streamId==2&&logs[0].records[99].sampleOffset==396&&logs[1].records[99].sampleOffset==396,"parallel stream separation");cases++;
        std::array<uint64_t,200> ids;
        auto allocateIds=[&](uint32_t index){for(uint32_t i=0;i<100;i++)ids[index*100+i]=nextObjectId.fetch_add(1,std::memory_order_relaxed);};
        std::thread c(allocateIds,0),d(allocateIds,1);c.join();d.join();std::sort(ids.begin(),ids.end());
        require(ids.front()>0&&std::adjacent_find(ids.begin(),ids.end())==ids.end(),"global creation IDs unique across threads");cases++;
        HMODULE dll=LoadLibraryExW(fs::canonical(argv[1]).c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        require(dll!=nullptr,"load producer");auto run=reinterpret_cast<RunProducer>(GetProcAddress(dll,"RunSpatialProducer"));require(run!=nullptr,"export");
        ProducerResult result;SourceLog reference;
        require(run(nullptr,&reference,&result)==E_POINTER,"null endpoint");cases++;
        require(run(L"",&reference,&result)==E_POINTER,"empty endpoint");cases++;
        require(run(L"unused",nullptr,&result)==E_POINTER,"null reference");cases++;
        require(run(L"unused",&reference,nullptr)==E_POINTER,"null result");cases++;
        // Keep a host CoCreate import without creating any audio or COM object.
        CoCreateInstance(GUID_NULL,nullptr,CLSCTX_INPROC_SERVER,__uuidof(IUnknown),nullptr);
        void** slot=importSlot(dll);require(slot!=nullptr,"producer CoCreate import");void* original=*slot;
        void** hostSlot=importSlot(GetModuleHandleW(nullptr));require(hostSlot!=nullptr,"host CoCreate import");void* hostOriginal=*hostSlot;
        MEMORY_BASIC_INFORMATION before{},during{},after{};require(VirtualQuery(slot,&before,sizeof(before))!=0,"query protection");
        {ProducerImport patch(dll);require(*slot==reinterpret_cast<void*>(&interceptedCreate)&&*hostSlot==hostOriginal,"only producer patched");cases++;
         require(VirtualQuery(slot,&during,sizeof(during))&&during.Protect==before.Protect,"protection restored while patched");cases++;
         patch.restore();require(patch.restored()&&*slot==original,"explicit import restoration");cases++;}
        {ProducerImport patch(dll);require(*slot!=original,"second patch");}
        require(*slot==original&&*hostSlot==hostOriginal,"destructor import restoration");cases++;
        require(VirtualQuery(slot,&after,sizeof(after))&&after.Protect==before.Protect,"final protection restoration");cases++;
        FreeLibrary(dll);
        bool rejected=false;try{ProducerImport invalid(GetModuleHandleW(L"kernel32.dll"));}catch(const std::exception&){rejected=true;}
        require(rejected,"missing CoCreate import rejected");cases++;
        std::cout<<"{\"passed\":true,\"offline_cases\":"<<cases<<",\"windows_audio_streams_started\":0}\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
