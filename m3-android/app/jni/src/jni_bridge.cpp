// MCLA - JNI Bridge for Android
// This file provides the JNI interface between Java and the native mcla_main executable

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include <rex/rex_app.h>
#include <rex/system.h>
#include <rex/system/function_dispatcher.h>
#include <rex/system/kernel_state.h>
#include <rex/system/thread_state.h>
#include <rex/system/xthread.h>  // For XThread class definition

#include "larecomp_init.h"
#include "game_hooks.h"
#include "config.h"
#include "menu.h"
#include "gamepad_manager.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <jni.h>
#include <android/log.h>

#define LOG_TAG "MCLA_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

// Entry point address (defined in larecomp_register.cpp)
constexpr uint32_t kXstartAddress = 0x821322B8;

// Global state
static std::atomic<bool> g_running{false};
static std::atomic<bool> g_paused{false};
static std::thread* g_game_thread = nullptr;
static std::mutex g_thread_mutex;
static std::condition_variable g_thread_cv;

static rex::Runtime* g_runtime = nullptr;
static rex::runtime::ThreadState* g_thread_state = nullptr;
static float g_fps = 0.0f;

static int g_frame_count = 0;
static std::chrono::high_resolution_clock::time_point g_last_fps_time;

static int g_surface_width = 0;
static int g_surface_height = 0;
static std::atomic<bool> g_surface_ready{false};

static std::string g_assets_dir;
static std::string g_files_dir;
static std::string g_cache_dir;

// Helper function to extract assets from APK
bool ExtractAssetsFromAPK() {
    // Assets are already extracted by Java code, this is a placeholder
    // If needed, use AAssetManager to extract assets dynamically
    return true;
}

// Helper function to find XEX file
std::string FindXexFile() {
    // On Android, XEX is extracted to files_dir/assets/
    std::vector<std::string> search_paths = {
        g_files_dir + "/assets/default.xex",
        g_files_dir + "/default.xex",
        g_cache_dir + "/default.xex"
    };
    
    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            LOGI("Found XEX at: %s", path.c_str());
            return path;
        }
    }
    
    LOGE("XEX file not found in any expected location");
    return "";
}

// =============================================================
// Game launch
//
// Follows the ReXApp::ConstructRuntime + LaunchModule flow:
//   Setup(PPCImageConfig) -> register_modules -> LoadXexImage
//   -> PrepareModuleLaunch -> thread->Resume()
// The resumed XThread is the guest main thread; it invokes
// xstart() through the function dispatcher.
// =============================================================
static rex::system::object_ref<rex::system::XThread> g_module_thread;

bool LaunchGame() {
    if (!g_runtime || !g_runtime->function_dispatcher()) {
        LOGE("LaunchGame: runtime not ready");
        return false;
    }

    auto* runtime = g_runtime;

    // Multi-binary projects: codegen may have a register_modules callback
    if (PPCImageConfig.register_modules) {
        PPCImageConfig.register_modules(runtime->kernel_state());
    }

    // Load the XEX through the VFS (game:\ maps to game_data_root)
    auto status = runtime->LoadXexImage("game:\\default.xex");
    if (XFAILED(status)) {
        LOGE("LoadXexImage failed: 0x%08X", status);
        return false;
    }
    LOGI("XEX image loaded");

    // Prepare the guest main thread (suspended)
    g_module_thread = runtime->PrepareModuleLaunch();
    if (!g_module_thread) {
        LOGE("PrepareModuleLaunch failed");
        return false;
    }

    // Resume - the guest main thread runs xstart() from here
    g_module_thread->Resume();
    LOGI("Module launched - guest main thread running");
    return true;
}

