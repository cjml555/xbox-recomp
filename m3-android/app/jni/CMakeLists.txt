cmake_minimum_required(VERSION 3.16)
project(mcla_android LANGUAGES C CXX)

# C++2b (C++23) standard (required by ReXGlue SDK)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Android configuration - must be set BEFORE toolchain
set(ANDROID_PLATFORM 24)
set(ANDROID_ABI arm64-v8a)
set(ANDROID_STL c++_shared)

# NDK sysroot (set by toolchain, fallback for local builds)
if(DEFINED ENV{ANDROID_SDK_ROOT})
    set(_ndk_base "$ENV{ANDROID_SDK_ROOT}/ndk/26.1.10909125")
elseif(DEFINED ENV{ANDROID_HOME})
    set(_ndk_base "$ENV{ANDROID_HOME}/ndk/26.1.10909125")
else()
    set(_ndk_base "/home/jhona/tools/android-sdk/ndk/26.1.10909125")
endif()
set(ANDROID_NDK_SYSROOT "${_ndk_base}/toolchains/llvm/prebuilt/linux-x86_64/sysroot" CACHE PATH "Android NDK sysroot")
link_directories("${_ndk_base}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/24")

# =============================================================================
# ReXGlue SDK Setup
# =============================================================================
set(REXSDK_DIR "/home/jhona/xbox360-recomp/rexglue-sdk" CACHE PATH "Path to ReXGlue SDK")

# Include SDK headers
include_directories(${REXSDK_DIR}/include)
include_directories(${REXSDK_DIR}/thirdparty/fmt/include)
include_directories(${REXSDK_DIR}/thirdparty/spdlog/include)
include_directories(${REXSDK_DIR}/thirdparty/simde)
include_directories(${REXSDK_DIR}/thirdparty/imgui)
include_directories(${REXSDK_DIR}/thirdparty/imgui/backends)
include_directories(${REXSDK_DIR}/thirdparty/sdl3/include)
include_directories(${REXSDK_DIR}/thirdparty/inja/third_party/include)
include_directories(${REXSDK_DIR}/thirdparty/renderdoc)

# ReXGlue vendored Vulkan HPP headers — fully compatible with sdk targets
include_directories(${REXSDK_DIR}/thirdparty/vulkan-headers/include)

# =============================================================================
# Generated Code Setup
# =============================================================================
set(GENERATED_DIR "/home/jhona/xbox360-recomp/m2-codegen/generated")

# Define generated sources
set(GENERATED_SOURCES
    ${GENERATED_DIR}/larecomp_init.cpp
    ${GENERATED_DIR}/larecomp_register.cpp
)

# Count the number of recomp files
file(GLOB RECOMP_SOURCES "${GENERATED_DIR}/larecomp_recomp.*.cpp")
list(APPEND GENERATED_SOURCES ${RECOMP_SOURCES})

list(LENGTH RECOMP_SOURCES RECOMP_SOURCES_COUNT)
message(STATUS "Found ${RECOMP_SOURCES_COUNT} generated recomp files")

# Add game hooks source (must be added before library creation)
list(APPEND GENERATED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/game_hooks.cpp)
list(APPEND GENERATED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/timer_precision.cpp)

# =============================================================================
# SDL3 Setup (using pre-built static library from Android SDK build)
# =============================================================================
set(SDL3_LIB "${REXSDK_DIR}/out/android-arm64/libSDL3.a")
set(SDL3_INCLUDE_DIR "${REXSDK_DIR}/thirdparty/sdl3/include")

if(NOT EXISTS "${SDL3_LIB}")
    message(WARNING "SDL3 static library not found: ${SDL3_LIB} — build Android SDK first")
endif()

