// MCLA - Menu System
// Dear ImGui-based menus for startup, settings, and HUD.
// Renders via the ReXGlue ImmediateDrawer (Vulkan) instead of OpenGL ES.

#pragma once

#include <SDL3/SDL.h>
#include <optional>
#include <string>
#include <atomic>
#include <memory>
#include <imgui.h>

// ReXGlue UI headers for UIDrawer derivation and texture storage.
#include <rex/ui/ui_drawer.h>
#include <rex/ui/immediate_drawer.h>
namespace rex { namespace ui {
    class UIDrawContext;
}}

// Forward declare AAssetManager for texture loading
struct AAssetManager;

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

class MenuSystem final : public rex::ui::UIDrawer {
public:
    MenuSystem();
    ~MenuSystem();

    void Initialize(rex::ui::ImmediateDrawer* drawer);
    void Shutdown();

    // UIDrawer interface: called by Presenter on each paint pass.
    void Draw(rex::ui::UIDrawContext&) override;

    void SetState(MenuState state);
    MenuState GetState() const { return m_current_state; }

    void HandleEvent(const SDL_Event* event);

    bool IsGameRunning() const { return m_game_running.load(); }
    void SetGameRunning(bool running) { m_game_running.store(running); }

    bool IsInitialized() const { return m_initialized; }

    // Menu navigation
    void NavigateBack();
    void GoToMain();

    // FPS / HUD
    void SetFPS(float fps) { m_fps = fps; }
    float GetFPS() const { return m_fps; }
    void AddFrametimeSample(float ms) { m_ft[m_ft_idx] = ms; m_ft_idx = (m_ft_idx + 1) & (kFtBufSize - 1); }
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
    void SetSize(int width, int height);
    int GetWidth() const { return m_display_width; }
    int GetHeight() const { return m_display_height; }

    // Touch coordinate mapping (converts screen coords to render-target coords)
    void SetTouchOffset(float ox, float oy) { m_touch_offset_x = ox; m_touch_offset_y = oy; }
    ImVec2 MapTouchEventCoords(float screen_x, float screen_y) const;

    // Texture loading from assets
    void LoadTextures(AAssetManager* assetManager);

private:
    MenuState m_current_state = MenuState::MainMenu;
    MenuState m_previous_state = MenuState::MainMenu;
    bool m_initialized = false;
    std::atomic<bool> m_game_running{false};

    float m_fps = 0.0f;
    // Rolling frametime ring buffer for the HUD graph (MangoHUD-style).
    static constexpr int kFtBufSize = 64;
    float m_ft[kFtBufSize] = {0};
    int   m_ft_idx = 0;
    bool m_should_exit = false;
    bool m_game_start_requested = false;
    bool m_virtual_gamepad_visible = false;
    bool m_mouse_down = false;       // Set on DOWN, cleared on pending UP injection
    int m_up_defer_counter = 0;      // Defer-inject the mouse UP after a frame boundary
    int m_display_width = 0;
    int m_display_height = 0;
    // Actual UI draw-target size reported by the presenter's UIDrawContext
    // (the Vulkan swapchain extent). Captured in Draw(); drives touch mapping.
    int m_rt_width = 0;
    int m_rt_height = 0;
    float m_touch_scale = 1.0f;      // Surface -> render-target scale
    float m_touch_offset_x = 0.0f;   // Letterbox X offset in surface pixels
    float m_touch_offset_y = 0.0f;   // Letterbox Y offset in surface pixels
    void UpdateTouchMapping();

    // ImmediateDrawer owned reference — used only as the renderer target.
    rex::ui::ImmediateDrawer* m_immediate_drawer = nullptr;

    // Owned textures — storing unique_ptr lets us set ImTextureID
    // to the raw ImmediateTexture* pointer.
    struct TextureEntry {
        std::unique_ptr<rex::ui::ImmediateTexture> tex;
        ImTextureID id = 0;
    };
    std::vector<TextureEntry> m_textures;

    // Owned font atlas texture (kept alive for the lifetime of the menu).
    std::unique_ptr<rex::ui::ImmediateTexture> m_font_texture;

    // Menu texture handles (ImTextureID = raw ImmediateTexture*).
    ImTextureID m_backdrop_texture = 0;
    ImTextureID m_icon_jugar = 0;
    ImTextureID m_icon_config = 0;
    ImTextureID m_icon_controles = 0;
    ImTextureID m_icon_desarrollo = 0;
    ImTextureID m_icon_creditos = 0;
    ImTextureID m_icon_actualizar = 0;
    ImTextureID m_icon_salir = 0;

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
    void RenderHudControls();
    void RenderHUD(float fps);

    void RenderVirtualGamepad();
    void RenderGameFileSelector();
    void RenderSaveLoadMenu();

    void DrawMenuItem(const std::string& label, bool* selected, const MenuItem& item);
    void DrawButton(const std::string& label, float width = 0);
};

// Global menu instance
MenuSystem& GetMenus();

} // namespace mcla
