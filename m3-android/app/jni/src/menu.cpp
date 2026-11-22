// MCLA - Menu System Implementation
// Renders via ReXGlue ImmediateDrawer (Vulkan). No OpenGL ES dependency.

#include <android/log.h>
#include <thread>
#include <functional>
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

// Touch mapping is derived at runtime from the actual UI draw-target size
// reported by the presenter's UIDrawContext (the Vulkan swapchain extent),
// captured in Draw(). No hardcoded render-target dimensions: the previous
// 2306x950 constants produced a phantom (0,65) letterbox offset on devices
// whose swapchain matches the surface (e.g. 2400x1080).

MenuSystem::MenuSystem() = default;
MenuSystem::~MenuSystem() = default;

void MenuSystem::SetSize(int width, int height) {
    m_display_width = width;
    m_display_height = height;
    UpdateTouchMapping();
}

void MenuSystem::UpdateTouchMapping() {
    // The presenter draws UI directly into the swapchain at its full extent,
    // so the UI coordinate space == swapchain space. Map surface pixels into
    // that space with an aspect-fit (letterbox) transform; when the swapchain
    // matches the surface this is the identity (scale=1, offset=0).
    const int rt_w = m_rt_width > 0 ? m_rt_width : m_display_width;
    const int rt_h = m_rt_height > 0 ? m_rt_height : m_display_height;
    if (rt_w <= 0 || rt_h <= 0 || m_display_width <= 0 || m_display_height <= 0) {
        m_touch_scale = 1.0f;
        m_touch_offset_x = 0.0f;
        m_touch_offset_y = 0.0f;
        return;
    }
    const float sx = static_cast<float>(m_display_width) / static_cast<float>(rt_w);
    const float sy = static_cast<float>(m_display_height) / static_cast<float>(rt_h);
    m_touch_scale = std::min(sx, sy);
    m_touch_offset_x = (m_display_width - rt_w * m_touch_scale) * 0.5f;
    m_touch_offset_y = (m_display_height - rt_h * m_touch_scale) * 0.5f;
    LOGI("Touch mapping: scale=%.3f offset=(%.1f, %.1f) surface=%dx%d rt=%dx%d",
         (double)m_touch_scale, (double)m_touch_offset_x, (double)m_touch_offset_y,
         m_display_width, m_display_height, rt_w, rt_h);
}

