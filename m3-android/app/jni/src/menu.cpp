// MCLA - Menu System Implementation
// Renders via ReXGlue ImmediateDrawer (Vulkan). No OpenGL ES dependency.

#include <android/log.h>
#define IM_ASSERT(_EXPR) do { if (!(_EXPR)) { __android_log_print(ANDROID_LOG_ERROR, "MCLA_Menu", "IM_ASSERT FAILED: %s at line %d", #_EXPR, __LINE__); raise(SIGABRT); } } while(0)

#include "menu.h"
#include "config.h"
#include "gamepad_manager.h"
#include "vulkan_catalog.h"
#include "shader_generator.h"
#include "crash_logger.h"
#include "updater.h"

#include <android/log.h>
#include <android/asset_manager.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_events.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>

// stb_image for loading textures from memory
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

#include <rex/ui/immediate_drawer.h>
#include <rex/ui/ui_drawer.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MCLA_Menu", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MCLA_Menu", __VA_ARGS__)

namespace mcla {

MenuSystem::MenuSystem() = default;
MenuSystem::~MenuSystem() = default;

void MenuSystem::Initialize(rex::ui::ImmediateDrawer* drawer) {
    if (m_initialized) return;

    m_immediate_drawer = drawer;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigNavMoveSetMousePos = true;

    // Set initial display size (required before first NewFrame)
    io.DisplaySize = {(float)m_display_width, (float)m_display_height};

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 7.0f;
    style.WindowPadding = ImVec2(12, 12);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.ScrollbarRounding = 6.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
    style.GrabRounding = 4.0f;

    // Load configuration
    GetConfig().Load();

    // Build + upload the font atlas so io.Fonts->TexIsBuilt is set before the
    // first NewFrame(). Replicate ImGuiDrawer::SetupFontTexture() using our own
    // ImmediateDrawer (the SDK ImGuiDrawer is not used here).
    if (m_immediate_drawer) {
        unsigned char* font_pixels = nullptr;
        int font_w = 0, font_h = 0;
        io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_w, &font_h);
        if (font_pixels) {
            auto font_tex = m_immediate_drawer->CreateTexture(
                static_cast<uint32_t>(font_w), static_cast<uint32_t>(font_h),
                rex::ui::ImmediateTextureFilter::kLinear,
                true,  // repeat
                font_pixels);
            if (font_tex) {
                io.Fonts->TexID = reinterpret_cast<ImTextureID>(font_tex.get());
                m_font_texture = std::move(font_tex);
                LOGI("Font atlas uploaded (%dx%d)", font_w, font_h);
            } else {
                LOGE("CreateTexture failed for font atlas");
            }
        }
    }

    m_initialized = true;
    LOGI("Menu system initialized (vulkan path)");
}

void MenuSystem::Shutdown() {
    if (!m_initialized) return;

    // Clean up owned textures
    m_textures.clear();
    m_font_texture.reset();

    ImGui::DestroyContext();
    m_initialized = false;
}

