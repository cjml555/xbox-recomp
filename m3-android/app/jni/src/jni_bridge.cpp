// MCLA - JNI Bridge for Android
// Full-native Vulkan + SDLWindowedAppContext path. No OpenGL ES dependency.
// The game loop runs on the Java-managed thread (started by nativeInit),
// creates an SDL window on Android's SurfaceView surface, and uses the
// ReXGlue presenter/ImmediateDrawer stack for menu rendering via Vulkan.

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <rex/rex_app.h>
#include <rex/system.h>
#include <rex/system/function_dispatcher.h>
#include <rex/system/kernel_state.h>
#include <rex/system/thread_state.h>
#include <rex/system/xthread.h>
#include <rex/system/gpu_plugin.h>
#include <rex/kernel/crt/heap.h>
#include <rex/kernel/init.h>
#include <rex/input/input_system.h>
#include <rex/input/device_assignment.h>
#include <rex/filesystem/vfs.h>
#include <rex/cvar.h>
#include <rex/logging/api.h>
#include <spdlog/sinks/android_sink.h>

#include "larecomp_init.h"
#include "game_hooks.h"
#include "config.h"
#include "menu.h"
#include "gamepad_manager.h"

#include <rex/ui/windowed_app_context_sdl.h>
#include <rex/ui/presenter.h>
#include <rex/ui/immediate_drawer.h>
#include <rex/ui/ui_drawer.h>
#include <rex/ui/window.h>
#include <rex/ui/window_sdl.h>
#include <rex/ui/graphics_provider.h>

#include <dlfcn.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <signal.h>

#include "crash_trace.h"
#include "audio_output.h"

#define LOG_TAG "MCLA_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

extern "C" {
void __assert2(const char* file, int line, const char* func, const char* expr) {
    __android_log_print(ANDROID_LOG_ERROR, "MCLA_JNI",
        "ASSERT FAIL: %s:%d (%s): %s", file, line, func ? func : "?", expr);
    abort();
}
}

constexpr uint32_t kXstartAddress = 0x821322B8;

static struct sigaction g_old_sa_sigsegv, g_old_sa_sigabrt, g_old_sa_sigbus;

static void crash_signal_handler(int signum, siginfo_t* info, void* ucontext) {
    __android_log_print(ANDROID_LOG_ERROR, "MCLA_JNI",
        "CRASH: signal %d (%s), si_addr=%p, pid=%d, tid=%d, ucontext=%p",
        signum,
        signum == SIGSEGV ? "SIGSEGV" :
        signum == SIGABRT ? "SIGABRT" :
        signum == SIGBUS ? "SIGBUS" :
        signum == SIGILL ? "SIGILL" : "UNKNOWN",
        info ? info->si_addr : nullptr,
        getpid(), gettid(), ucontext);
    signal(signum, SIG_DFL);
    raise(signum);
}

static void install_crash_handler(int sig) {
    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = crash_signal_handler;
    sigaction(sig, &sa, sig == SIGSEGV ? &g_old_sa_sigsegv :
                            sig == SIGABRT ? &g_old_sa_sigabrt :
                            &g_old_sa_sigbus);
}

// Global state
static std::atomic<bool> g_running{false};
static std::atomic<bool> g_paused{false};
static std::thread* g_game_thread = nullptr;

static rex::Runtime* g_runtime = nullptr;
static float g_fps = 0.0f;
static int g_frame_count = 0;
static int g_target_fps = 120;  // Edge 50 Fusion runs 120 Hz; 0 = uncapped
// Per-frame time tracking for the MangoHUD-style frametime graph.
static std::chrono::high_resolution_clock::time_point g_last_frame_time;
static std::chrono::high_resolution_clock::time_point g_last_fps_time;

static JNIEnv* g_jni_env = nullptr;
static JavaVM* g_jvm = nullptr;

// Surface dimensions — published when Java calls nativeOnSurfaceCreated.
static std::atomic<int> g_surface_width{0};
static std::atomic<int> g_surface_height{0};
static std::atomic<bool> g_surface_ready{false};

static std::string g_assets_dir;
static std::string g_files_dir;
static std::string g_cache_dir;

// Asset manager for loading menu textures from APK assets.
static AAssetManager* g_asset_manager = nullptr;

// Native library directory passed from Java (for GPU plugin loading).
static std::string g_native_library_dir;

