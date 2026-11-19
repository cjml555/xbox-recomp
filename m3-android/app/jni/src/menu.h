// MCLA - Menu System
// Dear ImGui-based menus for startup, settings, and HUD

#pragma once

#include <SDL3/SDL.h>
#include <optional>
#include <string>
#include <imgui.h>

namespace mcla {

enum class MenuState {
    MainMenu,
    LoadGame,
    GraphicsSettings,
    LanguageSettings,
    CheatsMenu,
    ModsMenu,
    VulkanCatalog,
    RendererSelector,
    GraphicsQuality,
    Credits,
    CrashLogs,
    ShaderGenerator,
    LANMenu,
    DevelopmentMode,
    Hidden,
    GameRunning  // Only HUD visible
};

struct MenuItem {
    std::string label;
    MenuState target_state;
    bool visible = true;
};

class MenuSystem {
public:
    MenuSystem();
    ~MenuSystem() = default;
    
    void Initialize(SDL_Window* window);
    void Shutdown();

    void SetState(MenuState state);
    MenuState GetState() const { return m_current_state; }

    void Render();

    // Frame lifecycle - call BeginFrame before Render, EndFrame after
    void BeginFrame();
    void EndFrame();

    void HandleEvent(const SDL_Event* event);

    bool IsGameRunning() const { return m_game_running; }
    void SetGameRunning(bool running) { m_game_running = running; }

    bool IsInitialized() const { return m_initialized; }

    // Menu navigation
    void NavigateBack();
    void GoToMain();

    // FPS / HUD
    void SetFPS(float fps) { m_fps = fps; }
    float GetFPS() const { return m_fps; }
    void RenderFPSHUD(float fps);

    // Exit request (from "SALIR" button)
    bool ShouldExit() const { return m_should_exit; }
    void SetShouldExit(bool exit) { m_should_exit = exit; }

    // Game start request (from "JUGAR" button)
    bool GameStartRequested() const { return m_game_start_requested; }
    void ConsumeGameStartRequest() { m_game_start_requested = false; }

    // Virtual gamepad control (touch)
    void SetVirtualGamepadVisible(bool visible);
    bool IsVirtualGamepadVisible() const { return m_virtual_gamepad_visible; }

    // Display size (updated by Android surface callbacks)
    void SetSize(int width, int height) { m_display_width = width; m_display_height = height; }
    int GetWidth() const { return m_display_width; }
    int GetHeight() const { return m_display_height; }

private:
    MenuState m_current_state = MenuState::MainMenu;
    MenuState m_previous_state = MenuState::MainMenu;
    bool m_initialized = false;
    bool m_game_running = false;
    bool m_using_sdl_window = false;  // true if initialized with SDL window, false for GLSurfaceView mode
    SDL_Window* m_window = nullptr;

    float m_fps = 0.0f;
    bool m_should_exit = false;
    bool m_game_start_requested = false;
    bool m_virtual_gamepad_visible = false;
    int m_display_width = 0;
    int m_display_height = 0;

    void SwitchToState(MenuState state) {
        m_previous_state = m_current_state;
        m_current_state = state;
    }

    void RenderMainMenu();
    void RenderLoadGameMenu();
    void RenderGraphicsMenu();
    void RenderLanguageMenu();
    void RenderCheatsMenu();
    void RenderModsMenu();
    void RenderVulkanCatalogMenu();
    void RenderRendererSelectorMenu();
    void RenderGraphicsQualityMenu();
    void RenderCreditsMenu();
    void RenderCrashLogsMenu();
    void RenderShaderGeneratorMenu();
    void RenderLANMenu();
    void RenderDevelopmentModeMenu();
    void RenderHUD(float fps);
    
    // Helper functions for virtual gamepad
    void RenderVirtualGamepad();
    void RenderGameFileSelector();
    void RenderSaveLoadMenu();

    // Helper functions
    void DrawMenuItem(const std::string& label, bool* selected, const MenuItem& item);
    void DrawButton(const std::string& label, float width = 0);
};

// Global menu instance
MenuSystem& GetMenus();

} // namespace mcla