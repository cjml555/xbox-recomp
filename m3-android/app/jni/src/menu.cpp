// MCLA - Menu System Implementation
#include "menu.h"
#include "config.h"
#include "gamepad_manager.h"
#include "vulkan_catalog.h"
#include "shader_generator.h"
#include "crash_logger.h"
#include "updater.h"
#include <android/log.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <SDL3/SDL_scancode.h>  // For SDL_Scancode enum in key conversion
#include <SDL3/SDL_events.h>   // For SDL_Event and event type constants
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MCLA_Menu", __VA_ARGS__)

namespace mcla {

MenuSystem::MenuSystem() = default;

void MenuSystem::Initialize(SDL_Window* window) {
    if (m_initialized) return;

    m_window = window;
    m_using_sdl_window = (window != nullptr);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Docking and gamepad nav are handled via GLSurfaceView mode
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 7.0f;
    style.WindowPadding = ImVec2(12, 12);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    // style.StatSpacing = ImVec2(8, 8); // Removed in newer ImGui - use WindowPadding or ItemSpacing
    style.ScrollbarRounding = 6.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
    style.GrabRounding = 4.0f;

    // Setup Platform/Renderer bindings
    // Note: window may be nullptr when using GLSurfaceView - the SDL platform
    // binding is skipped and we use raw OpenGL3 directly
    if (m_using_sdl_window) {
        ImGui_ImplSDL3_InitForOpenGL(window, nullptr);
    }
    ImGui_ImplOpenGL3_Init("#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "uniform mat4 u_mvp;\n"
        "uniform sampler2D tex;\n"
        "in vec2 v_tex;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = texture(tex, v_tex);\n"
        "}");

    // Load configuration
    GetConfig().Load();

    m_initialized = true;
    LOGI("Menu system initialized (SDL window: %s)", m_using_sdl_window ? "yes" : "no");
}

void MenuSystem::Shutdown() {
    if (!m_initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    if (m_using_sdl_window) {
        ImGui_ImplSDL3_Shutdown();
    }
    ImGui::DestroyContext();

    m_initialized = false;
}

void MenuSystem::SetState(MenuState state) {
    SwitchToState(state);
}

void MenuSystem::NavigateBack() {
    if (m_current_state == MenuState::MainMenu) {
        m_current_state = MenuState::Hidden;
    } else {
        SwitchToState(m_previous_state);
    }
}

void MenuSystem::GoToMain() {
    SwitchToState(MenuState::MainMenu);
}

void MenuSystem::BeginFrame() {
    if (!m_initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    if (m_using_sdl_window) {
        ImGui_ImplSDL3_NewFrame();
    }
    ImGui::NewFrame();
}

void MenuSystem::Render() {
    if (!m_initialized) return;

    if (m_game_running) {
        RenderHUD(GetConfig().show_fps ? m_fps : 0.0f);
        RenderVirtualGamepad();
    } else {
        // Render current menu
        switch (m_current_state) {
            case MenuState::MainMenu: RenderMainMenu(); break;
            case MenuState::LoadGame: RenderLoadGameMenu(); break;
            case MenuState::GraphicsSettings: RenderGraphicsMenu(); break;
            case MenuState::LanguageSettings: RenderLanguageMenu(); break;
            case MenuState::CheatsMenu: RenderCheatsMenu(); break;
            case MenuState::ModsMenu: RenderModsMenu(); break;
            case MenuState::VulkanCatalog: RenderVulkanCatalogMenu(); break;
            case MenuState::RendererSelector: RenderRendererSelectorMenu(); break;
            case MenuState::GraphicsQuality: RenderGraphicsQualityMenu(); break;
            case MenuState::Credits: RenderCreditsMenu(); break;
            case MenuState::CrashLogs: RenderCrashLogsMenu(); break;
            case MenuState::ShaderGenerator: RenderShaderGeneratorMenu(); break;
            case MenuState::LANMenu: RenderLANMenu(); break;
            case MenuState::DevelopmentMode: RenderDevelopmentModeMenu(); break;
            default: break;
        }
    }
}

void MenuSystem::EndFrame() {
    if (!m_initialized) return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData());
}

void MenuSystem::HandleEvent(const SDL_Event* event) {
    if (!m_initialized) return;

    // When using SDL window, let the SDL backend process events
    if (m_using_sdl_window) {
        if (ImGui_ImplSDL3_ProcessEvent(event)) {
            return;
        }
    } else {
        // When using GLSurfaceView, process events directly into ImGui IO
        ImGuiIO& io = ImGui::GetIO();

        switch (event->type)
        {
            case SDL_EVENT_MOUSE_MOTION:
                io.AddMousePosEvent((float)event->motion.x, (float)event->motion.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                io.AddMouseButtonEvent(event->button.button, event->button.down);
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL:
                io.AddMouseWheelEvent((float)event->wheel.y, 0.0f);
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            {
                // Convert SDL keycode to ImGuiKey for GLSurfaceView mode
                ImGuiKey key = ImGuiKey_None;
                SDL_Scancode scancode = event->key.scancode;
                
                // Simple key conversion - use default mappings
                switch (scancode) {
                    case SDL_SCANCODE_KP_0: key = ImGuiKey_Keypad0; break;
                    case SDL_SCANCODE_KP_1: key = ImGuiKey_Keypad1; break;
                    case SDL_SCANCODE_KP_2: key = ImGuiKey_Keypad2; break;
                    case SDL_SCANCODE_KP_3: key = ImGuiKey_Keypad3; break;
                    case SDL_SCANCODE_KP_4: key = ImGuiKey_Keypad4; break;
                    case SDL_SCANCODE_KP_5: key = ImGuiKey_Keypad5; break;
                    case SDL_SCANCODE_KP_6: key = ImGuiKey_Keypad6; break;
                    case SDL_SCANCODE_KP_7: key = ImGuiKey_Keypad7; break;
                    case SDL_SCANCODE_KP_8: key = ImGuiKey_Keypad8; break;
                    case SDL_SCANCODE_KP_9: key = ImGuiKey_Keypad9; break;
                    case SDL_SCANCODE_SPACE: key = ImGuiKey_Space; break;
                    case SDL_SCANCODE_RETURN: key = ImGuiKey_Enter; break;
                    case SDL_SCANCODE_TAB: key = ImGuiKey_Tab; break;
                    case SDL_SCANCODE_BACKSPACE: key = ImGuiKey_Backspace; break;
                    case SDL_SCANCODE_ESCAPE: key = ImGuiKey_Escape; break;
                    case SDL_SCANCODE_UP: key = ImGuiKey_UpArrow; break;
                    case SDL_SCANCODE_DOWN: key = ImGuiKey_DownArrow; break;
                    case SDL_SCANCODE_LEFT: key = ImGuiKey_LeftArrow; break;
                    case SDL_SCANCODE_RIGHT: key = ImGuiKey_RightArrow; break;
                    default:
                        // Fallback: convert alphanumeric keys
                        if (event->key.key >= SDLK_A && event->key.key <= SDLK_Z) {
                            key = (ImGuiKey)(ImGuiKey_A + (event->key.key - SDLK_A));
                        } else if (event->key.key >= SDLK_0 && event->key.key <= SDLK_9) {
                            key = (ImGuiKey)(ImGuiKey_0 + (event->key.key - SDLK_0));
                        } else {
                            key = (ImGuiKey)event->key.key;
                        }
                        break;
                }
                
                if (key != ImGuiKey_None) {
                    io.AddKeyEvent(key, event->type == SDL_EVENT_KEY_DOWN);
                    io.SetKeyEventNativeData(key, (int)event->key.key, (int)scancode, (int)scancode);
                }
                break;
            }
            default:
                break;
        }
    }
}

// ============================================
// Menu Rendering Functions
// ============================================

void MenuSystem::RenderMainMenu() {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus;

    if (ImGui::Begin("MCLA Launcher", nullptr, window_flags)) {
        // Game logo
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "MIDNIGHT CLUB: LOS ANGELES");
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // Main buttons
        if (ImGui::Button("▶  JUGAR", ImVec2(200, 50))) {
            m_game_running = true;
            SwitchToState(MenuState::GameRunning);
        }
        ImGui::Spacing();

        if (ImGui::Button("≡  CONFIGURACIÓN", ImVec2(200, 50))) {
            SwitchToState(MenuState::GraphicsSettings);
        }
        ImGui::Spacing();

        if (ImGui::Button("🎮  CONTROLES", ImVec2(200, 50))) {
            SwitchToState(MenuState::LanguageSettings);
        }
        ImGui::Spacing();

        if (ImGui::Button("🔧  DESARROLLO", ImVec2(200, 50))) {
            SwitchToState(MenuState::DevelopmentMode);
        }
        ImGui::Spacing();

        if (ImGui::Button(" ℹ️  CRÉDITOS", ImVec2(200, 50))) {
            SwitchToState(MenuState::Credits);
        }
        ImGui::Spacing();

        if (ImGui::Button("🔄  ACTUALIZAR", ImVec2(200, 50))) {
            // TODO: Update logic
        }
        ImGui::Spacing();

        if (ImGui::Button("❌  SALIR", ImVec2(200, 50))) {
            exit(0);
        }
    }
    ImGui::End();
}

void MenuSystem::RenderGameFileSelector() {
    if (ImGui::Begin("Selectar Archivo de Juego")) {
        ImGui::Text("Ruta del juego: %s", GetConfig().game_data_path.c_str());
        
        static char file_path[256] = "";
        if (ImGui::InputText("Ruta del archivo", file_path, sizeof(file_path))) {
            GetConfig().game_data_path = std::string(file_path);
        }
        
        ImGui::Spacing();
        
        if (ImGui::Button("Cargar")) {
            // File dialog would go here
        }
        ImGui::SameLine();
        if (ImGui::Button("Explorar")) {
            // File browser would go here
        }

        ImGui::Spacing();
        if (ImGui::Button("Cargar Partida Guardada")) {
            SwitchToState(MenuState::LoadGame);
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderLoadGameMenu() {
    if (ImGui::Begin("Cargar Juego")) {
        ImGui::Text("Ruta del juego: %s", GetConfig().game_data_path.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Buscar")) {
            // File dialog
        }

        ImGui::Spacing();
        if (ImGui::Button("Cargar Partida Guardada")) {
            // Load save
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderGraphicsMenu() {
    if (ImGui::Begin("Gráficos")) {
        // Quality presets
        static int quality_idx = static_cast<int>(GetConfig().quality);
        const char* quality_labels[] = {"Bajo", "Medio", "Alto", "Ultra", "Personalizado"};
        if (ImGui::Combo("Calidad", &quality_idx, quality_labels, 5)) {
            GetConfig().quality = static_cast<GraphicsQuality>(quality_idx);
            switch (GetConfig().quality) {
                case GraphicsQuality::Low:
                    GetConfig().msaa_enabled = false;
                    GetConfig().motion_blur = false;
                    GetConfig().imposter_shadows = false;
                    break;
                case GraphicsQuality::High:
                    GetConfig().msaa_enabled = true;
                    GetConfig().msaa_samples = 4;
                    break;
                case GraphicsQuality::Ultra:
                    GetConfig().msaa_enabled = true;
                    GetConfig().msaa_samples = 8;
                    break;
                default: break;
            }
        }

        ImGui::Checkbox("VSync", &GetConfig().vsync);
        ImGui::Checkbox("Motion Blur", &GetConfig().motion_blur);
        ImGui::Checkbox("Impostor Shadows", &GetConfig().imposter_shadows);
        ImGui::Checkbox("Rubber Banding", &GetConfig().rubber_banding);

        ImGui::SliderFloat("FOV Scale", &GetConfig().fov_scale, 0.5f, 2.0f);
        ImGui::SliderFloat("Gamma", &GetConfig().gamma, 0.5f, 2.0f);

        ImGui::Spacing();
        if (ImGui::Button("Selector de Renderizador")) {
            SwitchToState(MenuState::RendererSelector);
        }

        ImGui::Spacing();
        if (ImGui::Button("Catálogo Vulkan")) {
            SwitchToState(MenuState::VulkanCatalog);
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderLanguageMenu() {
    static int selected_lang = static_cast<int>(GetConfig().language);
    const char* languages[] = {"Español", "English", "Francés", "Deutsch", "日本語", "中文"};

    if (ImGui::Begin("Idioma")) {
        if (ImGui::Combo("Idioma", &selected_lang, languages, 6)) {
            GetConfig().language = static_cast<Language>(selected_lang);
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderCheatsMenu() {
    static bool infinite_health = false;
    static bool infinite_ammunition = false;
    static bool no_ads = false;

    if (ImGui::Begin("Trucos")) {
        ImGui::Checkbox("Salud Ilimitable", &infinite_health);
        ImGui::Checkbox("Armas Ilimitadas", &infinite_ammunition);
        ImGui::Checkbox("Sin Publicidad", &no_ads);
        ImGui::Checkbox("Activar Trucos", &GetConfig().cheats_enabled);
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderModsMenu() {
    if (ImGui::Begin("Mods")) {
        if (ImGui::Button("Instalar Mod")) {
            // File dialog
        }

        ImGui::Checkbox("Mods Activados", &GetConfig().mods_enabled);

        if (ImGui::BeginListBox("Mods Activos", ImVec2(300, 200))) {
            for (const auto& mod : GetConfig().active_mods) {
                ImGui::Selectable(mod.c_str());
            }
            ImGui::EndListBox();
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderVulkanCatalogMenu() {
    const auto& drivers = GetVulkanCatalog().GetDrivers();
    static int selected_driver = 0;

    if (ImGui::Begin("Catálogo Vulkan Drivers")) {
        ImGui::Text("Drivers Vulkan disponibles:");
        ImGui::Spacing();

        if (ImGui::BeginListBox("Drivers Instalados", ImVec2(0, 200))) {
            for (size_t i = 0; i < drivers.size(); i++) {
                bool is_selected = (selected_driver == static_cast<int>(i));
                if (ImGui::Selectable(drivers[i].name.c_str(), is_selected)) {
                    selected_driver = static_cast<int>(i);
                    GetConfig().vulkan_driver = drivers[i].name;
                }
            }
            ImGui::EndListBox();
        }

        ImGui::Spacing();
        if (ImGui::Button("Agregar Driver Custom")) {
            SwitchToState(MenuState::VulkanCatalog);
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderRendererSelectorMenu() {
    RendererType current = GetConfig().renderer;

    if (ImGui::Begin("Selector de Renderizador")) {
        if (ImGui::RadioButton("Xenia (Recompilado)", current == RendererType::XeniaBased)) {
            GetConfig().renderer = RendererType::XeniaBased;
        }
        ImGui::Spacing();

        if (ImGui::RadioButton("Vulkan Nativo", current == RendererType::NativeVulkan)) {
            GetConfig().renderer = RendererType::NativeVulkan;
        }
        ImGui::Spacing();

        if (ImGui::RadioButton("OpenGL (Fallback)", current == RendererType::OpenGL)) {
            GetConfig().renderer = RendererType::OpenGL;
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Vulkan recomendado para mejor rendimiento");
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderGraphicsQualityMenu() {
    if (ImGui::Begin("Calidad Gráfica Personalizada")) {
        // Per-setting sliders
        ImGui::SliderInt("Resolución", &GetConfig().resolution_width, 720, 3840);
        GetConfig().resolution_height = static_cast<int>(GetConfig().resolution_width * 0.5625f);

        ImGui::SliderInt("MSAA Samples", &GetConfig().msaa_samples, 0, 16);
        ImGui::Checkbox("Motion Blur", &GetConfig().motion_blur);
        ImGui::Checkbox("Impostor Shadows", &GetConfig().imposter_shadows);
        ImGui::SliderFloat("FOV", &GetConfig().fov_scale, 0.5f, 1.5f);
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderCreditsMenu() {
    if (ImGui::Begin("Créditos")) {
        ImGui::Text("Midnight Club: Los Angeles - Xbox 360 Emulation");
        ImGui::Spacing();
        ImGui::Text("Desarrollado por:");
        ImGui::Text("- Xbox Recomp Team");
        ImGui::Spacing();
        ImGui::Text("Agradecimientos:");
        ImGui::Text("- Xbox 360 Community");
        ImGui::Text("- Dolphin Devs");
        ImGui::Text("- ReXGlue SDK Contributors");
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderCrashLogsMenu() {
    if (ImGui::Begin("Logs de Error")) {
        ImGui::Text("Últimos errores:");
        ImGui::BeginChild("logs", ImVec2(0, 0), true);
        ImGui::Text("No hay errores registrados");
        ImGui::EndChild();

        if (ImGui::Button("Enviar Log al Desarrollador")) {
            // Email/Send intent
        }
        ImGui::SameLine();
        if (ImGui::Button("Borrar Logs")) {
            // Clear
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderShaderGeneratorMenu() {
    static bool generate_all = false;
    
    if (ImGui::Begin("Generador de Shaders")) {
        ImGui::Text("Herramienta para generar shaders optimizados para múltiples dispositivos");
        ImGui::Spacing();

        if (ImGui::Button("Generar Shaders para este dispositivo")) {
            // Trigger shader generation
            generate_all = true;
        }
        ImGui::Spacing();

        if (ImGui::Button("Exportar Shaders personalizados")) {
            // Export
        }
        ImGui::Spacing();
        
        if (ImGui::Button("Limpiar Caché")) {
            // Clear cache
        }
        
        // Suppress unused variable warning (placeholder for future implementation)
        (void)generate_all;
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderLANMenu() {
    ImGui::Checkbox("Habilitar LAN Mode", &GetConfig().lan_mode_enabled);

    if (ImGui::Begin("Modo LAN")) {
        if (GetConfig().lan_mode_enabled) {
            static char game_tag[64] = "";
            ImGui::InputText("Game Tag", game_tag, 64);
            ImGui::SameLine();
            ImGui::Text("ID: %s", game_tag);

            if (ImGui::Button("Buscar Juegos")) {
                // LAN discovery
            }
            if (ImGui::Button("Crear Sala")) {
                // Host
            }
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Nota: Requiere conexión WiFi");
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderDevelopmentModeMenu() {
    if (ImGui::Begin("Modo Desarrollo")) {
        ImGui::Checkbox("Modo Dev", &GetConfig().dev_mode);
        ImGui::Checkbox("Mostrar FPS", &GetConfig().show_fps);
        ImGui::Checkbox("Debug Info", &GetConfig().show_debug_info);

        ImGui::Spacing();
        if (ImGui::Button("Ver Function Registry")) {
            // Show registered functions count
            SDL_Log("Registered functions: %d", 30047);
        }
        ImGui::Spacing();
        if (ImGui::Button("Forzar Recompilación")) {
            // JIT cache clear
        }
        ImGui::Spacing();
        
        if (ImGui::Button("Guardar Configuración")) {
            GetConfig().Save();
        }
        ImGui::SameLine();
        if (ImGui::Button("Recargar Configuración")) {
            GetConfig().Load();
        }
    }
    ImGui::End();

    if (ImGui::Button("< Atrás")) {
        NavigateBack();
    }
}

void MenuSystem::RenderHUD(float fps) {
    if (fps <= 0) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.3f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("HUD", nullptr, window_flags)) {
        // FPS counter (MangoHUD style)
        ImVec4 fps_color = fps >= 60.0f ? ImVec4(0, 1, 0, 1) :
                          fps >= 30.0f ? ImVec4(1, 1, 0, 1) :
                          ImVec4(1, 0, 0, 1);

        ImGui::TextColored(fps_color, "FPS: %.1f", fps);

        // Frame time
        float frame_time = 1000.0f / fps;
        ImGui::SameLine();
        ImGui::Text("Frame Time: %.2f ms", frame_time);

        // Simple progress bar
        float fill_ratio = std::min(1.0f, fps / 60.0f);
        ImGui::ProgressBar(fill_ratio, ImVec2(-1, 0), "");
    }
    ImGui::End();
}

void MenuSystem::RenderVirtualGamepad() {
    if (!GetConfig().virtual_gamepad_enabled) return;
    
    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 200), 
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.5f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("Virtual Gamepad", nullptr, window_flags)) {
        ImGui::Text("Virtual Gamepad");
        // Virtual gamepad visualization can be added here
    }
    ImGui::End();
}

void MenuSystem::SetVirtualGamepadVisible(bool visible) {
    m_virtual_gamepad_visible = visible;
}

void MenuSystem::DrawButton(const std::string& label, float width) {
    if (width > 0) {
        ImGui::SetNextItemWidth(width);
    }
    ImGui::Button(label.c_str());
}

MenuSystem& GetMenus() {
    static MenuSystem instance;
    return instance;
}

} // namespace mcla