#pragma once

#include <cstdint>

namespace rex {
class Runtime;
}

namespace mcla {

// Builds a host-PC -> guest-function map from the runtime's function dispatcher
// and installs an exception handler that logs the guest function responsible
// for guest access violations (null-page reads/writes). Diagnostic only.
// Pattern adapted from holdmysocks/mcla-recomp src/crash_trace.cpp.
void InstallCrashTrace(rex::Runtime* runtime);

}  // namespace mcla