void MenuSystem::Initialize(rex::ui::ImmediateDrawer* drawer) {
    if (m_initialized) return;

    m_immediate_drawer = drawer;

    // Touch mapping is derived from the real draw-target size (captured in
    // Draw()); until the first paint, assume identity surface->UI mapping.
    UpdateTouchMapping();

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

ImVec2 MenuSystem::MapTouchEventCoords(float screen_x, float screen_y) const {
    // Convert from surface-pixel coords into the UI draw-target space used by
    // ImGui (io.DisplaySize, set from the presenter's UIDrawContext in Draw()).
    const float scale = m_touch_scale > 0.0f ? m_touch_scale : 1.0f;
    ImVec2 out{(screen_x - m_touch_offset_x) / scale,
               (screen_y - m_touch_offset_y) / scale};
    // Clamp into [0, rt] so touches outside the active area don't leak in.
    const float max_x = static_cast<float>(m_rt_width > 0 ? m_rt_width : m_display_width);
    const float max_y = static_cast<float>(m_rt_height > 0 ? m_rt_height : m_display_height);
    out.x = std::max(0.0f, std::min(out.x, max_x));
    out.y = std::max(0.0f, std::min(out.y, max_y));
    return out;
}

void MenuSystem::Draw(rex::ui::UIDrawContext& ui_draw_context) {
    if (!m_initialized) return;

    // Align ImGui coordinate space with the actual Vulkan render target so
    // ImGui scaling/positioning matches what the ImmediateDrawer actually presents.
    ImGuiIO& io_ = ImGui::GetIO();
    io_.DisplaySize.x = static_cast<float>(ui_draw_context.render_target_width());
    io_.DisplaySize.y = static_cast<float>(ui_draw_context.render_target_height());

    // Track the real draw-target size so touch mapping matches what is
    // actually presented (refresh on swapchain/surface resize).
    const int rt_w = static_cast<int>(ui_draw_context.render_target_width());
    const int rt_h = static_cast<int>(ui_draw_context.render_target_height());
    if (rt_w != m_rt_width || rt_h != m_rt_height) {
        m_rt_width = rt_w;
        m_rt_height = rt_h;
        UpdateTouchMapping();
    }

    // Defer-inject the pending mouse UP across a frame boundary so that
    // Button()'s PressedOnClickRelease logic (which fires on the release) works
    // even when Android delivers DOWN+UP within the same SDL_PollEvent batch.
    if (m_up_defer_counter > 0) {
        m_up_defer_counter--;
        if (m_up_defer_counter == 0) {
            m_mouse_down = false;
            io_.AddMouseButtonEvent(0, false);
        }
    }

    // Start ImGui frame
    ImGui::NewFrame();

    // Render current menu or HUD. The HUD is drawn last (on top of menus and
    // the virtual pad) so position/size changes are visible live while editing.
    if (m_game_running.load()) {
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

    if (GetConfig().show_fps && m_fps > 0) {
        RenderHUD(m_fps);
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
            io.AddMousePosEvent(event->motion.x, event->motion.y);
            { static int ml=0; if(ml<5){++ml; LOGI("MOUSE_MOTION: pos=(%.1f, %.1f)", (double)event->motion.x, (double)event->motion.y);} }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            // Update mouse position BEFORE the button event
            io.AddMousePosEvent(event->button.x, event->button.y);
            { static int bl=0; if(bl<5){++bl; LOGI("MOUSE_BTN: type=%d btn=%d down=%d pos=(%.1f, %.1f)",
                  (int)event->type, (int)event->button.button, (int)event->button.down,
                  (double)event->button.x, (double)event->button.y);} }
            if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                m_mouse_down = true;
                // Queue the DOWN so ImGui knows the cursor source etc.
                // SDL uses 1-based button indices (1=left), ImGui uses 0-based (0=left)
                const int imgui_button = event->button.button - 1;
                io.AddMouseButtonEvent(imgui_button, true);
            } else {
                // Defer the UP across a frame boundary: schedule it for injection
                // at the start of the 2nd Draw after this one. This guarantees
                // the DOWN is observed for a full NewFrame first, so that Button()'s
                // PressedOnClickRelease logic (which fires on the release) triggers.
                m_up_defer_counter = 2;
            }
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

void MenuSystem::RenderMainMenu() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    // ── Bottom-right panel with large touch targets ─────────────────────
    constexpr float kItemCount = 7.0f;
    const float margin = 30.0f;
    const float panelW = ImMin(640.0f, displaySize.x - margin * 2.0f);
    const float pad  = 34.0f;
    const float br   = 18.0f;

    const float title_size = ImMin(30.0f, panelW / 18.0f);
    const float title_line = title_size + 10.0f;
    const float title_top  = 46.0f;
    const float btn_top    = title_top + title_line * 2.0f + 26.0f;
    const float version_h  = 36.0f;
    const float gap        = 14.0f;
    // Only shrink buttons on displays too short for the full layout.
    const float avail_h = displaySize.y - margin * 2.0f - btn_top - version_h;
    const float btnH = ImMax(36.0f, ImMin(74.0f,
                         (avail_h - (kItemCount - 1.0f) * gap) / kItemCount));
    const float panelH = btn_top + kItemCount * btnH + (kItemCount - 1.0f) * gap + version_h;
    const float px = displaySize.x - panelW - margin;   // right edge
    const float py = displaySize.y - panelH - margin;   // bottom edge

    // ── Backdrop: keep the car art clearly visible (light scrim only) ───
    if (m_backdrop_texture) {
        dl->AddImage(m_backdrop_texture, ImVec2(0, 0), displaySize,
                     ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
    }
    dl->AddRectFilled(ImVec2(0, 0), displaySize, IM_COL32(0, 0, 0, 80), 0);
    // Panel shadow
    dl->AddRectFilled(ImVec2(px + 6, py + 8), ImVec2(px + panelW + 6, py + panelH + 8),
                      IM_COL32(0, 0, 0, 140), br + 2);
    // Panel background (dark glass)
    dl->AddRectFilled(ImVec2(px, py), ImVec2(px + panelW, py + panelH),
                      IM_COL32(14, 14, 20, 235), br);
    // Subtle top-edge highlight
    dl->AddLine(ImVec2(px + br, py), ImVec2(px + panelW - br, py),
                IM_COL32(120, 130, 160, 90), 2.0f);

    // ── Title ────────────────────────────────────────────────────────────
    const float titleX = px + panelW * 0.5f;
    const float titleY = py + title_top;
    {
        static const char* sub[] = {"MIDNIGHT CLUB:", "LOS ANGELES"};
        static const ImVec4 col[] = {{0.2f, 1.0f, 1.0f, 1.0f},
                                     {1.0f, 0.22f, 0.85f, 1.0f}};
        int idx = 0;
        for (auto txt : sub) {
            ImFont* font = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;
            ImVec2 ts = font ? font->CalcTextSizeA(title_size, 9999, -1.0f, txt)
                             : ImVec2(strlen(txt) * title_size * 0.6f, title_size);
            dl->AddText(font, title_size, ImVec2(titleX - ts.x * 0.5f, titleY + idx * title_line),
                        IM_COL32((int)(col[idx].x*255),(int)(col[idx].y*255),(int)(col[idx].z*255),255), txt);
            idx++;
        }
    }

    // ── Buttons ──────────────────────────────────────────────────────────
    struct MenuItem {
        const char* label;
        std::function<void()> action;
    };
    static const MenuItem items[] = {
        {"JUGAR",          [&]{ LOGI("JUGAR clicked"); m_game_start_requested = true; SwitchToState(MenuState::GameRunning); }},
        {"CONFIGURACIÓN",  [&]{ SwitchToState(MenuState::GraphicsSettings); }},
        {"CONTROLES",      [&]{ SwitchToState(MenuState::LanguageSettings); }},
        {"DESARROLLO",     [&]{ SwitchToState(MenuState::DevelopmentMode); }},
        {"CRÉDITOS",       [&]{ SwitchToState(MenuState::Credits); }},
        {"ACTUALIZAR",     [&]{ /* TODO */ }},
        {"SALIR",          [&]{ exit(0); }},
    };

    const int N = sizeof(items) / sizeof(items[0]);
    const float btnW = panelW - pad * 2.0f;
    const float label_size = ImMin(26.0f, btnH * 0.40f);
    const ImVec4 col_text    = ImVec4(0.92f, 0.92f, 0.95f, 1.0f);
    const ImVec4 col_dim     = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
    const ImU32  col_btn_n   = IM_COL32(32,  32,  42,  230);
    const ImU32  col_btn_h   = IM_COL32(55,  55,  72,  245);
    const ImU32  col_accent  = IM_COL32(0,   179, 68,  255);  // Xbox green
    const ImU32  col_border  = IM_COL32(80,  85,  100, 180);
    const ImU32  col_border_h= IM_COL32(140, 150, 180, 220);

    float curY = py + btn_top;

    // Pre-calc hover rect each frame so we know which item is hovered.
    ImVec2 mouse = io.MousePos;
    int hover_idx = -1;
    for (int i = 0; i < N; ++i) {
        float bx = px + pad;
        float by = curY + i * (btnH + gap);
        if (mouse.x >= bx && mouse.x <= bx + btnW &&
            mouse.y >= by && mouse.y <= by + btnH) {
            hover_idx = i;
        }
    }

    for (int i = 0; i < N; ++i) {
        float bx = px + pad;
        float by = curY + i * (btnH + gap);
        bool hovered = (hover_idx == i);
        bool pressed = hovered && io.WantCaptureMouse && io.MouseDown[0];

        // Button bg
        ImU32 bg = pressed ? col_btn_h : (hovered ? col_btn_h : col_btn_n);
        dl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + btnW, by + btnH), bg, 14.0f);

        // Left accent bar (green for JUGAR, subtle for others)
        ImU32 accent_col = (items[i].label[0] == 'J') ? col_accent : col_border;
        if (hovered) accent_col = col_border_h;
        dl->AddRectFilled(ImVec2(bx, by + 10), ImVec2(bx + 7, by + btnH - 10), accent_col, 3);

        // Border
        dl->AddRect(ImVec2(bx, by), ImVec2(bx + btnW, by + btnH),
                    hovered ? col_border_h : IM_COL32(20, 20, 28, 120),
                    14.0f, 0, 1.5f);

        // Label
        ImFont* font = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;
        ImVec2 ts = font ? font->CalcTextSizeA(label_size, 9999, -1.0f, items[i].label)
                         : ImVec2(strlen(items[i].label) * label_size * 0.6f, label_size);
        ImVec4 lblCol = pressed ? col_dim : col_text;
        dl->AddText(font, label_size, ImVec2(bx + 28, by + (btnH - ts.y) * 0.5f),
                    IM_COL32((int)(lblCol.x*255),(int)(lblCol.y*255),(int)(lblCol.z*255),255),
                    items[i].label);

        // Click handling
        if (hovered && io.MouseClicked[0]) {
            items[i].action();
        }
    }

    // ── Version / info line at bottom ────────────────────────────────────
    {
        ImFont* font = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;
        const char* ver = "v0.1.0 — MCLA Android Recompilation";
        const float ver_size = 16.0f;
        ImVec2 vs = font ? font->CalcTextSizeA(ver_size, 9999, -1.0f, ver)
                         : ImVec2(strlen(ver) * ver_size * 0.6f, ver_size);
        dl->AddText(font, ver_size, ImVec2(px + (panelW - vs.x) * 0.5f, py + panelH - 30),
                    IM_COL32(110, 110, 125, 200), ver);
    }
}

// Number of xarchive_*.rpf archives a full install is expected to carry.
static constexpr int kRequiredArchives = 6;

// Validates whether a directory contains a loadable MCLA game install.
static bool IsValidGameDir(const std::filesystem::path& dir) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) return false;
    bool has_xex = false;
    int rpf_count = 0;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        std::string name = entry.path().filename().string();
        std::string lower;
        lower.reserve(name.size());
        for (char c : name) lower += (char)::tolower(c);
        if (lower == "default.xex") has_xex = true;
        else if (lower.find("xarchive_") == 0 && lower.rfind(".rpf") == lower.size() - 4) ++rpf_count;
    }
    return has_xex && rpf_count >= kRequiredArchives;
}