# =============================================================================
# DearImgui Setup (for menus, HUD, virtual gamepad)
# =============================================================================
set(IMGUI_SOURCES
    ${REXSDK_DIR}/thirdparty/imgui/imgui.cpp
    ${REXSDK_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${REXSDK_DIR}/thirdparty/imgui/imgui_tables.cpp
    ${REXSDK_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${REXSDK_DIR}/thirdparty/imgui/backends/imgui_impl_sdl3.cpp
)

# =============================================================================
# Source Files
# =============================================================================

# Create static library from generated code (includes game_hooks.cpp)
add_library(larecomp_generated OBJECT ${GENERATED_SOURCES})
target_include_directories(larecomp_generated PUBLIC ${GENERATED_DIR})
target_compile_options(larecomp_generated PRIVATE -fno-strict-aliasing -fPIC -O2)

# Set include directories for generated code
target_include_directories(larecomp_generated PUBLIC
    ${REXSDK_DIR}/include
    ${REXSDK_DIR}/thirdparty/fmt/include
    ${REXSDK_DIR}/thirdparty/spdlog/include
    ${REXSDK_DIR}/thirdparty/simde
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =============================================================================
# Main Shared Library (for JNI)
# =============================================================================
# Note: main.cpp is NOT used for Android - jni_bridge.cpp contains the entry point
# for JNI-based initialization. main.cpp is for standalone testing/desktop.
add_library(mcla_main SHARED
    src/stubs.cpp
    src/menu.cpp
    src/config.cpp
    src/gamepad_manager.cpp
    src/vulkan_catalog.cpp
    src/shader_generator.cpp
    src/crash_logger.cpp
    src/crash_trace.cpp
    src/updater.cpp
    src/jni_bridge.cpp
)

# Include directories
target_include_directories(mcla_main PRIVATE
    ${REXSDK_DIR}/include
    ${GENERATED_DIR}
    ${SDL3_INCLUDE_DIR}
    ${REXSDK_DIR}/thirdparty/imgui
    ${REXSDK_DIR}/thirdparty/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set C++ standard
target_compile_features(mcla_main PRIVATE cxx_std_23)

# Add imgui sources to the main target
target_sources(mcla_main PRIVATE ${IMGUI_SOURCES})

# =============================================================================
# Runtime Libraries (from prebuilt SDK output)
# =============================================================================
set(RUNTIME_LIB_DIR "/home/jhona/xbox360-recomp/rexglue-sdk/out/android-arm64")

# Add prebuilt libraries as OBJECT libraries for proper linking
add_library(librexruntime SHARED IMPORTED GLOBAL)
set_target_properties(librexruntime PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/librexruntime.so
)

add_library(librexgpu-xenos SHARED IMPORTED GLOBAL)
set_target_properties(librexgpu-xenos PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/librexgpu-xenos.so
)

add_library(librexcodegen STATIC IMPORTED GLOBAL)
set_target_properties(librexcodegen PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/librexcodegen.a
)

add_library(libfmt STATIC IMPORTED GLOBAL)
set_target_properties(libfmt PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/libfmt.a
)

add_library(libo1heap STATIC IMPORTED GLOBAL)
set_target_properties(libo1heap PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/libo1heap.a
)

add_library(libdisasm STATIC IMPORTED GLOBAL)
set_target_properties(libdisasm PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/libdisasm.a
)

add_library(libaes128 STATIC IMPORTED GLOBAL)
set_target_properties(libaes128 PROPERTIES
    IMPORTED_LOCATION ${RUNTIME_LIB_DIR}/libaes128.a
)

# Add android library for asset extraction
add_library(android_shared SHARED IMPORTED GLOBAL)
set_target_properties(android_shared PROPERTIES
    IMPORTED_LOCATION "${ANDROID_NDK_SYSROOT}/usr/lib/aarch64-linux-android/24/libandroid.so"
)

# libc++_shared is handled automatically by AGP when ANDROID_STL=c++_shared
# Do NOT manually import it here - AGP will package it into the APK

# =============================================================================
# Link Everything
# =============================================================================
target_link_libraries(mcla_main PRIVATE
    $<TARGET_OBJECTS:larecomp_generated>
    librexgpu-xenos
    librexruntime
    librexcodegen
    libfmt
    libo1heap
    libdisasm
    libaes128
    ${SDL3_LIB}
    android_shared
    -ldl -lm -llog
)

# =============================================================================
# Build Options
# =============================================================================
target_compile_options(mcla_main PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -fPIC
    -ffp-contract=off
)

# Set position independent code
set_target_properties(mcla_main PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Android-specific target settings
set_target_properties(mcla_main PROPERTIES ANDROID_STL c++_shared)

# Set the sysroot for Android
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_compile_options(mcla_main PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -fPIC
        -ffp-contract=off
        -fomit-frame-pointer
        -fdata-sections
        -ffunction-sections
    )
endif()

# =============================================================================
# Assets
# =============================================================================
# Copy XEX file to assets directory
set(XEX_SOURCE "/home/jhona/xbox360-recomp/m2-codegen/assets/default.xex")
set(XEX_DEST "${CMAKE_CURRENT_BINARY_DIR}/assets/default.xex")

if(EXISTS "${XEX_SOURCE}")
    add_custom_command(TARGET mcla_main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${XEX_SOURCE}" "${XEX_DEST}"
        COMMENT "Copying XEX file to assets"
    )
else()
    message(WARNING "XEX file not found: ${XEX_SOURCE}")
endif()

message(STATUS "MCLA Android build configured")
message(STATUS "SDK Path: ${REXSDK_DIR}")
message(STATUS "Generated sources: ${RECOMP_SOURCES_COUNT} files")