void MenuSystem::Draw(rex::ui::UIDrawContext& ui_draw_context) {
    if (!m_initialized) return;

    // TEMP DIAGNOSTIC
    static int draw_log_count = 0;
    if (draw_log_count < 5) {
        LOGI("MenuSystem::Draw called (draw_data pending=%d)", m_game_running.load() ? 1 : 0);
        ++draw_log_count;
    }

    // Update display size each frame (must be set before NewFrame)
    if (m_display_width > 0 && m_display_height > 0) {
        ImGui::GetIO().DisplaySize = {(float)m_display_width, (float)m_display_height};
    }

    // Start ImGui frame
    ImGui::NewFrame();

    // Render current menu or HUD
    if (m_game_running.load()) {
        RenderHUD(GetConfig().show_fps ? m_fps : 0.0f);
        RenderVirtualGamepad();
    } else {
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

    // End frame — this builds ImGui draw data
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data) {
        // Replicate ImGuiDrawer::RenderDrawLists exactly using ImmediateDrawer.
        ImGuiIO& io = ImGui::GetIO();

        m_immediate_drawer->Begin(ui_draw_context, io.DisplaySize.x, io.DisplaySize.y);

        for (int i = 0; i < draw_data->CmdListsCount; ++i) {
            const auto cmd_list = draw_data->CmdLists[i];

            rex::ui::ImmediateDrawBatch batch;
            batch.vertices = reinterpret_cast<rex::ui::ImmediateVertex*>(cmd_list->VtxBuffer.Data);
            batch.vertex_count = cmd_list->VtxBuffer.size();
            batch.indices = cmd_list->IdxBuffer.Data;
            batch.index_count = cmd_list->IdxBuffer.size();

            m_immediate_drawer->BeginDrawBatch(batch);

            for (int j = 0; j < cmd_list->CmdBuffer.size(); ++j) {
                const auto& cmd = cmd_list->CmdBuffer[j];

                rex::ui::ImmediateDraw draw;
                draw.primitive_type = rex::ui::ImmediatePrimitiveType::kTriangles;
                draw.count = cmd.ElemCount;
                draw.index_offset = cmd.IdxOffset;
                draw.base_vertex = 0;
                draw.texture = reinterpret_cast<rex::ui::ImmediateTexture*>(cmd.GetTexID());
                draw.scissor = true;
                draw.scissor_left = cmd.ClipRect.x;
                draw.scissor_top = cmd.ClipRect.y;
                draw.scissor_right = cmd.ClipRect.z;
                draw.scissor_bottom = cmd.ClipRect.w;

                m_immediate_drawer->Draw(draw);
            }

            m_immediate_drawer->EndDrawBatch();
        }

        m_immediate_drawer->End();
    }
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

void MenuSystem::HandleEvent(const SDL_Event* event) {
    if (!m_initialized) return;

    ImGuiIO& io = ImGui::GetIO();

    switch (event->type) {
        case SDL_EVENT_MOUSE_MOTION:
            io.AddMousePosEvent((float)event->motion.x, (float)event->motion.y);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            // Update mouse position BEFORE the button event
            io.AddMousePosEvent((float)event->button.x, (float)event->button.y);
            io.AddMouseButtonEvent(event->button.button, event->button.down);
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL:
            io.AddMouseWheelEvent((float)event->wheel.y, 0.0f);
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            // Convert SDL keycode to ImGuiKey
            ImGuiKey key = ImGuiKey_None;
            SDL_Scancode scancode = event->key.scancode;

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
            if (event->type == SDL_EVENT_KEY_DOWN && key == ImGuiKey_Escape) {
                GetMenus().NavigateBack();
            }
            break;
        }
        default:
            break;
    }
}

void MenuSystem::SetVirtualGamepadVisible(bool visible) {
    m_virtual_gamepad_visible = visible;
}

// ============================================
// Texture loading
// ============================================

void MenuSystem::LoadTextures(AAssetManager* assetManager) {
    if (!m_immediate_drawer || !assetManager) {
        LOGE("Cannot load textures: no drawer or asset manager");
        return;
    }

    LOGI("Loading menu textures from assets...");

    auto load_one = [this, assetManager](const char* assetPath, ImTextureID& out_id) {
        AAsset* asset = AAssetManager_open(assetManager, assetPath, AASSET_MODE_BUFFER);
        if (!asset) {
            LOGE("Failed to open asset: %s", assetPath);
            return;
        }
        off_t assetSize = AAsset_getLength(asset);
        if (assetSize <= 0) {
            LOGE("Asset %s has invalid size: %ld", assetPath, (long)assetSize);
            AAsset_close(asset);
            return;
        }
        const void* assetData = AAsset_getBuffer(asset);
        if (!assetData) {
            LOGE("Failed to get buffer for asset: %s", assetPath);
            AAsset_close(asset);
            return;
        }
        int width, height, channels;
        stbi_uc* pixels = stbi_load_from_memory(
            static_cast<const stbi_uc*>(assetData),
            static_cast<int>(assetSize),
            &width, &height, &channels, 4);
        AAsset_close(asset);
        if (!pixels) {
            LOGE("stbi_load_from_memory failed for: %s", assetPath);
            return;
        }

        auto tex = m_immediate_drawer->CreateTexture(
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            rex::ui::ImmediateTextureFilter::kLinear,
            false,  // not repeated
            pixels
        );
        stbi_image_free(pixels);
        if (!tex) {
            LOGE("CreateTexture failed for: %s", assetPath);
            return;
        }
        ImTextureID id = (ImTextureID)tex.get(); // raw pointer as ImTextureID
        m_textures.push_back({std::move(tex), id});
        out_id = id;
        LOGI("Loaded texture: %s (%dx%d)", assetPath, width, height);
    };

    load_one("menu_images/backdrop.png", m_backdrop_texture);
    load_one("menu_images/icon_jugar.png", m_icon_jugar);
    load_one("menu_images/icon_config.png", m_icon_config);
    load_one("menu_images/icon_controles.png", m_icon_controles);
    load_one("menu_images/icon_desarrollo.png", m_icon_desarrollo);
    load_one("menu_images/icon_creditos.png", m_icon_creditos);
    load_one("menu_images/icon_actualizar.png", m_icon_actualizar);
    load_one("menu_images/icon_salir.png", m_icon_salir);

    LOGI("Menu textures loaded: %zu textures", m_textures.size());
}

// ============================================
// Menu Rendering Functions
// ============================================

