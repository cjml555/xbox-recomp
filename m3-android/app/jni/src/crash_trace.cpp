// Guest crash tracer for Android.
//
// When recompiled code faults on a guest address, log the host PC mapped back
// to the guest function it belongs to. Needs no debug symbols: host function
// addresses come from the runtime's function dispatch table.
//
// Pattern adapted from holdmysocks/mcla-recomp src/crash_trace.cpp (Win32 VEH
// there; here we use rex::arch::ExceptionHandler, which the ReXGlue SDK wires
// to SIGSEGV on POSIX/Android).

#include "crash_trace.h"

#include <dlfcn.h>
#include <android/log.h>
#include <signal.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include <rex/exception_handler.h>
#include <rex/ppc/context.h>
#include <rex/ppc/func.h>
#include <rex/runtime.h>
#include <rex/system/function_dispatcher.h>

#define CT_TAG "MCLA_CrashTrace"
#define CT_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, CT_TAG, __VA_ARGS__)
#define CT_LOGI(...) __android_log_print(ANDROID_LOG_INFO, CT_TAG, __VA_ARGS__)

namespace mcla {
namespace {

// MCLA Complete Edition code range (same bounds as holdmysocks crash_trace).
constexpr uint32_t kCodeBegin = 0x82130000;
constexpr uint32_t kCodeEnd = 0x827CD054;
// Largest plausible distance from a function's entry to an address inside it.
constexpr uintptr_t kMaxFunctionSpan = 0x80000;

std::vector<std::pair<uintptr_t, uint32_t>> g_host_to_guest;
std::vector<uint32_t> g_guest_entries;  // sorted guest function entries
std::atomic<uint64_t> g_av_count{0};
uintptr_t g_guest_base = 0;

uint32_t GuestFunctionContaining(uint32_t guest_addr, uint32_t* offset) {
    auto it = std::upper_bound(g_guest_entries.begin(), g_guest_entries.end(), guest_addr);
    if (it == g_guest_entries.begin()) {
        return 0;
    }
    --it;
    if (guest_addr - *it > 0x20000) {
        return 0;
    }
    *offset = guest_addr - *it;
    return *it;
}

uint32_t GuestFunctionFor(uintptr_t host_pc, uintptr_t* offset) {
    auto it = std::upper_bound(g_host_to_guest.begin(), g_host_to_guest.end(),
                               std::make_pair(host_pc, UINT32_MAX));
    if (it == g_host_to_guest.begin()) {
        return 0;
    }
    --it;
    if (host_pc - it->first > kMaxFunctionSpan) {
        return 0;
    }
    *offset = host_pc - it->first;
    return it->second;
}

bool OnException(rex::arch::Exception* ex, void* /*data*/) {
    if (ex->code() != rex::arch::Exception::Code::kAccessViolation) {
        return false;
    }
    const uint64_t n = g_av_count.fetch_add(1, std::memory_order_relaxed);
    // Log the first 12 faults in full detail, then one every 50000 so the
    // infinite-retry loop does not drown logcat.
    if (n >= 12 && (n % 50000) != 0) {
        return false;
    }

    const uintptr_t pc = static_cast<uintptr_t>(ex->pc());
    const uint64_t fault = ex->fault_address();
    const bool is_write =
        ex->access_violation_operation() == rex::arch::Exception::AccessViolationOperation::kWrite;

    uintptr_t offset = 0;
    const uint32_t guest_fn = GuestFunctionFor(pc, &offset);

    // Resolve the hosting shared library + offset for offline addr2line.
    Dl_info info{};
    const char* lib = "?";
    uintptr_t lib_off = 0;
    if (dladdr(reinterpret_cast<void*>(pc), &info) && info.dli_fname) {
        lib = info.dli_fname;
        lib_off = pc - reinterpret_cast<uintptr_t>(info.dli_fbase);
    }

    uint32_t guest_fault = 0;
    if (g_guest_base && fault >= g_guest_base) {
        guest_fault = static_cast<uint32_t>(fault - g_guest_base);
    }

    if (guest_fn) {
        CT_LOGE("AV #%llu %s guest 0x%08X | host pc %p = sub_%08X+0x%lX | %s+0x%lX",
                static_cast<unsigned long long>(n), is_write ? "write" : "read",
                guest_fault, reinterpret_cast<void*>(pc), guest_fn,
                static_cast<unsigned long>(offset), lib,
                static_cast<unsigned long>(lib_off));
    } else {
        CT_LOGE("AV #%llu %s guest 0x%08X | host pc %p (no guest fn) | %s+0x%lX",
                static_cast<unsigned long long>(n), is_write ? "write" : "read",
                guest_fault, reinterpret_cast<void*>(pc), lib,
                static_cast<unsigned long>(lib_off));
    }

    // Recover the guest PPCContext from callee-saved host registers: the
    // recompiled code keeps &ctx in one of x19..x28. Validate via lr (must be
    // a guest code address) and r1 (guest stack). Then log the guest LR —
    // the return address into the CALLER — plus key argument registers.
    if (n < 12) {
        auto* hctx = ex->thread_context();
        for (int i = 19; i <= 28; ++i) {
            const uintptr_t cand = hctx->x[i];
            // PPCContext is alignas(0x40) and lives in host memory (never in
            // the guest reservation). Filter garbage before dereferencing.
            if (cand < 0x100000000ull || (cand & 0x3F)) continue;
            if (g_guest_base && cand >= g_guest_base &&
                cand < g_guest_base + 0x100000000ull) continue;
            auto* ctx = reinterpret_cast<PPCContext*>(cand);
            const uint32_t lr = static_cast<uint32_t>(ctx->lr);
            const uint32_t sp = ctx->r1.u32;
            if (lr < kCodeBegin || lr >= kCodeEnd) continue;
            if (sp < 0x10000 || (sp & 3)) continue;
            uint32_t call_off = 0;
            const uint32_t caller = GuestFunctionContaining(lr, &call_off);
            CT_LOGE("  ctx=x%d: guest lr=0x%08X (caller sub_%08X+0x%X) r3=0x%08X r4=0x%08X r1=0x%08X r31=0x%08X",
                    i, lr, caller, call_off, ctx->r3.u32, ctx->r4.u32, sp,
                    ctx->r31.u32);
            break;
        }
    }
    return false;  // observer only; let the SDK's own handling proceed
}

// Last-resort handler in the exception chain: if no handler claims the fault
// (i.e. the runtime's guest-access-violation emulation did not take it), re-raise
// the signal with SIG_DFL so Android's debuggerd writes a tombstone. Without this,
// rex::arch::ExceptionHandlerCallback returns without re-raising and the faulting
// instruction re-executes → the process hangs in an infinite SIGSEGV loop and we
// get NO diagnostic output — exactly what you don't want while debugging a boot.
// Must be installed AFTER any handler that legitimately handles guest AVs (FIFO),
// so it only fires for truly unhandled faults.
bool ReRaiseOnUnhandled(rex::arch::Exception* ex, void* /*data*/) {
    (void)ex;
    struct sigaction sa{};
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigaction(SIGSEGV, &sa, nullptr);
    raise(SIGSEGV);
    return false;  // unreached; keeps the compiler happy
}

}  // namespace

void InstallCrashTrace(rex::Runtime* runtime) {
    if (!runtime || !runtime->function_dispatcher()) {
        CT_LOGE("InstallCrashTrace: runtime/dispatcher not ready");
        return;
    }
    g_guest_base = reinterpret_cast<uintptr_t>(runtime->virtual_membase());
    auto* dispatcher = runtime->function_dispatcher();
    g_host_to_guest.clear();
    for (uint32_t address = kCodeBegin; address < kCodeEnd; address += 4) {
        if (auto* function = dispatcher->GetFunction(address)) {
            g_host_to_guest.emplace_back(reinterpret_cast<uintptr_t>(function), address);
        }
    }
    std::sort(g_host_to_guest.begin(), g_host_to_guest.end());
    g_guest_entries.clear();
    g_guest_entries.reserve(g_host_to_guest.size());
    for (const auto& entry : g_host_to_guest) {
        g_guest_entries.push_back(entry.second);
    }
    std::sort(g_guest_entries.begin(), g_guest_entries.end());
    CT_LOGI("crash trace: %zu guest functions mapped, guest base %p",
            g_host_to_guest.size(), reinterpret_cast<void*>(g_guest_base));
    rex::arch::ExceptionHandler::Install(OnException, nullptr);
    // Register the re-raise fallback LAST (FIFO), so it only fires when nothing
    // else — including the runtime's own guest-AV handler — claimed the fault.
    rex::arch::ExceptionHandler::Install(ReRaiseOnUnhandled, nullptr);
    CT_LOGI("crash trace: fallback SIGSEGV re-raise installed (debuggerd tombstone path)");
}

}  // namespace mcla