// Game loop thread function
void gameThreadMain(const char* assets_dir, const char* files_dir, const char* cache_dir) {
    LOGI("Game thread started");

    // Store paths
    g_assets_dir = assets_dir ? assets_dir : "";
    g_files_dir = files_dir ? files_dir : "";
    g_cache_dir = cache_dir ? cache_dir : "";

    LOGI("Paths - assets: %s, files: %s, cache: %s",
         g_assets_dir.c_str(), g_files_dir.c_str(), g_cache_dir.c_str());

    // Initialize SDL3 (needed for events, gamepad, audio)
    // SDL_INIT_VIDEO is skipped when using GLSurfaceView (handled by Android)
    uint32_t sdl_flags = SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO;
    if (!SDL_Init(sdl_flags)) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    LOGI("SDL3 initialized successfully");

    // ===========================================
    // Runtime Initialization
    // ===========================================

    // Find XEX file - Java extracts it to <files_dir>/assets/default.xex,
    // so game_data_root is <files_dir>/assets (XEX sits directly in it).
    std::string xex_path = FindXexFile();
    std::filesystem::path xex_dir;
    if (!xex_path.empty()) {
        xex_dir = std::filesystem::path(xex_path).parent_path();
    } else {
        xex_dir = std::filesystem::path(g_files_dir) / "assets";
    }

    LOGI("Game data root: %s", xex_dir.string().c_str());

    // Configure runtime - native rendering: no GPU emulation plugin,
    // no injected backends. Kernel HLE is initialized by the kernel_init
    // callback; audio/input are optional.
    rex::RuntimeConfig config;
    config.tool_mode = false;  // Full runtime mode

    // Create runtime instance
    g_runtime = new rex::Runtime(
        xex_dir,           // game_data_root (contains XEX and assets)
        xex_dir,           // user_data_root
        {},                // update_data_root
        xex_dir / "cache", // cache_root
        xex_dir / "metadata" // metadata_root
    );

    // Display window / app context are owned by Android (GLSurfaceView);
    // the runtime only needs them for presentation which we handle ourselves.
    g_runtime->set_display_window(nullptr);

    // Setup the runtime with PPCImageConfig:
    // - initializes memory, VFS, kernel state, function dispatcher
    // - registers all 30k recompiled functions including xstart
    auto setup_status = g_runtime->Setup(PPCImageConfig, std::move(config));
    if (XFAILED(setup_status)) {
        LOGE("Runtime::Setup failed: 0x%08X", setup_status);
        delete g_runtime;
        g_runtime = nullptr;
        SDL_Quit();
        return;
    }
    LOGI("Runtime initialized successfully (xstart registered at 0x%08X)", kXstartAddress);

    // ===========================================
    // Initialize Menu / UI subsystems
    // ===========================================
    // Wait for surface to be created before initializing menus
    // (Surface dimensions needed for proper viewport setup)
    while (!g_surface_ready && g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOGI("Surface ready (%dx%d), initializing menu system", g_surface_width, g_surface_height);

    // No SDL window - we render into the GLSurfaceView's GL context,
    // so the SDL platform binding is skipped; the OpenGL3 renderer is
    // bound to the current context.
    mcla::GetMenus().Initialize(nullptr);
    mcla::GetMenus().SetSize(g_surface_width, g_surface_height);
    LOGI("Menu system initialized");

    // FPS counter
    g_last_fps_time = std::chrono::high_resolution_clock::now();
    g_frame_count = 0;

    // Main game loop - runs on game thread, handles game logic only.
    // Rendering happens on the GL thread (nativeOnDrawFrame) because
    // the GLSurfaceView owns the OpenGL context.
    LOGI("Entering main game loop...");
    g_running = true;

    while (g_running) {
        // Handle pause
        if (g_paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Process SDL events (touch/key events are pushed via JNI)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            mcla::GetMenus().HandleEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                g_running = false;
                break;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    g_running = false;
                    break;
                }
            }
        }

        // User pressed JUGAR - launch the guest main thread (runs xstart)
        if (mcla::GetMenus().GameStartRequested()) {
            mcla::GetMenus().ConsumeGameStartRequest();
            if (LaunchGame()) {
                mcla::GetMenus().SetGameRunning(true);
            }
        }

        // Calculate FPS
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_last_fps_time);
        if (elapsed.count() >= 1000) {
            g_fps = static_cast<float>(g_frame_count * 1000) / elapsed.count();
            g_last_fps_time = now;
            g_frame_count = 0;
        }
        g_frame_count++;
        mcla::GetMenus().SetFPS(g_fps);

        // Update gamepad state
        mcla::GetGamepadManager().Update();

        // Yield
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Cleanup
    LOGI("Game thread exiting");
    mcla::GetMenus().Shutdown();

    if (g_runtime) {
        g_runtime->Shutdown();
        delete g_runtime;
        g_runtime = nullptr;
    }

    SDL_Quit();
    LOGI("Game thread finished");
}

