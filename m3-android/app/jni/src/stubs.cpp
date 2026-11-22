// MCLA - Midnight Club: Los Angeles Android Stub Functions
// This file provides stub implementations for functions that were called by the
// decompiled Xbox 360 code but don't exist on Android.
//
// Generated: 2024 - Auto-generated stubs for ~240 missing functions

#include <cstdint>
#include <cstring>
#include <cmath>
#include <ctime>
#include <cstdlib>

#include <rex/ppc.h>

// Helper macro to create both name and __imp__name versions
// When DECLARE_REX_FUNC("name") is used in the generated code:
// - Creates declarations for both "name" and "__imp__name"
#define DEFINE_STUB_2(name, imp_name) \
    extern "C" void imp_name(PPCContext& ctx, uint8_t* base) { \
    } \
    extern "C" void name(PPCContext& ctx, uint8_t* base) { \
        imp_name(ctx, base); \
    }

#define DEFINE_STUB_EMPTY(name) \
    extern "C" void name(PPCContext& ctx, uint8_t* base) { \
    }

// ============================================
// Cache/Memory System Functions
// ============================================


// ============================================
// Graphics/Direct3D Functions
// ============================================


// Graphics resource functions  
// D3DDevice_CreateTexture: has real generated body in larecomp_recomp.77.cpp:14157
// Unmask to allocate guest texture objects (populates [0x828393AC]/[0x828393B0])
// DEFINE_STUB_EMPTY(D3DDevice_CreateTexture)

// Unmasked: real generated bodies exist in m2-codegen/generated/
// DEFINE_STUB_EMPTY(grcTextureFactoryXenon_CreateTexture)
// DEFINE_STUB_EMPTY(grcTextureFactoryXenon_DestroyTexture)
// DEFINE_STUB_EMPTY(grcDevice_BeginTiledRendering)
// DEFINE_STUB_EMPTY(grcDevice_EndTiledRendering)
// DEFINE_STUB_EMPTY(grcDevice_EndFrame)
// DEFINE_STUB_EMPTY(grcTextureXenon_Copy)
// DEFINE_STUB_EMPTY(grcTextureXenon_LockRect)
// DEFINE_STUB_EMPTY(grcTextureXenon_UnlockRect)
// DEFINE_STUB_EMPTY(grcTextureXenon_Init)
// DEFINE_STUB_EMPTY(grcTextureXenon_ResourceCtor)
// DEFINE_STUB_EMPTY(grcTextureXenon_dtor)
// DEFINE_STUB_EMPTY(grcTextureXenon_ctor)

// ============================================
// Windows Kernel Functions
// These are Xbox 360 kernel functions that don't exist on Android
// For __imp__ prefixed functions, declare both variants
// ============================================

// Ke functions - DECLARE_REX_FUNC(__imp__KeXxx) creates both __imp__KeXxx and ____imp__KeXxx
DEFINE_STUB_2(KeDelayExecutionThread, __imp__KeDelayExecutionThread)
DEFINE_STUB_2(KeSetEvent, __imp__KeSetEvent)
DEFINE_STUB_2(KeWaitForSingleObject, __imp__KeWaitForSingleObject)
DEFINE_STUB_2(KeWaitForMultipleObjects, __imp__KeWaitForMultipleObjects)

// ============================================
// Device/FS functions
// ============================================

DEFINE_STUB_EMPTY(IoCheckShareAccess)
DEFINE_STUB_EMPTY(IoCompleteRequest)
DEFINE_STUB_EMPTY(IoCreateDevice)
DEFINE_STUB_EMPTY(IoDeleteDevice)
DEFINE_STUB_EMPTY(IoDismountVolume)
DEFINE_STUB_EMPTY(IoDismountVolumeByFileHandle)
DEFINE_STUB_EMPTY(IoReadFile)
DEFINE_STUB_EMPTY(IoWriteFile)
DEFINE_STUB_EMPTY(IoRemoveShareAccess)
DEFINE_STUB_EMPTY(IoSetShareAccess)
DEFINE_STUB_EMPTY(IoSetInformationFile)
DEFINE_STUB_EMPTY(IoGetDeviceInterfaces)
DEFINE_STUB_EMPTY(IoSetDeviceInterfaceState)
DEFINE_STUB_EMPTY(IoCreateFile)
DEFINE_STUB_EMPTY(IoOpenFile)

// ============================================
// Memory functions
// ============================================

DEFINE_STUB_EMPTY(MmAllocatePhysicalMemoryEx)
DEFINE_STUB_EMPTY(MmFreePhysicalMemory)
DEFINE_STUB_EMPTY(MmGetPhysicalAddress)
DEFINE_STUB_EMPTY(MmQueryAddressProtect)
DEFINE_STUB_EMPTY(MmQueryAllocationSize)

// ============================================
// Object functions
// ============================================

DEFINE_STUB_EMPTY(ObReferenceObject)
DEFINE_STUB_EMPTY(ObReferenceObjectByHandle)
DEFINE_STUB_EMPTY(ObDereferenceObject)
DEFINE_STUB_EMPTY(ObIsTitleObject)
DEFINE_STUB_EMPTY(ObCreateSymbolicLink)
DEFINE_STUB_EMPTY(ObDeleteSymbolicLink)

