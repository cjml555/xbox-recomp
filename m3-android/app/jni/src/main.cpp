// MCLA - Midnight Club: Los Angeles Android Main Entry Point
// Generated for M3 milestone
// 
// This file initializes the ReXGlue runtime and launches the recompiled
// game code via xstart() at address 0x821322B8.

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include <rex/rex_app.h>
#include <rex/system.h>
#include <rex/system/function_dispatcher.h>
#include <rex/system/thread_state.h>

#include "larecomp_init.h"
#include "game_hooks.h"
#include "config.h"
#include "menu.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/stat.h>

// Entry point address (defined in larecomp_register.cpp)
constexpr uint32_t kXstartAddress = 0x821322B8;

// App state for managing the game lifecycle
static std::atomic<bool> g_running{false};
static rex::Runtime* g_runtime = nullptr;
static rex::runtime::ThreadState* g_thread_state = nullptr;
static float g_fps = 0.0f;

// FPS counter
static int g_frame_count = 0;
static std::chrono::high_resolution_clock::time_point g_last_fps_time;

// Override roundeven for ARM64 compatibility
extern "C" float roundevenf(float x) {
    return std::nearbyintf(x);
}

extern "C" double roundeven(double x) {
    return std::nearbyint(x);
}

int main(int argc, char* argv[]) {
    SDL_Log("MCLA Android starting...");
    
    // Get the executable folder as the base directory
    auto exe_dir = rex::filesystem::GetExecutableFolder();
    SDL_Log("Executable folder: %s", exe_dir.string().c_str());
    
    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Log("SDL3 initialized successfully");
    
    // Create window with SDL3
    // SDL3 CreateWindow signature: (title, w, h, flags)
    SDL_Window* window = SDL_CreateWindow(
        "Midnight Club: LA",
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Log("SDL3 window created successfully");
    
    // Initialize menu system
    mcla::GetMenus().Initialize(window);
    
    // Start FPS counter
    g_last_fps_time = std::chrono::high_resolution_clock::now();
    
    // Main entry - show menu first
    mcla::GetMenus().SetState(mcla::MenuState::MainMenu);
    
    // Event loop - wait for user to start the game
    while (!mcla::GetMenus().IsGameRunning()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            mcla::GetMenus().HandleEvent(&event);
            
            if (event.type == SDL_EVENT_QUIT) {
                mcla::GetMenus().Shutdown();
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
        }
        
        // Render menu
        mcla::GetMenus().Render();
        SDL_GL_SwapWindow(window);
        
        SDL_Delay(16);
    }
    
    SDL_Log("User selected to start game, initializing runtime...");
    
    // ===========================================
    // Runtime Initialization
    // ===========================================
    
    // Configure runtime
    rex::RuntimeConfig config;
    config.tool_mode = false;  // We want full runtime
    
    // On Android, game data may be in assets/ directory
    std::string xex_path;
    
#ifdef __ANDROID__
    // Try to find XEX in standard locations
    // First check if there's a game data directory relative to executable
    std::filesystem::path xex_path_try = exe_dir / "assets" / "default.xex";
    if (std::filesystem::exists(xex_path_try)) {
        xex_path = xex_path_try.string();
    } else {
        xex_path_try = exe_dir / "default.xex";
        if (std::filesystem::exists(xex_path_try)) {
            xex_path = xex_path_try.string();
        }
    }
#else
    // On desktop, look for XEX in standard locations
    std::filesystem::path xex_path_try = exe_dir / "assets" / "default.xex";
    if (std::filesystem::exists(xex_path_try)) {
        xex_path = xex_path_try.string();
    } else {
        xex_path_try = exe_dir / "default.xex";
        if (std::filesystem::exists(xex_path_try)) {
            xex_path = xex_path_try.string();
        }
    }
#endif
    
    if (xex_path.empty() || !std::filesystem::exists(xex_path)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XEX file not found at expected location");
        SDL_Log("Current working directory check...");
        
        // Try relative paths
        const char* cwd = std::filesystem::current_path().c_str();
        SDL_Log("CWD: %s", cwd);
        
        // Check for XEX in various locations
        std::vector<std::string> search_paths = {
            "assets/default.xex",
            "default.xex",
            "/home/jhon/xbox-recomp/assets/default.xex",
            "./assets/default.xex"
        };
        
        for (const auto& path : search_paths) {
            if (std::filesystem::exists(path)) {
                xex_path = path;
                SDL_Log("Found XEX at: %s", xex_path.c_str());
                break;
            }
        }
    }
    
    if (xex_path.empty() || !std::filesystem::exists(xex_path)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XEX file not found - will use tool mode for testing");
    } else {
        SDL_Log("XEX file: %s", xex_path.c_str());
    }
    
    // Determine game data root
    std::filesystem::path xex_dir;
    if (!xex_path.empty()) {
        xex_dir = std::filesystem::path(xex_path).parent_path().parent_path();
    } else {
        xex_dir = std::filesystem::path("/home/jhon/xbox-recomp");
    }
    
    // Create runtime instance
    g_runtime = new rex::Runtime(
        xex_dir,           // game_data_root (contains XEX and assets)
        xex_dir,           // user_data_root
        {},                // update_data_root
        xex_dir / "cache", // cache_root
        xex_dir / "metadata" // metadata_root
    );
    
    // Set the display window for graphics presentation (pass nullptr for tool/headless mode)
    g_runtime->set_display_window(nullptr);
    
    // Setup the runtime with PPCImageConfig and config
    // This initializes:
    // 1. Memory subsystem
    // 2. Function dispatcher with function table
    // 3. Registers all recompiled functions (including xstart at 0x821322B8)
    auto setup_status = g_runtime->Setup(PPCImageConfig, std::move(config));
    if (XFAILED(setup_status)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Runtime::Setup failed: 0x%08X", setup_status);
        delete g_runtime;
        mcla::GetMenus().Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Log("Runtime initialized successfully");
    
    // Get the function dispatcher for calling xstart
    auto* function_dispatcher = g_runtime->function_dispatcher();
    if (!function_dispatcher) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Function dispatcher not available");
        g_runtime->Shutdown();
        delete g_runtime;
        mcla::GetMenus().Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Get memory from runtime for thread state
    auto* memory = g_runtime->memory();
    if (!memory) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Memory not initialized");
        g_runtime->Shutdown();
        delete g_runtime;
        mcla::GetMenus().Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Create thread state with:
    // - thread_id: 0 (main thread)
    // - stack_base: 0 (placeholder, not used in current implementation)
    // - pcr_address: 0 (not used on host)
    // - memory: pointer to the runtime's memory
    g_thread_state = new rex::runtime::ThreadState(
        0,  // thread_id
        0,  // stack_base (placeholder)
        0,  // pcr_address
        memory
    );
    
    // Bind this thread state to the current thread
    rex::runtime::ThreadState::Bind(g_thread_state);
    SDL_Log("Thread state created and bound");
    
    // ===========================================
    // Call xstart - the game entry point
    // ===========================================
    
    // Call xstart via the function dispatcher
    // This is the main game entry point at address 0x821322B8
    SDL_Log("Calling xstart at address 0x%08X...", kXstartAddress);
    
    // Execute xstart - prepares guest state
    function_dispatcher->Execute(g_thread_state, kXstartAddress, nullptr, 0);
    SDL_Log("xstart preparation complete");
    
    // Signal that game is running
    mcla::GetMenus().SetGameRunning(true);
    
    // ===========================================
    // Game Loop
    // ===========================================
    
    SDL_Log("Entering main game loop...");
    g_running = true;
    g_frame_count = 0;
    
    while (g_running) {
        // Handle events
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
        
        // Calculate FPS
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_last_fps_time);
        if (elapsed.count() >= 1000) {
            g_fps = static_cast<float>(g_frame_count * 1000) / elapsed.count();
            g_last_fps_time = now;
            g_frame_count = 0;
        }
        g_frame_count++;
        
        // Render HUD with ImGui
        mcla::GetMenus().Render();
        SDL_GL_SwapWindow(window);
        
        // Yield to allow other threads
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // ===========================================
    // Cleanup
    // ===========================================
    
    SDL_Log("MCLA exiting normally");
    
    g_running = false;
    
    if (g_thread_state) {
        delete g_thread_state;
        g_thread_state = nullptr;
    }
    
    if (g_runtime) {
        g_runtime->Shutdown();
        delete g_runtime;
        g_runtime = nullptr;
    }
    
    mcla::GetMenus().Shutdown();
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}