// JNI exports
extern "C" {

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeInit(JNIEnv* env, jobject obj,
                                              jstring assetsDir, jstring filesDir, jstring cacheDir) {
    const char* assets_dir = env->GetStringUTFChars(assetsDir, nullptr);
    const char* files_dir = env->GetStringUTFChars(filesDir, nullptr);
    const char* cache_dir = env->GetStringUTFChars(cacheDir, nullptr);

    LOGI("nativeInit: assets=%s, files=%s, cache=%s", assets_dir, files_dir, cache_dir);

    // Start game thread
    g_game_thread = new std::thread(gameThreadMain, assets_dir, files_dir, cache_dir);

    env->ReleaseStringUTFChars(assetsDir, assets_dir);
    env->ReleaseStringUTFChars(filesDir, files_dir);
    env->ReleaseStringUTFChars(cacheDir, cache_dir);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeRunGame(JNIEnv* env, jobject obj) {
    LOGI("nativeRunGame called");
    // Game is already running in the thread
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeShutdown(JNIEnv* env, jobject obj) {
    LOGI("nativeShutdown called");
    g_running = false;
    if (g_game_thread && g_game_thread->joinable()) {
        g_game_thread->join();
        delete g_game_thread;
        g_game_thread = nullptr;
    }
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnPause(JNIEnv* env, jobject obj) {
    LOGI("nativeOnPause called");
    g_paused = true;
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnResume(JNIEnv* env, jobject obj) {
    LOGI("nativeOnResume called");
    g_paused = false;
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnSurfaceCreated(JNIEnv* env, jobject obj,
                                                          jint width, jint height) {
    LOGI("nativeOnSurfaceCreated: %dx%d", width, height);
    g_surface_width = width;
    g_surface_height = height;
    g_surface_ready = true;
    g_thread_cv.notify_one();
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnSurfaceChanged(JNIEnv* env, jobject obj,
                                                          jint width, jint height) {
    LOGI("nativeOnSurfaceChanged: %dx%d", width, height);
    g_surface_width = width;
    g_surface_height = height;
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnDrawFrame(JNIEnv* env, jobject obj) {
    // Called from the GLSurfaceView's render thread.
    // This is the ONLY place rendering happens - the game thread owns
    // game state and this thread owns the OpenGL context.
    if (!mcla::GetMenus().IsInitialized()) return;

    // Update display size from surface
    mcla::GetMenus().SetSize(g_surface_width, g_surface_height);

    // Begin frame (calls ImGui_ImplOpenGL3_NewFrame, ImGui_ImplSDL3_NewFrame, ImGui::NewFrame internally)
    mcla::GetMenus().BeginFrame();

    // Render HUD or menu
    mcla::GetMenus().Render();

    // End frame - draw and swap (calls ImGui::Render() and ImGui_ImplOpenGL3_RenderDrawData internally)
    mcla::GetMenus().EndFrame();
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnTouchEvent(JNIEnv* env, jobject obj,
                                                      jint action, jfloat x, jfloat y, jint pointerId) {
    // Forward touch events to menu system
    SDL_Event event;
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.x = static_cast<int>(x * g_surface_width);
    event.button.y = static_cast<int>(y * g_surface_height);
    event.button.button = SDL_BUTTON_LEFT;
    event.button.down = 1;
    SDL_PushEvent(&event);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnKeyEvent(JNIEnv* env, jobject obj,
                                                    jint keyCode, jint action) {
    // Forward key events
    SDL_Event event;
    event.type = (action == 0) ? SDL_EVENT_KEY_UP : SDL_EVENT_KEY_DOWN;
    event.key.key = static_cast<SDL_Keycode>(keyCode);
    SDL_PushEvent(&event);
}

JNIEXPORT jint JNICALL
Java_com_mcla_recomp_MainActivity_nativeGetFPS(JNIEnv* env, jobject obj) {
    return static_cast<jint>(g_fps);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSetSurfaceSize(JNIEnv* env, jobject obj,
                                                        jint width, jint height) {
    g_surface_width = width;
    g_surface_height = height;
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSaveConfig(JNIEnv* env, jobject obj) {
    mcla::GetConfig().Save();
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeLoadConfig(JNIEnv* env, jobject obj) {
    mcla::GetConfig().Load();
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSetFPSVisible(JNIEnv* env, jobject obj, jboolean visible) {
    mcla::GetConfig().show_fps = (visible == JNI_TRUE);
}

JNIEXPORT jboolean JNICALL
Java_com_mcla_recomp_MainActivity_nativeIsGameRunning(JNIEnv* env, jobject obj) {
    return g_running ? JNI_TRUE : JNI_FALSE;
}

} // extern "C"