// ============================================
// RTL (Runtime Library) functions
// ============================================

DEFINE_STUB_EMPTY(ExEnterCriticalSection)
DEFINE_STUB_EMPTY(ExLeaveCriticalSection)

DEFINE_STUB_EMPTY(RtlEnterCriticalSection)
DEFINE_STUB_EMPTY(RtlLeaveCriticalSection)
DEFINE_STUB_EMPTY(RtlTryEnterCriticalSection)
DEFINE_STUB_EMPTY(RtlInitializeCriticalSection)
DEFINE_STUB_EMPTY(RtlFreeCriticalSection)
DEFINE_STUB_EMPTY(RtlAllocateHeap)
DEFINE_STUB_EMPTY(RtlFreeHeap)
DEFINE_STUB_EMPTY(RtlReAllocateHeap)
DEFINE_STUB_EMPTY(RtlSizeHeap)
DEFINE_STUB_EMPTY(RtlCompareMemoryUlong)
DEFINE_STUB_EMPTY(RtlCompareStringN)
DEFINE_STUB_EMPTY(RtlFillMemoryUlong)
DEFINE_STUB_EMPTY(RtlImageXexHeaderField)
DEFINE_STUB_EMPTY(RtlInitAnsiString)
DEFINE_STUB_EMPTY(RtlMultiByteToUnicodeN)
DEFINE_STUB_EMPTY(RtlUnicodeToMultiByteN)
DEFINE_STUB_EMPTY(RtlTimeFieldsToTime)
DEFINE_STUB_EMPTY(RtlTimeToTimeFields)

// Standard C functions - these are declared as PPC functions with different signatures
// to avoid conflict with the standard library versions, we use :: to reference our
// own definitions
extern "C" void ppc___imp__sprintf(PPCContext& ctx, uint8_t* base) {}
extern "C" void ppc____imp__sprintf(PPCContext& ctx, uint8_t* base) { ppc___imp__sprintf(ctx, base); }

// ============================================
// Network functions
// ============================================

DEFINE_STUB_EMPTY(NetDll_accept)
DEFINE_STUB_EMPTY(NetDll_bind)
DEFINE_STUB_EMPTY(NetDll_closesocket)
DEFINE_STUB_EMPTY(NetDll_connect)
DEFINE_STUB_EMPTY(NetDll_getsockname)
DEFINE_STUB_EMPTY(NetDll_inet_addr)
DEFINE_STUB_EMPTY(NetDll_ioctlsocket)
DEFINE_STUB_EMPTY(NetDll_listen)
DEFINE_STUB_EMPTY(NetDll_recv)
DEFINE_STUB_EMPTY(NetDll_recvfrom)
DEFINE_STUB_EMPTY(NetDll_select)
DEFINE_STUB_EMPTY(NetDll_send)
DEFINE_STUB_EMPTY(NetDll_sendto)
DEFINE_STUB_EMPTY(NetDll_setsockopt)
DEFINE_STUB_EMPTY(NetDll_socket)
DEFINE_STUB_EMPTY(NetDll_WSACleanup)
DEFINE_STUB_EMPTY(NetDll_WSAGetLastError)
DEFINE_STUB_EMPTY(NetDll_WSAStartup)

// ============================================
// Xex loading functions
// ============================================

DEFINE_STUB_EMPTY(XexLoadImage)
DEFINE_STUB_EMPTY(XexLoadImageHeaders)
DEFINE_STUB_EMPTY(XexGetModuleHandle)
DEFINE_STUB_EMPTY(XexGetProcedureAddress)
DEFINE_STUB_EMPTY(XexUnloadImage)
DEFINE_STUB_EMPTY(XexCheckExecutablePrivilege)

// ============================================
// XAudio2 functions
// ============================================

DEFINE_STUB_EMPTY(XAudioGetSpeakerConfig)
DEFINE_STUB_EMPTY(XAudioGetVoiceCategoryVolume)
DEFINE_STUB_EMPTY(XAudioGetVoiceCategoryVolumeChangeMask)
DEFINE_STUB_EMPTY(XAudioRegisterRenderDriverClient)
DEFINE_STUB_EMPTY(XAudioSubmitRenderDriverFrame)
DEFINE_STUB_EMPTY(XAudioUnregisterRenderDriverClient)

// ============================================
// Xam (Xbox Account Management) functions
// ============================================

DEFINE_STUB_EMPTY(XamAlloc)
DEFINE_STUB_EMPTY(XamFree)
DEFINE_STUB_EMPTY(XamGetSystemVersion)
DEFINE_STUB_EMPTY(XamTaskShouldExit)
DEFINE_STUB_EMPTY(XeCryptSha)

// ============================================
// Debug functions
// ============================================

DEFINE_STUB_EMPTY(DbgBreakPoint)
DEFINE_STUB_EMPTY(DbgPrint)

// ============================================
// Hardware/Platform functions
// ============================================

DEFINE_STUB_EMPTY(HalReturnToFirmware)

// ============================================
// Floating Point Register Save/Restore
// ============================================
