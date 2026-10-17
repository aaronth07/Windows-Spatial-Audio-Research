#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <audioenginebaseapo.h>
#include <audiomediatype.h>
#include <mmdeviceapi.h>
inline constexpr GUID BootstrapClass={0x27e623f8,0x71ce,0x40d1,{0xae,0x8c,0xbc,0xac,0x46,0xa7,0x12,0x43}};
// PKEY_AudioEndpoint_GUID from SDK mmdeviceapi.h, represented locally to avoid registration libraries.
inline constexpr PROPERTYKEY EndpointGuidKey={{0x1da5d803,0xd492,0x4edd,{0x8c,0x23,0xe0,0xc0,0xff,0xee,0x7f,0x0e}},4};
inline constexpr PROPERTYKEY BootstrapLogPathKey={BootstrapClass,1};
inline constexpr PROPERTYKEY BootstrapFactoryPathKey={BootstrapClass,2};
inline constexpr PROPERTYKEY BootstrapPreloadKey={BootstrapClass,3};
inline constexpr PROPERTYKEY BootstrapFactorySha256Key={BootstrapClass,4};
struct BootstrapEvent {
 UINT32 size,sequence,phase,pid,tid,cpuPresent,hashPresent;
 HRESULT status;
 LONGLONG qpc;
 GUID endpoint;
 WCHAR cpuPath[1024];
 BYTE cpuSha256[32];
};
using ConfigureBootstrapFn=HRESULT(WINAPI*)(const wchar_t*,BOOL,const wchar_t*);
using CloseBootstrapFn=HRESULT(WINAPI*)();
using GetBootstrapCounterFn=ULONG(WINAPI*)();
