#!/bin/bash
# Environment setup for MCLA Codegen (M2)

# Tool paths
export CMAKE=/home/jhon/xbox\ 360\ recomps/tools/cmake-3.31.0-linux-x86_64/bin/cmake
export NINJA=/home/jhon/xbox\ 360\ recomps/tools/ninja
export PATH="/home/jhon/xbox 360 recomps/tools/cmake-3.31.0-linux-x86_64/bin:/home/jhon/xbox 360 recomps/tools/ninja:/home/jhon/xbox 360 recomps/tools/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"

# NDK paths
export ANDROID_NDK=/home/jhon/xbox\ 360\ recomps/tools/android-ndk-r26b

# SDK paths
export REXGLUE_SDK=/home/jhon/xbox\ 360\ recomps/rexglue-sdk
export REXGLUE_BUILD_HOST=/home/jhon/xbox\ 360\ recomps/rexglue-build-host

# Working directory
export MCLA_ROOT=/home/jhon/xbox\ 360\ recomps/m2-codegen
