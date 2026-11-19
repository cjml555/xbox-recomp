#!/bin/bash
set -e
ROOT="/home/jhon/xbox 360 recomps"
export PATH="$ROOT/tools/cmake-3.31.0-linux-x86_64/bin:$ROOT/tools/ninja:$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin:$ROOT/tools/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"
export LD_LIBRARY_PATH="$ROOT/tools/local-libs:$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/lib/x86_64-unknown-linux-gnu"
export PKG_CONFIG_PATH="$ROOT/tools/pkgconfig:$PKG_CONFIG_PATH"

LLVM18="$ROOT/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04"
BUILD="$ROOT/rexglue-build-host"
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
  -DREXGLUE_ENABLE_PERF_COUNTERS=ON \
  -DREXGLUE_ENABLE_SANITIZERS=OFF \
  -DREXGLUE_ENABLE_TRACY=OFF \
  -DREXGLUE_USE_VULKAN=ON \
  "$ROOT/rexglue-sdk"

echo "=== BUILDING ==="
"$ROOT/tools/ninja" -j$(nproc) rexglue 2>&1 | tail -50