# Host build toolchain for codegen
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compiler configuration
set(CMAKE_C_COMPILER "/home/jhon/xbox 360 recomps/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin/clang")
set(CMAKE_CXX_COMPILER "/home/jhon/xbox 360 recomps/tools/clang+llvm-18.1.8-x86_64-linux-gnu-ubuntu-18.04/bin/clang++")

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64-v2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2 -stdlib=libc++")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")

# Additional include paths for stub headers (using no-space path)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/tmp/stubs/include")

# Build options
set(REXGLUE_BUILD_TESTS OFF)
set(REXGLUE_ENABLE_FIDELITYFX OFF)
set(REXGLUE_ENABLE_PERF_COUNTERS ON)
set(REXGLUE_ENABLE_SANITIZERS OFF)
set(REXGLUE_ENABLE_TRACY OFF)
set(REXGLUE_USE_VULKAN ON)
set(REXUI_HEADLESS ON)