// Presentation wiring (filled after surface ready + runtime setup).
static std::unique_ptr<rex::ui::SDLWindowedAppContext> g_app_context;
static rex::ui::Presenter* g_presenter = nullptr;
static rex::ui::GraphicsProvider* g_provider = nullptr;
static std::unique_ptr<rex::ui::WindowSDL> g_window;
static std::unique_ptr<rex::ui::ImmediateDrawer> g_immediate_drawer;

std::string FindXexFile() {
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

static std::unique_ptr<rex::system::IGraphicsSystem> LoadGpuPluginFromPath(
        const std::string& plugin_path, const std::string& backend) {
    if (!std::filesystem::exists(plugin_path)) {
        LOGE("GPU plugin not found at: %s", plugin_path.c_str());
        return nullptr;
    }
    void* handle = dlopen(plugin_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        LOGE("GPU plugin failed to load: %s", dlerror());
        return nullptr;
    }
    using GpuAbiVersionFn = uint32_t (*)();
    using GpuCreateFn = rex::system::IGraphicsSystem* (*)(uint32_t, const rex::system::GpuCreateInfo*);
    auto abi_version_fn = reinterpret_cast<GpuAbiVersionFn>(dlsym(handle, "rex_gpu_abi_version"));
    auto create_fn = reinterpret_cast<GpuCreateFn>(dlsym(handle, "rex_gpu_create"));
    if (!abi_version_fn || !create_fn) {
        LOGE("GPU plugin missing exports: %s", plugin_path.c_str());
        dlclose(handle);
        return nullptr;
    }
    uint32_t plugin_abi = abi_version_fn();
    if (plugin_abi != 1) {
        LOGE("GPU plugin ABI mismatch: got %u, expected 1", plugin_abi);
        dlclose(handle);
        return nullptr;
    }
    rex::system::GpuCreateInfo info{};
    info.struct_size = sizeof(rex::system::GpuCreateInfo);
    info.backend = backend.c_str();
    rex::system::IGraphicsSystem* gs = create_fn(1, &info);
    if (!gs) {
        LOGE("GPU plugin factory returned null");
        dlclose(handle);
        return nullptr;
    }
    static std::vector<void*> s_handles;
    s_handles.push_back(handle);
    LOGI("GPU plugin loaded from: %s", plugin_path.c_str());
    return std::unique_ptr<rex::system::IGraphicsSystem>(gs);
}

static rex::system::object_ref<rex::system::XThread> g_module_thread;

bool LaunchGame() {
    if (!g_runtime || !g_runtime->function_dispatcher()) {
        LOGE("LaunchGame: runtime not ready");
        return false;
    }
    auto* runtime = g_runtime;
    if (PPCImageConfig.register_modules) {
        PPCImageConfig.register_modules(runtime->kernel_state());
    }
    auto status = runtime->LoadXexImage("game:\\default.xex");
    if (XFAILED(status)) {
        LOGE("LoadXexImage failed: 0x%08X", status);
        return false;
    }
    LOGI("XEX image loaded");
    // Initialize the guest CRT heap (mirrors ReXApp::Setup flow, rex_app.cpp).
    // Without this, rexcrt_RtlAllocateHeap has no backing heap and the guest
    // hangs/crashes at boot (black screen).
    if (PPCImageConfig.rexcrt_heap) {
        uint32_t heap_mb = REXCVAR_GET(rexcrt_heap_size_mb);
        LOGI("Initializing rexcrt heap (%u MB)...", heap_mb);
        if (!rex::kernel::crt::InitHeap(heap_mb, runtime->memory())) {
            LOGE("Failed to initialize rexcrt heap");
            return false;
        }
        LOGI("rexcrt heap initialized (%u MB)", heap_mb);
    }
    LOGI("Preparing module launch...");
    g_module_thread = runtime->PrepareModuleLaunch();
    if (!g_module_thread) {
        LOGE("PrepareModuleLaunch failed");
        return false;
    }
    // Install guest crash tracer (host PC -> guest function mapping) before
    // the guest starts faulting, so access violations can be attributed.
    mcla::InstallCrashTrace(runtime);
    g_module_thread->Resume();
    LOGI("Module launch initiated - guest main thread running");
    return true;
}

void gameThreadMain(std::string assets_dir, std::string files_dir, std::string cache_dir) {
    LOGI("Game thread started");
    if (g_jvm) g_jvm->AttachCurrentThread(&g_jni_env, nullptr);

    g_assets_dir = std::move(assets_dir);
    g_files_dir = std::move(files_dir);
    g_cache_dir = std::move(cache_dir);

    LOGI("Paths - assets: %s, files: %s, cache: %s",
         g_assets_dir.c_str(), g_files_dir.c_str(), g_cache_dir.c_str());

    // Initialize SDL3 — VIDEO is required for SDL_CreateWindow on Android.
    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return;
    }
    LOGI("SDL3 initialized successfully");

    // Locate XEX
    std::string xex_path = FindXexFile();
    std::filesystem::path xex_dir;
    if (!xex_path.empty()) {
        xex_dir = std::filesystem::path(xex_path).parent_path();
    } else {
        xex_dir = std::filesystem::path(g_files_dir) / "assets";
    }
    LOGI("Game data root: %s", xex_dir.string().c_str());

    // Load GPU plugin (Vulkan)
    rex::RuntimeConfig config;
    config.tool_mode = false;
    // HLE kernel modules + XAM apps (mirrors ReXApp::SetupPresentation).
    // Without this, xboxkrnl.exe/xam.xex are not registered and guest
    // XexLoadImage/XexGetProcedureAddress calls fail at boot.
    config.kernel_init = rex::kernel::InitializeKernel;
    // Audio with silent fallback (LARecomp audio_output.h / holdmysocks
    // audio_fallback.cpp): without a driver the game aborts its audio engine
    // init and later dereferences the engine it never created.
    config.audio_factory = &mcla::audio::CreateAudioSystem;
    // Every controller is player one (LARecomp larecomp_app.h): the SDK
    // default assigns each pad its own user, and user 1+ has no profile, so
    // the game reports nobody signed in and never saves.
    config.input_factory = [](bool tool_mode) -> std::unique_ptr<rex::system::IInputSystem> {
        auto input = rex::input::CreateDefaultInputSystem(tool_mode);
        if (input) {
            input->SetDeviceAssignment(std::make_unique<rex::input::SharedAssignment>());
            // Android has no rex window, so the SDL input driver never
            // initializes SDL_INIT_GAMEPAD and the guest never sees the on-screen
            // gamepad. Bridge the mcla virtual/physical gamepad state directly.
            mcla::RegisterMclaInputDriver(
                static_cast<rex::input::InputSystem*>(input.get()));
        }
        return input;
    };
    auto exe_folder = rex::filesystem::GetExecutableFolder();
    LOGI("Executable folder: %s", exe_folder.string().c_str());
    LOGI("Native library dir: %s", g_native_library_dir.c_str());

    // MCLA ships with "invalid" texture fetch constants on some draw paths; the
    // GPU cache rejects them by default and spams a validation warning per draw
    // (50k+ logcat lines/min). LARecomp enables this flag by default — mirror it.
    rex::cvar::SetFlagByName("gpu_allow_invalid_fetch_constants", "true");

    // rex::SetAllLevels(trace) below makes every gpu_logger->should_log() check
    // pass, so the SDK creates the debug-utils messenger with all severities and
    // all message types. The Android driver emits an empty ID-0 message with an
    // invalid severity every frame; formatting it with vk::to_string and writing
    // 100+ logcat lines/s burns CPU on the render thread and hides real errors.
    // Real Vulkan failures still surface via REXLOG_ERROR at the call sites.
    rex::cvar::SetFlagByName("vulkan_log_debug_messages", "false");

    config.graphics = nullptr;
    if (!g_native_library_dir.empty()) {
        std::string plugin_path = g_native_library_dir + "/librexgpu-xenos.so";
        LOGI("Attempting to load GPU plugin from: %s", plugin_path.c_str());
        config.graphics = LoadGpuPluginFromPath(plugin_path, "vulkan");
    }
    if (!config.graphics) {
        config.graphics = rex::system::LoadGpuPlugin("xenos", "vulkan");
        if (!config.graphics) {
            LOGE("Failed to load GPU plugin 'xenos' — guest rendering disabled");
        } else {
            LOGI("GPU plugin 'xenos' loaded successfully (fallback)");
        }
    }

    // Create runtime instance
    g_runtime = new rex::Runtime(
        xex_dir, xex_dir, {},
        xex_dir / "cache",
        xex_dir / "metadata"
    );
    g_runtime->set_display_window(nullptr);

    // Route spdlog output to logcat.
    try {
        auto android_sink = std::make_shared<spdlog::sinks::android_sink_mt>();
        android_sink->set_level(spdlog::level::trace);
        rex::AddSink(android_sink);
        rex::SetAllLevels(spdlog::level::trace);
    } catch (...) {
        LOGW("Failed to attach spdlog android sink");
    }

    // Wait for surface ready before calling Runtime::Setup (which needs app_context).
    LOGI("Waiting for surface...");
    while (!g_surface_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    int surf_w = g_surface_width.load();
    int surf_h = g_surface_height.load();
    LOGI("Surface ready (%dx%d)", surf_w, surf_h);

    // Construct SDLWindowedAppContext on this (game) thread — must happen BEFORE Setup.
    g_app_context = std::make_unique<rex::ui::SDLWindowedAppContext>();
    g_app_context->Initialize();
    LOGI("SDLWindowedAppContext created and initialized");

    g_runtime->set_app_context(g_app_context.get());
    LOGI("App context set on runtime");

    auto setup_status = g_runtime->Setup(PPCImageConfig, std::move(config));
    if (XFAILED(setup_status)) {
        LOGE("Runtime::Setup failed: 0x%08X", setup_status);
        delete g_runtime;
        g_runtime = nullptr;
        SDL_Quit();
        return;
    }
    LOGI("Runtime initialized (xstart at 0x%08X)", kXstartAddress);

    // Self-test: verify the AAudio output driver works on this device before the
    // guest code requests it during gameplay. Opens and closes a stream once.
    // This is diagnostic-only and never blocks boot on failure.
    //
    // Device quirks encountered on Motorola Edge 50 Fusion (Android 16):
    //   - AAUDIO_DIRECTION_OUTPUT is 0, not 1 (1 = INPUT = createRecord → permission denied)
    //   - AAUDIO_FORMAT_PCM_FLOAT = 2, not 1 (1 = PCM_I16)
    //   - AAUDIO_PERFORMANCE_MODE_LOW_LATENCY is rejected (illegal performanceMode),
    //     so we leave the default NONE.
    if (g_runtime->function_dispatcher()) {
        auto* mem = g_runtime->memory();
        auto sem = rex::thread::Semaphore::Create(0, 1);
        if (mem && sem) {
            mcla::audio::AAudioDriver probe(mem, sem.get());
            if (probe.Initialize()) {
                LOGI("AAUDIO_SELFTEST OK: stream opened at 48 kHz stereo float");
                probe.Shutdown();
            } else {
                LOGW("AAUDIO_SELFTEST FAILED: AAudio unavailable — will fall back to SDL/null");
            }
        } else {
            LOGW("AAUDIO_SELFTEST SKIPPED: no memory or semaphore");
        }
    }

    // Register t: drive — the game probes it for loose city/art/collision
    // data (.loc files). Without the device the probes fail with "no such
    // device" instead of "not found" (LARecomp larecomp_app.h:1200,
    // holdmysocks mcla_app.cpp OnPostSetup).
    if (auto* fs = g_runtime->file_system()) {
        fs->RegisterSymbolicLink("t:", "\\Device\\Harddisk0\\Partition1");
        LOGI("Registered t: symbolic link");
    }

    // Wire presenter + immediate drawer + window + menus (mirrors ReXApp flow).
    auto* gs = g_runtime->graphics_system();
    g_presenter = gs ? gs->presenter() : nullptr;
    g_provider = gs ? gs->provider() : nullptr;

    if (g_presenter && g_provider) {
        g_window = std::make_unique<rex::ui::WindowSDL>(
            *g_app_context, "MCLA",
            static_cast<uint32_t>(surf_w),
            static_cast<uint32_t>(surf_h)
        );
        if (!g_window->Open()) {
            LOGE("WindowSDL::Open() FAILED");
        } else {
            LOGI("WindowSDL opened (physical %dx%d)",
                 g_window->GetActualPhysicalWidth(),
                 g_window->GetActualPhysicalHeight());
        }
        g_immediate_drawer = g_provider->CreateImmediateDrawer();
        if (g_immediate_drawer) {
            g_immediate_drawer->SetPresenter(g_presenter);
            LOGI("ImmediateDrawer created and presenter set");
        } else {
            LOGE("Failed to create ImmediateDrawer");
        }
        if (g_window) {
            g_window->SetPresenter(g_presenter);
        }
    } else {
        LOGE("Skipping presentor wiring (presenter=%p provider=%p)",
             (void*)g_presenter, (void*)g_provider);
    }

    mcla::GetMenus().SetSize(surf_w, surf_h);
    mcla::GetMenus().Initialize(g_immediate_drawer.get());
    if (g_asset_manager) {
        mcla::GetMenus().LoadTextures(g_asset_manager);
    }
    LOGI("Menu system initialized (vulkan path)");

    if (g_presenter) {
        g_presenter->AddUIDrawerFromUIThread(&mcla::GetMenus(), /*z_order=*/0);
        LOGI("MenuSystem added as UIDrawer z=0");
    }

    g_last_fps_time = std::chrono::high_resolution_clock::now();
    g_frame_count = 0;

    LOGI("Entering main game loop...");
    g_running = true;

    while (g_running && !g_app_context->HasQuitFromUIThread()) {
        if (g_paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Poll SDL events forwarded from Java via JNI.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (g_window) {
                switch (event.type) {
                    case SDL_EVENT_KEY_DOWN:
                    case SDL_EVENT_KEY_UP:
                        g_window->HandleKeyEvent(event);
                        break;
                    case SDL_EVENT_MOUSE_MOTION:
                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        g_window->HandleMouseEvent(event);
                        break;
                    case SDL_EVENT_TEXT_INPUT:
                        g_window->HandleTextInputEvent(event);
                        break;
                    default:
                        break;
                }
            }
            mcla::GetMenus().HandleEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                g_running = false;
                break;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                g_running = false;
                break;
            }
        }

        if (mcla::GetMenus().GameStartRequested()) {
            mcla::GetMenus().ConsumeGameStartRequest();
            bool launched = LaunchGame();
            if (launched) {
                mcla::GetMenus().SetGameRunning(true);
            }
        }

        mcla::GetGamepadManager().Update();

        // Keep menu display size in sync with the latest surface dimensions
        // (the initial snapshot taken before Setup may predate a resize).
        {
            int sw = g_surface_width.load();
            int sh = g_surface_height.load();
            if (sw > 0 && sh > 0 &&
                (sw != mcla::GetMenus().GetWidth() || sh != mcla::GetMenus().GetHeight())) {
                mcla::GetMenus().SetSize(sw, sh);
            }
        }

        // FPS counter + periodic progress log
        auto now = std::chrono::high_resolution_clock::now();
        {
            auto dt = now - g_last_frame_time;
            float dt_ms = std::chrono::duration<float, std::milli>(dt).count();
            g_last_frame_time = now;
            mcla::GetMenus().AddFrametimeSample(dt_ms);
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_last_fps_time);
        if (elapsed.count() >= 1000) {
            g_fps = static_cast<float>(g_frame_count * 1000) / elapsed.count();
            g_last_fps_time = now;
            int frames_this_sec = g_frame_count;
            g_frame_count = 0;
            static int fps_log = 0;
            if (fps_log < 20) { ++fps_log;
                LOGI("FPS: %.1f, frames=%d, game_running=%d", g_fps, frames_this_sec, g_running ? 1 : 0);
            }
        }
        g_frame_count++;
        mcla::GetMenus().SetFPS(g_fps);

        // Block until the presenter composites the frame onto the surface.
        if (g_presenter) {
            g_presenter->PaintFromUIThread(/*block_until_presented=*/true);
        }

        // Frame-rate cap — PaintFromUIThread may return early if Vulkan swapchain
        // isn't waiting on vsync, so enforce a ceiling here.  Target 60 FPS to keep
        // CPU/GPU sane on any device while still feeling smooth.
        if (g_target_fps > 0) {
            uint64_t kMinFrameNs = UINT64_C(1'000'000'000) / g_target_fps;
            auto frame_end = std::chrono::high_resolution_clock::now();
            auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(frame_end - now).count();
            int64_t remaining_ns = static_cast<int64_t>(kMinFrameNs) - static_cast<int64_t>(elapsed_ns);
            if (remaining_ns > 0) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(remaining_ns));
            }
        }
    }

    LOGI("Game thread exiting");
    mcla::GetMenus().Shutdown();

    if (g_runtime) {
        g_runtime->Shutdown();
        delete g_runtime;
        g_runtime = nullptr;
    }
    g_presenter = nullptr;
    g_provider = nullptr;
    g_window.reset();
    g_immediate_drawer.reset();
    g_app_context.reset();

    SDL_Quit();
    LOGI("Game thread finished");
    if (g_jvm && g_jni_env) g_jvm->DetachCurrentThread();
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeInit(JNIEnv* env, jobject obj,
                                               jstring assetsDir, jstring filesDir, jstring cacheDir) {
    const char* ad = env->GetStringUTFChars(assetsDir, nullptr);
    const char* fd = env->GetStringUTFChars(filesDir, nullptr);
    const char* cd = env->GetStringUTFChars(cacheDir, nullptr);
    std::string assets_dir(ad ? ad : "");
    std::string files_dir(fd ? fd : "");
    std::string cache_dir(cd ? cd : "");
    if (ad) env->ReleaseStringUTFChars(assetsDir, ad);
    if (fd) env->ReleaseStringUTFChars(filesDir, fd);
    if (cd) env->ReleaseStringUTFChars(cacheDir, cd);

    LOGI("nativeInit: assets=%s, files=%s, cache=%s",
         assets_dir.c_str(), files_dir.c_str(), cache_dir.c_str());

    install_crash_handler(SIGSEGV);
    install_crash_handler(SIGABRT);
    install_crash_handler(SIGBUS);

    env->GetJavaVM(&g_jvm);
    g_game_thread = new std::thread(gameThreadMain, assets_dir, files_dir, cache_dir);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeRunGame(JNIEnv* env, jobject obj) {
    LOGI("nativeRunGame called");
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeShutdown(JNIEnv* env, jobject obj) {
    LOGI("nativeShutdown called");
    g_running = false;
    if (g_app_context) {
        g_app_context->PlatformQuitFromUIThread();
    }
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
    g_surface_width.store(width);
    g_surface_height.store(height);
    g_surface_ready.store(true);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnSurfaceChanged(JNIEnv* env, jobject obj,
                                                           jint width, jint height) {
    LOGI("nativeOnSurfaceChanged: %dx%d", width, height);
    g_surface_width.store(width);
    g_surface_height.store(height);
    // Keep the menu's display size + touch mapping in sync with the surface.
    mcla::GetMenus().SetSize(width, height);
    // Also sync the virtual gamepad hit-test coordinate space so stick/button
    // regions match what's drawn on screen.
    mcla::GetGamepadManager().SetDisplaySize(width, height);
    if (g_presenter) {
        g_presenter->RequestUIPaintFromUIThread();
    }
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnTouchEvent(JNIEnv* env, jobject obj,
                                                        jint action, jfloat x, jfloat y, jint pointerId) {
    bool is_down = (action == 0); // AMOTION_EVENT_ACTION_DOWN = 0
    static int touch_log = 0;
    if (touch_log < 12) { ++touch_log;
        LOGI("nativeOnTouchEvent action=%d x=%.1f y=%.1f pid=%d", (int)action, (double)x, (double)y, (int)pointerId);
    }

    // A rightward swipe from the left edge opens the in-game pad settings.
    // Those touches (and every touch while the drawer is open) belong to the
    // ImGui panel, not to the virtual controller.
    const bool swipe_consumed = mcla::GetMenus().HandleEdgeSwipe((int)action, x, y);
    if (!swipe_consumed && !mcla::GetMenus().IsPadSettingsOpen()) {
        mcla::GetGamepadManager().FeedTouch((int)pointerId, is_down, x, y);
    }

    // Map surface-pixel coords into the Vulkan render-target's local space
    // so that ImGui hit-testing aligns with what the immediate drawer presents.
    ImVec2 mapped = mcla::GetMenus().MapTouchEventCoords(x, y);
    float mx = mapped.x;
    float my = mapped.y;

    // Send MOUSE_MOTION first so ImGui cursor is at the right position before
    // processing the button down/up events.
    SDL_Event motion;
    std::memset(&motion, 0, sizeof(motion));
    motion.type       = SDL_EVENT_MOUSE_MOTION;
    motion.motion.timestamp = (Uint64)SDL_GetTicks() * 1000000ULL;
    motion.motion.windowID  = 0;
    motion.motion.which     = (SDL_MouseID)pointerId;
    motion.motion.state     = is_down ? SDL_BUTTON_LEFT : 0;
    motion.motion.x         = mx;
    motion.motion.y         = my;
    motion.motion.xrel      = 0;
    motion.motion.yrel      = 0;
    SDL_PushEvent(&motion);

    // Then the button event.
    SDL_Event btn;
    std::memset(&btn, 0, sizeof(btn));
    btn.type          = is_down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    btn.button.timestamp = (Uint64)SDL_GetTicks() * 1000000ULL;
    btn.button.windowID  = 0;
    btn.button.which     = (SDL_MouseID)pointerId;
    btn.button.button    = (Uint8)SDL_BUTTON_LEFT;
    btn.button.down      = (bool)is_down;
    btn.button.clicks    = 1;
    btn.button.padding   = 0;
    btn.button.x         = mx;
    btn.button.y         = my;
    SDL_PushEvent(&btn);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeOnKeyEvent(JNIEnv* env, jobject obj,
                                                      jint keyCode, jint action) {
    SDL_Keycode sdl_key = SDLK_UNKNOWN;
    switch (keyCode) {
        case 19: sdl_key = SDLK_UP;        break;
        case 20: sdl_key = SDLK_DOWN;      break;
        case 21: sdl_key = SDLK_LEFT;      break;
        case 22: sdl_key = SDLK_RIGHT;     break;
        case 61: sdl_key = SDLK_RETURN;    break;
        case 66: sdl_key = SDLK_RETURN;    break;
        case 67: sdl_key = SDLK_ESCAPE;    break;
        case 219: sdl_key = SDLK_LSHIFT;   break;
        case 220: sdl_key = SDLK_RSHIFT;   break;
        default:
            // Unmapped key — ignore silently to avoid per-key log spam.
            return;
    }
    SDL_Event event;
    event.type = (action == 0) ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.key = sdl_key;
    switch (sdl_key) {
        case SDLK_UP:        event.key.scancode = SDL_SCANCODE_UP; break;
        case SDLK_DOWN:      event.key.scancode = SDL_SCANCODE_DOWN; break;
        case SDLK_LEFT:      event.key.scancode = SDL_SCANCODE_LEFT; break;
        case SDLK_RIGHT:     event.key.scancode = SDL_SCANCODE_RIGHT; break;
        case SDLK_RETURN:    event.key.scancode = SDL_SCANCODE_RETURN; break;
        case SDLK_ESCAPE:    event.key.scancode = SDL_SCANCODE_ESCAPE; break;
        case SDLK_LSHIFT:    event.key.scancode = SDL_SCANCODE_LSHIFT; break;
        case SDLK_RSHIFT:    event.key.scancode = SDL_SCANCODE_RSHIFT; break;
        default:             event.key.scancode = SDL_SCANCODE_UNKNOWN; break;
    }
    SDL_PushEvent(&event);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSetAssetManager(JNIEnv* env, jobject obj, jobject assetManager) {
    g_asset_manager = AAssetManager_fromJava(env, assetManager);
    if (g_asset_manager) {
        LOGI("Asset manager set successfully");
    } else {
        LOGE("Failed to get asset manager from Java");
    }
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSetNativeLibraryDir(JNIEnv* env, jobject obj, jstring nativeLibDir) {
    const char* d = env->GetStringUTFChars(nativeLibDir, nullptr);
    g_native_library_dir = d ? std::string(d) : "";
    if (d) env->ReleaseStringUTFChars(nativeLibDir, d);
    LOGI("Native library dir set: %s", g_native_library_dir.c_str());
}

JNIEXPORT jint JNICALL
Java_com_mcla_recomp_MainActivity_nativeGetFPS(JNIEnv* env, jobject obj) {
    (void)env; (void)obj;
    return static_cast<jint>(g_fps);
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSaveConfig(JNIEnv* env, jobject obj) {
    (void)env; (void)obj;
    mcla::GetConfig().Save();
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeLoadConfig(JNIEnv* env, jobject obj) {
    (void)env; (void)obj;
    mcla::GetConfig().Load();
}

JNIEXPORT void JNICALL
Java_com_mcla_recomp_MainActivity_nativeSetFPSVisible(JNIEnv* env, jobject obj, jboolean visible) {
    (void)env; (void)obj;
    mcla::GetConfig().show_fps = (visible == JNI_TRUE);
}

JNIEXPORT jboolean JNICALL
Java_com_mcla_recomp_MainActivity_nativeIsGameRunning(JNIEnv* env, jobject obj) {
    (void)env; (void)obj;
    return g_running ? JNI_TRUE : JNI_FALSE;
}

} // extern "C"
