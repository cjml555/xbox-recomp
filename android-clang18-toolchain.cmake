# Custom Android toolchain using standalone Clang 18 with NDK r26b sysroot
# This must be included BEFORE project() in CMakeLists.txt

# Set the compilers to our standalone Clang 18
set(CMAKE_C_COMPILER "/home/jhona/tools/clang+llvm-18.1.8/bin/clang" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "/home/jhona/tools/clang+llvm-18.1.8/bin/clang++" CACHE FILEPATH "C++ compiler" FORCE)

# NDK sysroot
set(ANDROID_NDK_SYSROOT "/home/jhona/tools/android-ndk-r26b/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/sysroot")

# Target flags
set(ANDROID_TARGET_FLAGS "--target=aarch64-linux-android24 --sysroot=${ANDROID_NDK_SYSROOT}")

# Add target flags to all compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ANDROID_TARGET_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ANDROID_TARGET_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ANDROID_TARGET_FLAGS}" CACHE STRING "" FORCE)

# Linker flags
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ANDROID_TARGET_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ANDROID_TARGET_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${ANDROID_TARGET_FLAGS}" CACHE STRING "" FORCE)

# Cross-compilation settings
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 24)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Find libraries in NDK sysroot
set(CMAKE_FIND_ROOT_PATH "${ANDROID_NDK_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Android specific settings
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)
set(ANDROID_ABI arm64-v8a)
set(ANDROID_PLATFORM android-24)

# Disable tests
set(BUILD_TESTING OFF)
