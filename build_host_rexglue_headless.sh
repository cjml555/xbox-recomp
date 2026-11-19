#!/bin/bash
# Build ReXGlue SDK for Linux x86_64 in HEADLESS mode (no X11/Wayland/Vulkan UI).
# The rexglue CLI only needs rexcodegen + rexruntime; the UI subsystem is pulled
# in transitively but headless mode stubs out the X11/Wayland dependencies so the
# build succeeds in environments without those dev packages.
set -euo pipefail

ROOT="/home/jhon/xbox-recomp"
export PATH="$ROOT/tools/cmake-3.31.0-linux-x86_64/bin:$ROOT/tools/ninja:$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin:$PATH"
export LD_LIBRARY_PATH="$ROOT/tools/local-libs:$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/lib/x86_64-unknown-linux-gnu"
export PKG_CONFIG_PATH="$ROOT/tools/pkgconfig:${PKG_CONFIG_PATH:-}"

LLVM18="$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04"
BUILD="$ROOT/rexglue-build-host-headless"
rm -rf "$BUILD"
mkdir -p "$BUILD"

cd "$BUILD"
cmake -GNinja \
  -DCMAKE_MAKE_PROGRAM="$ROOT/tools/ninja" \
  -DCMAKE_C_COMPILER="$LLVM18/bin/clang" \
  -DCMAKE_CXX_COMPILER="$LLVM18/bin/clang++" \
  -DCMAKE_C_FLAGS="-march=x86-64-v2 -I$ROOT/tools/local-libs/include" \
  -DCMAKE_CXX_FLAGS="-march=x86-64-v2 -stdlib=libc++ -I$ROOT/tools/local-libs/include" \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" \
  -DCMAKE_BUILD_TYPE=Release \
  -DREXGLUE_BUILD_TESTS=OFF \
  -DREXGLUE_ENABLE_FIDELITYFX=OFF \
  -DREXGLUE_ENABLE_PERF_COUNTERS=OFF \
  -DREXGLUE_ENABLE_SANITIZERS=OFF \
  -DREXGLUE_ENABLE_TRACY=OFF \
  -DREXGLUE_USE_VULKAN=ON \
  -DREXGLUE_USE_D3D12=OFF \
  -DREXUI_HEADLESS=ON \
  "$ROOT/rexglue-sdk"

echo "=== BUILDING ==="
"$ROOT/tools/ninja" -j"$(nproc)" rexglue 2>&1 | tail -60