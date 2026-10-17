#pragma once
#include <filesystem>
#include <fstream>
#include <cstring>
#include "../eqapo16/pinned_binary.h"

// Prepared coefficients are an owned module companion, never resolved from CWD.
// The SHA-256 detects damaged data; it does not authenticate a publisher.
struct PreparedSofaHeader {
 char magic[8]; UINT32 version,mode,banks,taps,capacity,reserved; BYTE sha256[32];
};
static_assert(sizeof(PreparedSofaHeader)==64);
extern "C" __declspec(dllexport) HRESULT WINAPI InitializeCpuFactory(){
 try{
  static std::mutex preparationMutex;static bool preparedOnce{};
  std::lock_guard<std::mutex> preparationLock(preparationMutex);
  if(preparedOnce)return S_OK;
  if(liveDsps)return E_UNEXPECTED;
  {std::lock_guard<std::mutex> pathLock(pathMutex);if(!cpuPath.empty())return E_UNEXPECTED;}
  HMODULE self{};
  if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&InitializeCpuFactory),&self))return HRESULT_FROM_WIN32(GetLastError());
  std::array<wchar_t,32768> path{};
  DWORD length=GetModuleFileNameW(self,path.data(),static_cast<DWORD>(path.size()));
  if(!length||length>=path.size())return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
  auto prepared=std::filesystem::path(path.data());prepared.replace_extension(L".sofa.bin");
  std::ifstream input(prepared,std::ios::binary|std::ios::ate);
  if(!input)return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
  auto size=input.tellg();
  if(size<sizeof(PreparedSofaHeader)||size>128ll*1024*1024)return E_INVALIDARG;
  input.seekg(0);PreparedSofaHeader header{};
  if(!input.read(reinterpret_cast<char*>(&header),sizeof(header)))return E_FAIL;
  if(std::memcmp(header.magic,"WSSSOFA1",8)||header.version!=1||header.reserved)return E_INVALIDARG;
  size_t floats{};
  if(header.mode==0){if(header.banks||header.taps||header.capacity)return E_INVALIDARG;}
  else if(header.mode==2){
   if(!header.banks||header.banks>2048||header.taps!=4096||!header.capacity||header.capacity>64)return E_INVALIDARG;
   floats=size_t(header.banks)*(size_t(header.taps)*2+3);
  }else return E_INVALIDARG;
  if(static_cast<uint64_t>(size)!=sizeof(header)+floats*sizeof(float))return E_INVALIDARG;
  std::vector<float> payload(floats);
  if(floats&&!input.read(reinterpret_cast<char*>(payload.data()),static_cast<std::streamsize>(floats*sizeof(float))))return E_FAIL;
  std::array<BYTE,32> digest{};
  // BCrypt accepts zero bytes; provide a valid address for the empty payload.
  BYTE empty{};
  auto bytes=floats?reinterpret_cast<BYTE*>(payload.data()):&empty;
  if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes,static_cast<ULONG>(floats*sizeof(float)),digest.data(),32)<0)return E_FAIL;
  if(std::memcmp(digest.data(),header.sha256,32))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
  length=GetSystemDirectoryW(path.data(),static_cast<UINT>(path.size()));
  if(!length||length>=path.size())return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
  auto native=std::filesystem::path(path.data())/L"HrtfDspCpu.dll";
  RequirePinnedBinary(native.c_str(),"4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9");
  if(header.mode==2){
   HRESULT hr=ConfigureSofa(payload.data(),header.banks,header.taps,payload.data()+size_t(header.banks)*header.taps*2,header.capacity);
   if(FAILED(hr))return hr;
  }else{HRESULT hr=ConfigureDiagnostic(FALSE);if(FAILED(hr))return hr;}
  HRESULT status=ConfigureCpuPath(native.c_str());
  if(SUCCEEDED(status))preparedOnce=true;
  return status;
 }catch(const std::bad_alloc&){return E_OUTOFMEMORY;}catch(...){return E_FAIL;}
}