static bool ImageButtonWithLabel(ImTextureID texture, const char* label, const ImVec2& iconSize, const ImVec2& buttonSize) {
    ImGui::PushID(label);
    bool clicked = false;

    if (texture) {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImGui::Image(texture, iconSize);
        ImGui::SameLine(0, 12);
        ImGui::SetCursorPosY(cursor.y + (iconSize.y - ImGui::GetFontSize()) * 0.5f);
        clicked = ImGui::Button(label, ImVec2(buttonSize.x - iconSize.x - 12, buttonSize.y));
    } else {
        clicked = ImGui::Button(label, buttonSize);
    }

    ImGui::PopID();
    return clicked;
}

void MenuSystem::RenderMainMenu() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    // Full-screen backdrop
    if (m_backdrop_texture) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(displaySize);
        ImGuiWindowFlags backdropFlags = ImGuiWindowFlags_NoTitleBar |
                                         ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoScrollbar |
                                         ImGuiWindowFlags_NoCollapse |
                                         ImGuiWindowFlags_NoNav |
                                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                                         ImGuiWindowFlags_NoBackground;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

        if (ImGui::Begin("##backdrop", nullptr, backdropFlags)) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddImage(m_backdrop_texture, ImVec2(0, 0), displaySize,
                               ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    // Main menu window (centered)
    float menuWidth = 500;
    float menuHeight = 580;
    float menuX = (displaySize.x - menuWidth) * 0.5f;
    float menuY = (displaySize.y - menuHeight) * 0.5f;

    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(menuX, menuY), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("MCLA Launcher", nullptr, window_flags)) {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SetCursorPosX((menuWidth - 400) * 0.5f);
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 1.0f, 1.0f), "MIDNIGHT CLUB:");
        ImGui::SetCursorPosX((menuWidth - 400) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 1.0f, 1.0f), "LOS ANGELES");
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        const ImVec2 iconSize(32, 32);
        const ImVec2 buttonSize(320, 50);
        float btnX = (menuWidth - buttonSize.x) * 0.5f;
        ImGui::SetCursorPosX(btnX);

        static bool first_frame_focus = true;
        if (first_frame_focus) {
            ImGui::SetKeyboardFocusHere();
            first_frame_focus = false;
        }

        if (ImageButtonWithLabel(m_icon_jugar, "JUGAR", iconSize, buttonSize)) {
            LOGI("JUGAR button CLICKED!");
            m_game_start_requested = true;
            SwitchToState(MenuState::GameRunning);
        }
        if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            LOGI("JUGAR button focused + Enter pressed!");
            m_game_start_requested = true;
            SwitchToState(MenuState::GameRunning);
        }
        ImGui::Spacing();
        ImGui::SetCursorPosX(btnX);

        if (ImageButtonWithLabel(m_icon_config, "CONFIGURACIÓN", iconSize, buttonSize)) {
            SwitchToState(MenuState::GraphicsSettings);
        }
        ImGui::Spacing();
        ImGui::SetCursorPosX(btnX);

        if (ImageButtonWithLabel(m_icon_controles, "CONTROLES", iconSize, buttonSize)) {
            SwitchToState(MenuState::LanguageSettings);
        }
        ImGui::Spacing();
        ImGui::SetCursorPosX(btnX);

        if (ImageButtonWithLabel(m_icon_desarrollo, "DESARROLLO", iconSize, buttonSize)) {
            SwitchToState(MenuState::DevelopmentMode);
        }
        ImGui::Spacing();
        ImGui::SetCursorPosX(btnX);

        if (ImageButtonWithLabel(m_icon_creditos, "CRÉDITOS", iconSize, buttonSize)) {
            SwitchToState(MenuState::Credits);
        }
        ImGui::Spacing();
        ImGui::SetCursorPosX(btnX);

        if (ImageButtonWithLabel(m_icon_actualizar, "ACTUALIZAR", iconSize, buttonSize)) {
            // TODO: Update logic
        }
        ImGui::Spacing();
        ImGui::SetCursorPosX(btnX);

        if (ImageButtonWithLabel(m_icon_salir, "SALIR", iconSize, buttonSize)) {
            exit(0);
        }
    }
    ImGui::End();
}

void MenuSystem::RenderGameFileSelector() {
    if (ImGui::Begin("Seleccionar Archivo de Juego")) {
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
        ImGui::Checkbox("Salud Ilimitada", &infinite_health);
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
        ImVec4 fps_color = fps >= 60.0f ? ImVec4(0, 1, 0, 1) :
                          fps >= 30.0f ? ImVec4(1, 1, 0, 1) :
                          ImVec4(1, 0, 0, 1);

        ImGui::TextColored(fps_color, "FPS: %.1f", fps);

        float frame_time = 1000.0f / fps;
        ImGui::SameLine();
        ImGui::Text("Frame Time: %.2f ms", frame_time);

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
    }
    ImGui::End();
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