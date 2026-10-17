#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>
#include <stdexcept>
#include <string>
inline void RequirePinnedBinary(const wchar_t* path,const char* expected){
 std::ifstream input(path,std::ios::binary|std::ios::ate);if(!input)throw std::runtime_error("Cannot read reviewed binary");auto length=input.tellg();if(length<=0||length>64*1024*1024)throw std::runtime_error("Binary exceeds reviewed size bound");input.seekg(0);std::vector<BYTE> bytes(static_cast<size_t>(length));if(!input.read(reinterpret_cast<char*>(bytes.data()),length))throw std::runtime_error("Binary read failed");std::array<BYTE,32> digest{};
 if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),static_cast<ULONG>(bytes.size()),digest.data(),static_cast<ULONG>(digest.size()))<0)throw std::runtime_error("Binary SHA256 failed");
 std::ostringstream text;for(auto value:digest)text<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<unsigned>(value);if(text.str()!=expected)throw std::runtime_error("Unreviewed binary SHA256");
}
inline void WriteNew(const std::wstring& path,const void* data,DWORD bytes){
 HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("Output exists or cannot be created");DWORD written{};BOOL ok=WriteFile(file,data,bytes,&written,nullptr);BOOL closed=CloseHandle(file);if(!ok||written!=bytes||!closed)throw std::runtime_error("Output write failed");
}