// Counts the required game files present in a directory (for the validator UI).
static int CountGameFiles(const std::filesystem::path& dir) {
    namespace fs = std::filesystem;
    std::error_code ec;
    int present = 0;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        std::string name = entry.path().filename().string();
        std::string lower;
        lower.reserve(name.size());
        for (char c : name) lower += (char)::tolower(c);
        if (lower == "default.xex" || (lower.find("xarchive_") == 0 && lower.rfind(".rpf") == lower.size() - 4) ||
            lower == "nxeart" || lower.rfind(".bik") == lower.size() - 4)
            ++present;
    }
    return present;
}

void MenuSystem::RenderGameFileSelector() {
    if (ImGui::Begin("Seleccionar Carpeta de Juego")) {
        ImGui::TextWrapped("Busca la carpeta donde est\u00e1n los datos del juego "
                           "(debe contener default.xex y los xarchive_*.rpf).");

        // Current browse directory. Seed from the configured path or a sane default.
        static std::string s_browse_dir;
        if (s_browse_dir.empty()) {
            const std::string& cur = GetConfig().game_data_path;
            s_browse_dir = cur.empty() ? "/" : cur;
        }

        namespace fs = std::filesystem;

        ImGui::Text("Ruta:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.8f, 1.0f, 1.0f), "%s", s_browse_dir.c_str());
        ImGui::Separator();

        // ── Directory navigation ───────────────────────────────────────────
        if (ImGui::BeginChild("dir_browser", ImVec2(0, 260), true)) {
            std::error_code ec;
            if (fs::is_directory(s_browse_dir, ec)) {
                if (ImGui::Selectable("..  (subir)", false, ImGuiSelectableFlags_DontClosePopups)) {
                    fs::path parent = fs::path(s_browse_dir).parent_path();
                    s_browse_dir = parent.empty() ? "/" : parent.string();
                }
                ImGui::Separator();

                // Subdirectories first
                std::vector<std::string> dirs, files;
                for (const auto& entry : fs::directory_iterator(s_browse_dir, ec)) {
                    if (ec) break;
                    std::string name = entry.path().filename().string();
                    if (name.empty()) continue;
                    if (entry.is_directory(ec)) dirs.push_back(name);
                    else if (entry.is_regular_file(ec)) {
                        std::string lower;
                        for (char c : name) lower += (char)::tolower(c);
                        if (lower == "default.xex" || lower.find("xarchive_") == 0) files.push_back(name);
                    }
                }
                std::sort(dirs.begin(), dirs.end());
                std::sort(files.begin(), files.end());

                for (const auto& d : dirs) {
                    if (ImGui::Selectable(("[DIR] " + d).c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
                        s_browse_dir = (fs::path(s_browse_dir) / d).string();
                    }
                }
                for (const auto& f : files) {
                    bool is_xex = f.size() >= 4 && f.compare(f.size() - 4, 4, ".xex") == 0;
                    ImGui::Selectable(((is_xex ? "[XEX] " : "[RPF] ") + f).c_str(), false, ImGuiSelectableFlags_Disabled);
                }

                // Validation banner for the current folder
                bool valid = IsValidGameDir(s_browse_dir);
                int present = CountGameFiles(s_browse_dir);
                ImGui::Separator();
                if (valid) {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "\u2713 Carpeta v\u00e1lida (%d archivos encontrados)", present);
                } else if (!s_browse_dir.empty() && s_browse_dir != "/") {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "(%d archivos \u2014 falta default.xex o un xarchive)", present);
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Ruta inv\u00e1lida o inaccesible.");
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();
        bool valid = IsValidGameDir(s_browse_dir);
        if (valid) {
            if (ImGui::Button("Usar esta carpeta", ImVec2(220, 36))) {
                GetConfig().game_data_path = s_browse_dir;
                GetConfig().Save();
                SwitchToState(MenuState::LoadGame);
            }
            ImGui::SameLine();
        }
        if (ImGui::Button("Cancelar", ImVec2(120, 36))) {
            NavigateBack();
        }
    }
    ImGui::End();
}

void MenuSystem::RenderLoadGameMenu() {
    if (ImGui::Begin("Cargar Juego")) {
        ImGui::Text("Carpeta de datos: %s", GetConfig().game_data_path.empty() ? "(no configurada)" : GetConfig().game_data_path.c_str());
        ImGui::Separator();

        namespace fs = std::filesystem;
        std::error_code ecL;

        // Save dir: the runtime's save path, or <game>/saves
        static std::string s_save_dir;
        if (s_save_dir.empty()) {
            s_save_dir = GetConfig().save_path.empty()
                ? (GetConfig().game_data_path.empty() ? std::string("/") : (fs::path(GetConfig().game_data_path) / "saves").string())
                : GetConfig().save_path;
        }

        ImGui::Text("Carpeta de partidas: %s", s_save_dir.c_str());
        if (ImGui::SmallButton("Cambiar carpeta")) {
            s_save_dir = s_save_dir == "/" ? "/data/data/com.mcla.recomp/files/saves" : "/";
        }

        ImGui::Spacing();
        ImGui::Text("Partidas guardadas:");
        if (ImGui::BeginChild("save_list", ImVec2(0, 180), true)) {
            bool any = false;
            if (fs::is_directory(s_save_dir, ecL)) {
                for (const auto& entry : fs::directory_iterator(s_save_dir, ecL)) {
                    if (!entry.is_regular_file(ecL)) continue;
                    std::string name = entry.path().filename().string();
                    std::string lower;
                    for (char c : name) lower += (char)::tolower(c);
                    if (lower.rfind(".sav") == lower.size() - 4 || lower.rfind(".dat") == lower.size() - 4) {
                        any = true;
                        if (ImGui::Selectable(name.c_str())) {
                            LOGI("Seleccionada partida: %s", name.c_str());
                        }
                    }
                }
            }
            if (!any) ImGui::TextDisabled("No se encontraron partidas guardadas (.sav / .dat)");
        }
        ImGui::EndChild();

        ImGui::Spacing();
        if (ImGui::Button("Iniciar juego", ImVec2(200, 40))) {
            m_game_start_requested = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("< Atr\u00e1s")) {
            NavigateBack();
        }
    }
    ImGui::End();
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

        RenderHudControls();

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

void MenuSystem::RenderHudControls() {
    Config& cfg = GetConfig();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "HUD MangoHUD");
    ImGui::Checkbox("Mostrar FPS", &cfg.show_fps);

    int corner = static_cast<int>(cfg.hud_corner);
    const char* corner_labels[] = {"Sup. izquierda", "Sup. derecha", "Inf. izquierda", "Inf. derecha"};
    if (ImGui::Combo("Ubicación HUD", &corner, corner_labels, 4)) {
        cfg.hud_corner = static_cast<HudCorner>(corner);
    }
    ImGui::SliderFloat("Tamaño HUD", &cfg.hud_scale, 0.6f, 2.5f, "%.2fx");
    ImGui::SliderFloat("Margen X", &cfg.hud_offset_x, 0.0f, 400.0f, "%.0f px");
    ImGui::SliderFloat("Margen Y", &cfg.hud_offset_y, 0.0f, 400.0f, "%.0f px");
    if (ImGui::Button("Restablecer HUD")) {
        cfg.hud_corner = HudCorner::TopRight;
        cfg.hud_scale = 1.0f;
        cfg.hud_offset_x = 0.0f;
        cfg.hud_offset_y = 0.0f;
    }
    ImGui::TextColored(ImVec4(0.62f, 0.62f, 0.68f, 1.0f),
                       "Cambios en vivo. Usa Guardar Configuración para conservarlos.");
}

void MenuSystem::RenderDevelopmentModeMenu() {
    if (ImGui::Begin("Modo Desarrollo")) {
        ImGui::Checkbox("Modo Dev", &GetConfig().dev_mode);
        ImGui::Checkbox("Debug Info", &GetConfig().show_debug_info);
        RenderHudControls();

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

// Reads "MemAvailable" and "MemTotal" kB from /proc/meminfo; returns {total_kb, avail_kb}.
static std::pair<long, long> ReadMemInfo() {
    std::pair<long, long> result = {0, 0};
    std::ifstream f("/proc/meminfo");
    if (!f.is_open()) return result;
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("MemTotal:", 0) == 0 || line.rfind("MemAvailable:", 0) == 0) {
            long kb = 0;
            if (sscanf(line.c_str(), "%*s %ld", &kb) == 1) {
                if (line.find("MemTotal:") == 0) result.first = kb;
                else result.second = kb;
            }
        }
    }
    return result;
}

void MenuSystem::RenderHUD(float fps) {
    if (fps <= 0.0f) return;

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImFont* font = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;
    const Config& cfg = GetConfig();

    // Every dimension derives from one scale so the overlay keeps its
    // proportions no matter where it is anchored or how large it is drawn.
    const float s      = ImClamp(cfg.hud_scale, 0.5f, 3.0f);
    const float margin = 12.0f * s;
    const float pad    = 9.0f * s;
    const float gap    = 4.0f * s;
    const float fsz    = 26.0f * s;   // big FPS number
    const float small  = 12.0f * s;   // frametime / RAM
    const float tiny   = 11.0f * s;   // "FPS" unit
    const float graph_h = 44.0f * s;

    auto text_size = [&](float size, const char* t) -> ImVec2 {
        return font ? font->CalcTextSizeA(size, 9999.0f, -1.0f, t)
                    : ImVec2(strlen(t) * size * 0.6f, size);
    };
    auto draw_text = [&](float size, const ImVec2& at, ImU32 col, const char* t) {
        dl->AddText(font, size, at, col, t);
    };

    // ── Values ────────────────────────────────────────────────────────
    char fps_buf[16], ft_buf[32], mem_buf[64];
    snprintf(fps_buf, sizeof(fps_buf), "%3.0f", fps);
    snprintf(ft_buf, sizeof(ft_buf), "FRAME  %.1f ms", 1000.0f / fps);
    auto [mem_total_kb, mem_avail_kb] = ReadMemInfo();
    if (mem_total_kb > 0) {
        snprintf(mem_buf, sizeof(mem_buf), "RAM %ld / %ld MB",
                 (mem_total_kb - mem_avail_kb) / 1024L, mem_total_kb / 1024L);
    } else {
        mem_buf[0] = '\0';
    }

    const ImVec2 fps_sz = text_size(fsz, fps_buf);
    const ImVec2 ft_sz  = text_size(small, ft_buf);
    const ImVec2 mem_sz = text_size(small, mem_buf);
    const ImVec2 unit_sz = text_size(tiny, "FPS");

    float content_w = ImMax(fps_sz.x + unit_sz.x + 6.0f * s, ImMax(ft_sz.x, mem_sz.x));
    content_w = ImMax(content_w, 120.0f * s);
    const float panel_w = content_w + pad * 2.0f;
    const float panel_h = pad * 2.0f + fps_sz.y + gap + ft_sz.y + gap + graph_h + gap + mem_sz.y;

    // ── Anchor: corner + inward offsets, then clamp inside the display ─
    const ImVec2 display = io.DisplaySize;
    const bool left   = cfg.hud_corner == HudCorner::TopLeft || cfg.hud_corner == HudCorner::BottomLeft;
    const bool top    = cfg.hud_corner == HudCorner::TopLeft || cfg.hud_corner == HudCorner::TopRight;
    float px = left ? margin + cfg.hud_offset_x
                    : display.x - panel_w - margin - cfg.hud_offset_x;
    float py = top  ? margin + cfg.hud_offset_y
                    : display.y - panel_h - margin - cfg.hud_offset_y;
    px = ImClamp(px, 4.0f, ImMax(4.0f, display.x - panel_w - 4.0f));
    py = ImClamp(py, 4.0f, ImMax(4.0f, display.y - panel_h - 4.0f));
    const ImVec2 origin(px, py);

    // ── Panel ─────────────────────────────────────────────────────────
    dl->AddRectFilled(origin, ImVec2(px + panel_w, py + panel_h),
                      IM_COL32(10, 12, 18, 210), 8.0f * s);
    dl->AddRect(origin, ImVec2(px + panel_w, py + panel_h),
                IM_COL32(90, 100, 125, 110), 8.0f * s, 0, 1.0f * s);

    float x = px + pad;
    float y = py + pad;

    // FPS, coloured by threshold; unit sits beside the number, never on top.
    const ImU32 fps_col = fps >= 60.0f ? IM_COL32(80, 235, 90, 255)
                        : fps >= 30.0f ? IM_COL32(245, 215, 70, 255)
                                       : IM_COL32(245, 75, 75, 255);
    draw_text(fsz, ImVec2(x, y), fps_col, fps_buf);
    draw_text(tiny, ImVec2(x + fps_sz.x + 6.0f * s,
                           y + (fps_sz.y - unit_sz.y) * 0.5f),
              IM_COL32(150, 155, 170, 210), "FPS");
    y += fps_sz.y + gap;

    // Frametime readout, on its own row.
    draw_text(small, ImVec2(x, y), IM_COL32(190, 195, 210, 230), ft_buf);
    y += ft_sz.y + gap;

    // ── Rolling frametime graph ───────────────────────────────────────
    const float min_ft = 8.0f, max_ft = 50.0f;
    const float gx = x, gy = y, gw = content_w, gh = graph_h;
    auto ft_to_y = [&](float t) {
        return gy + gh - (ImClamp(t, min_ft, max_ft) - min_ft) / (max_ft - min_ft) * gh;
    };
    dl->AddRectFilled(ImVec2(gx, gy), ImVec2(gx + gw, gy + gh),
                      IM_COL32(24, 26, 34, 190), 3.0f * s);
    // 60 / 30 FPS reference lines.
    dl->AddLine(ImVec2(gx, ft_to_y(16.67f)), ImVec2(gx + gw, ft_to_y(16.67f)),
                IM_COL32(0, 190, 60, 110), 1.0f * s);
    dl->AddLine(ImVec2(gx, ft_to_y(33.33f)), ImVec2(gx + gw, ft_to_y(33.33f)),
                IM_COL32(210, 150, 40, 100), 1.0f * s);

    const float bar_w = ImMax(1.0f, gw / static_cast<float>(kFtBufSize));
    for (int i = 0; i < kFtBufSize; ++i) {
        const int idx = (m_ft_idx + i) & (kFtBufSize - 1);
        const float val = m_ft[idx];
        if (val <= 0.1f) continue;
        const float by = ft_to_y(val);
        const ImU32 col = val <= 16.67f ? IM_COL32(0, 205, 60, 230)
                        : val <= 33.33f ? IM_COL32(220, 185, 40, 225)
                                        : IM_COL32(220, 55, 55, 225);
        dl->AddRectFilled(ImVec2(gx + i * bar_w, by),
                          ImVec2(gx + (i + 1) * bar_w - 1.0f, gy + gh), col, 1.0f);
    }
    dl->AddRect(ImVec2(gx, gy), ImVec2(gx + gw, gy + gh),
                IM_COL32(70, 75, 90, 170), 3.0f * s, 0, 1.0f * s);
    y += gh + gap;

    // RAM on the final row; no longer collides with the graph or its label.
    if (mem_buf[0] != '\0') {
        draw_text(small, ImVec2(x, y), IM_COL32(150, 175, 215, 220), mem_buf);
    }
}

void MenuSystem::RenderVirtualGamepad() {
    if (!GetConfig().virtual_gamepad_enabled) return;
    GamepadManager& gp = GetGamepadManager();
    if (!gp.IsVirtualGamepadVisible() || gp.IsPhysicalConnected()) return;

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const VirtualPadLayout L = gp.GetLayout();
    const auto& state = gp.GetState();
    ImFont* font = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;
    const float k = L.face_r / 62.0f;

    // Xbox-style palette; every control uses the same shared layout as hit-test.
    const ImU32 col_A = IM_COL32(0, 179, 68, 235),   col_Ap = IM_COL32(60, 235, 110, 255);
    const ImU32 col_B = IM_COL32(230, 0, 35, 235),   col_Bp = IM_COL32(255, 55, 80, 255);
    const ImU32 col_X = IM_COL32(0, 90, 230, 235),   col_Xp = IM_COL32(60, 140, 255, 255);
    const ImU32 col_Y = IM_COL32(255, 185, 0, 235),  col_Yp = IM_COL32(255, 215, 60, 255);
    const ImU32 col_dark = IM_COL32(16, 18, 24, 230);
    const ImU32 col_panel = IM_COL32(28, 30, 38, 240);
    const ImU32 col_ring = IM_COL32(115, 120, 138, 255);
    const ImU32 col_text = IM_COL32(240, 242, 248, 255);
    const ImU32 col_dim = IM_COL32(175, 180, 195, 235);
    const ImU32 col_shadow = IM_COL32(0, 0, 0, 115);
    const ImU32 col_green = IM_COL32(0, 179, 68, 255);

    auto P = [](VirtualPadLayout::Point p) { return ImVec2(p.x, p.y); };

    auto text = [&](const char* s, float x, float y, float size, ImU32 col) {
        ImVec2 ts = font ? font->CalcTextSizeA(size, 9999.0f, -1.0f, s)
                         : ImVec2(strlen(s) * size * 0.6f, size);
        dl->AddText(font, size, ImVec2(x - ts.x * 0.5f, y - ts.y * 0.55f), col, s);
    };

    // Rounded Xbox face button with letter, gloss and pressed feedback.
    auto face_button = [&](VirtualPadLayout::Point c, float r, ImU32 base, ImU32 pressed_col,
                           const char* label, bool pressed) {
        const float rr = pressed ? r * 0.92f : r;
        dl->AddCircleFilled(ImVec2(c.x + 3 * k, c.y + 4 * k), rr + 3 * k, col_shadow, 32);
        dl->AddCircleFilled(ImVec2(c.x, c.y), rr, pressed ? pressed_col : base, 32);
        dl->AddCircleFilled(ImVec2(c.x, c.y - rr * 0.22f), rr * 0.72f,
                            IM_COL32(255, 255, 255, pressed ? 40 : 70), 32);
        dl->AddCircle(ImVec2(c.x, c.y), rr, IM_COL32(0, 0, 0, 130), 32, 2.0f * k);
        text(label, c.x, c.y, 36.0f * k, col_text);
    };

    // Small rounded button (bumpers, View, Menu).
    auto small_button = [&](VirtualPadLayout::Point c, float w, float h, const char* label,
                            bool pressed, float label_size = 21.0f) {
        ImVec2 a(c.x - w * 0.5f, c.y - h * 0.5f), b(c.x + w * 0.5f, c.y + h * 0.5f);
        dl->AddRectFilled(ImVec2(a.x + 2 * k, a.y + 3 * k), ImVec2(b.x + 2 * k, b.y + 3 * k), col_shadow, 12 * k);
        dl->AddRectFilled(a, b, pressed ? col_green : col_panel, 12 * k);
        dl->AddRectFilled(ImVec2(a.x + 5 * k, a.y + 3 * k), ImVec2(b.x - 5 * k, c.y),
                          IM_COL32(255, 255, 255, pressed ? 35 : 50), 10 * k);
        dl->AddRect(a, b, pressed ? col_green : col_ring, 12 * k, 0, 2.0f * k);
        text(label, c.x, c.y, label_size * k, pressed ? col_text : col_dim);
    };

    // Trigger: rounded bar with a live fill level from the analog value.
    auto trigger = [&](VirtualPadLayout::Point c, float w, float h, const char* label,
                       float fill, bool pressed) {
        ImVec2 a(c.x - w * 0.5f, c.y - h * 0.5f), b(c.x + w * 0.5f, c.y + h * 0.5f);
        dl->AddRectFilled(ImVec2(a.x + 3 * k, a.y + 4 * k), ImVec2(b.x + 3 * k, b.y + 4 * k), col_shadow, 14 * k);
        dl->AddRectFilled(a, b, pressed ? col_green : col_dark, 14 * k);
        if (fill > 0.01f) {
            const float top = b.y - 5 * k - (h - 10 * k) * fill;
            dl->AddRectFilled(ImVec2(a.x + 5 * k, top), ImVec2(b.x - 5 * k, b.y - 5 * k),
                              IM_COL32(0, 179, 68, 200), 10 * k);
        }
        dl->AddRect(a, b, col_ring, 14 * k, 0, 2.0f * k);
        text(label, c.x, c.y, 23.0f * k, col_text);
    };

    // D-pad arm: same centre/radius the hit-test uses.
    auto dpad_arm = [&](float x, float y, float w, float h, const char* glyph, bool pressed) {
        ImVec2 a(x - w * 0.5f, y - h * 0.5f), b(x + w * 0.5f, y + h * 0.5f);
        dl->AddRectFilled(ImVec2(a.x + 2 * k, a.y + 3 * k), ImVec2(b.x + 2 * k, b.y + 3 * k), col_shadow, 10 * k);
        dl->AddRectFilled(a, b, pressed ? col_green : col_panel, 10 * k);
        dl->AddRect(a, b, pressed ? col_green : col_ring, 10 * k, 0, 2.0f * k);
        text(glyph, x, y, 26.0f * k, pressed ? col_text : col_dim);
    };

    auto stick = [&](VirtualPadLayout::Point c, float ax, float ay) {
        const ImVec2 cp = P(c);
        dl->AddCircleFilled(cp, L.stick_r + 10 * k, col_shadow, 48);
        dl->AddCircleFilled(cp, L.stick_r + 6 * k, col_dark, 48);
        dl->AddCircle(cp, L.stick_r + 6 * k, col_ring, 48, 2.0f * k);
        dl->AddCircle(cp, L.stick_r * 0.62f, IM_COL32(70, 75, 90, 170), 40, 1.5f * k);
        dl->AddCircle(cp, L.stick_r * 0.22f, IM_COL32(95, 100, 115, 200), 32, 1.0f * k);
        ImVec2 knob(c.x + ax * L.stick_r * 0.85f, c.y - ay * L.stick_r * 0.85f);
        dl->AddCircleFilled(ImVec2(knob.x + 2 * k, knob.y + 3 * k), 30 * k, col_shadow, 32);
        dl->AddCircleFilled(knob, 27 * k, IM_COL32(210, 212, 220, 250), 32);
        dl->AddCircleFilled(ImVec2(knob.x - 4 * k, knob.y - 5 * k), 15 * k, IM_COL32(160, 164, 176, 240), 32);
        dl->AddCircle(knob, 27 * k, IM_COL32(120, 124, 138, 220), 32, 2.0f * k);
    };

    // Backing plates lift the controls off bright game frames.
    dl->AddCircleFilled(P(L.left_stick), L.stick_r * 1.35f, IM_COL32(0, 0, 0, 45), 48);
    dl->AddCircleFilled(P(L.right_stick), L.stick_r * 1.35f, IM_COL32(0, 0, 0, 45), 48);
    dl->AddCircleFilled(P(L.dpad), L.dpad_r * 3.0f, IM_COL32(0, 0, 0, 45), 40);
    dl->AddCircleFilled(P(L.face), L.face_r * 3.4f, IM_COL32(0, 0, 0, 45), 40);

    // Triggers / bumpers.
    trigger(L.lt, L.trigger_w, L.trigger_h, "LT", state.left_trigger, state.left_trigger > 0.5f);
    trigger(L.rt, L.trigger_w, L.trigger_h, "RT", state.right_trigger, state.right_trigger > 0.5f);
    small_button(L.lb, L.bumper_w, L.bumper_h, "LB", state.lb);
    small_button(L.rb, L.bumper_w, L.bumper_h, "RB", state.rb);

    // Xbox Guide and View/Menu cluster.
    dl->AddCircleFilled(P(L.guide), L.guide_r + 8 * k, IM_COL32(0, 179, 68, 75), 40);
    dl->AddCircleFilled(P(L.guide), L.guide_r, state.guide ? col_green : col_dark, 40);
    dl->AddCircle(P(L.guide), L.guide_r, col_green, 40, 3.0f * k);
    const float gs = 11 * k;
    dl->AddLine(ImVec2(L.guide.x - gs, L.guide.y - gs), ImVec2(L.guide.x + gs, L.guide.y + gs), col_green, 4 * k);
    dl->AddLine(ImVec2(L.guide.x - gs, L.guide.y + gs), ImVec2(L.guide.x + gs, L.guide.y - gs), col_green, 4 * k);
    small_button(L.view, L.vm_w, L.vm_h, "VIEW", state.back, 18.0f);
    small_button(L.menu, L.vm_w, L.vm_h, "MENU", state.start, 18.0f);

    // Analog sticks with live knob position.
    stick(L.left_stick, state.left_x, state.left_y);
    stick(L.right_stick, state.right_x, state.right_y);

    // D-pad cross: four large arms, pressed arm lights up green.
    const float dr = L.dpad_r, dg = L.dpad_r * L.dpad_gap;
    dpad_arm(L.dpad.x, L.dpad.y - dg, dr * 1.7f, dr * 2.0f, "^", state.dpad_y > 0);
    dpad_arm(L.dpad.x, L.dpad.y + dg, dr * 1.7f, dr * 2.0f, "v", state.dpad_y < 0);
    dpad_arm(L.dpad.x - dg, L.dpad.y, dr * 2.0f, dr * 1.7f, "<", state.dpad_x < 0);
    dpad_arm(L.dpad.x + dg, L.dpad.y, dr * 2.0f, dr * 1.7f, ">", state.dpad_x > 0);

    // Face buttons: A bottom (green), B right (red), X left (blue), Y top (yellow).
    const float fg = L.face_r * L.face_gap;
    face_button({L.face.x, L.face.y - fg}, L.face_r, col_Y, col_Yp, "Y", state.y);
    face_button({L.face.x + fg, L.face.y}, L.face_r, col_B, col_Bp, "B", state.b);
    face_button({L.face.x, L.face.y + fg}, L.face_r, col_A, col_Ap, "A", state.a);
    face_button({L.face.x - fg, L.face.y}, L.face_r, col_X, col_Xp, "X", state.x);
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