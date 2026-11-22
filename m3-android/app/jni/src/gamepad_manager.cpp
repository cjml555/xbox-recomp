// MCLA - Gamepad Manager Implementation
#include "gamepad_manager.h"
#include <SDL3/SDL_events.h>
#include <android/log.h>
#include <algorithm>
#include <cmath>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MCLA_GPAD", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MCLA_GPAD", __VA_ARGS__)

namespace mcla {

GamepadManager::GamepadManager() {
    m_current_state.type = GamepadType::Virtual;
    m_virtual_gamepad_visible = true;
    SetDisplaySize(1, 1);
}

void GamepadManager::Initialize() {
    if (m_initialized) return;
    SDL_Log("GamepadManager initialized");
    m_initialized = true;
}

void GamepadManager::Shutdown() {
    SDL_Log("GamepadManager shutdown");
    m_initialized = false;
}

void GamepadManager::SetDisplaySize(int w, int h) {
    m_display_width = w > 0 ? w : 2400;
    m_display_height = h > 0 ? h : 1080;
}

void GamepadManager::Update() {
    ClearState();
    static int ucount = 0;
    if (++ucount % 30 == 0)
      __android_log_print(ANDROID_LOG_INFO, "MCLA_UPDATE", "Update #%d", ucount);

    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);

    if (joysticks && count > 0) {
        SDL_Gamepad* gp = SDL_GetGamepadFromID(joysticks[0]);
        if (gp) {
            UpdatePhysicalState(gp, m_current_state);
            m_current_state.type = GamepadType::Physical;
            m_current_state.connected = true;
            return;
        }
    }

    if (!m_current_state.connected) {
        m_current_state.type = GamepadType::Virtual;
        UpdateVirtualState();
        m_current_state.connected = true;
    }
}

void GamepadManager::ClearState() {
    // Only clear the per-frame recomputed analog/directional inputs. Button
    // states are latched persistently by FeedTouch so the guest always sees a
    // stable press even though Update() runs ~30 Hz while the guest polls at
    // kHz rates. Clearing buttons here was the bug: they flickered on/off.
    m_current_state.left_x = 0.0f;
    m_current_state.left_y = 0.0f;
    m_current_state.right_x = 0.0f;
    m_current_state.right_y = 0.0f;
    m_current_state.left_trigger = 0.0f;
    m_current_state.right_trigger = 0.0f;
    m_current_state.dpad_x = 0;
    m_current_state.dpad_y = 0;
    m_current_state.type = GamepadType::Unknown;
    m_current_state.connected = false;
}

// Map a virtual zone to its persistent button flag and set it on/off. Returns
// true if the zone is a latching button (so callers know a touch "owns" it).
static bool SetZoneButton(GamepadState& s, VirtualZone z, bool on) {
    switch (z) {
    case VirtualZone::A:       s.a = on; return true;
    case VirtualZone::B:       s.b = on; return true;
    case VirtualZone::X:       s.x = on; return true;
    case VirtualZone::Y:       s.y = on; return true;
    case VirtualZone::LB:      s.lb = on; return true;
    case VirtualZone::RB:      s.rb = on; return true;
    case VirtualZone::Guide:   s.guide = on; return true;
    case VirtualZone::View:    s.back = on; return true;
    case VirtualZone::Menu:    s.start = on; return true;
    case VirtualZone::LT:      s.left_trigger  = on ? 1.0f : 0.0f; return true;
    case VirtualZone::RT:      s.right_trigger = on ? 1.0f : 0.0f; return true;
    default: return false;
    }
}

void GamepadManager::FeedTouch(int pointer_id, bool down, float x, float y) {
    __android_log_print(ANDROID_LOG_INFO, "MCLA_TOUCH", "feed id=%d down=%d x=%.0f y=%.0f",
                        pointer_id, down ? 1 : 0, x, y);
    int slot = -1;
    for (int i = 0; i < (int)m_touches.size(); ++i) {
        if (m_touches[i].pointer_id == pointer_id) { slot = i; break; }
    }
    if (slot < 0) {
        for (int i = 0; i < (int)m_touches.size(); ++i) {
            if (m_touches[i].pointer_id == -1) { slot = i; break; }
        }
    }
    if (slot < 0) {
        slot = 0;
        m_touches[slot].pointer_id = -1;
    }
    VirtualTouch& t = m_touches[slot];

    if (down) {
        // Releasing any button this slot previously owned, in case it moved.
        if (t.zone != VirtualZone::None)
            SetZoneButton(m_current_state, t.zone, false);
        t.pointer_id = pointer_id;
        t.x = x;
        t.y = y;
        t.down = true;
        VirtualZone z = HitTest(x, y);
        t.zone = z;
        if (z != VirtualZone::None && z != VirtualZone::LeftStick && z != VirtualZone::RightStick)
            __android_log_print(ANDROID_LOG_INFO, "MCLA_TOUCH",
                                "down zone=%d t=(%.0f,%.0f)", (int)z, x, y);
        // Latch the button immediately — persists until this pointer lifts.
        SetZoneButton(m_current_state, z, true);
    } else {
        // Lift: release the button this slot owned.
        if (t.zone != VirtualZone::None)
            SetZoneButton(m_current_state, t.zone, false);
        t.pointer_id = -1;
        t.x = 0.f; t.y = 0.f;
        t.down = false;
        t.zone = VirtualZone::None;
    }
}

VirtualZone GamepadManager::HitTest(float sx, float sy) const {
    static int htc = 0;
    if (htc < 6) { htc++;
      __android_log_print(ANDROID_LOG_INFO, "MCLA_TOUCH",
                          "hit x=%.0f y=%.0f vis=%d dw=%d dh=%d", sx, sy,
                          m_virtual_gamepad_visible ? 1 : 0, m_display_width,
                          m_display_height);
    }
    if (!m_virtual_gamepad_visible || m_display_width <= 0 || m_display_height <= 0)
        return VirtualZone::None;

    // ===== XBOX ASYMMETRIC LAYOUT =====
    // Left stick TOP-LEFT (17%, 65%), D-pad BOTTOM-LEFT
    const float lcx = m_display_width  * 0.17f;
    const float lcy = m_display_height * 0.65f;
    const float R   = GetStickRadius();

    // Right stick BOTTOM-RIGHT (83%, 73%) — lower than face buttons
    const float rcx = m_display_width  * 0.83f;
    const float rcy = m_display_height * 0.73f;

    // Face button cluster center (right side, above right stick)
    const float bdx = m_display_width  * 0.87f;
    const float bdy = m_display_height * 0.58f;
    const float br  = 58.f;

    // D-pad (centered below left stick)
    const float dpx = lcx;
    const float dpy = lcy + R + 45.f;
    const float dr  = 38.f;

    // Triggers (top edge)
    const float lt_x = m_display_width * 0.12f, lt_y = 24.f;
    const float rt_x = m_display_width * 0.88f, rt_y = 24.f;
    // Bumpers (below triggers)
    const float lb_x = lt_x, lb_y = lt_y + 55.f + 8.f;
    const float rb_x = rt_x, rb_y = lb_y;

    // Guide button (top-center)
    const float gx = m_display_width * 0.5f;
    const float gy = 87.f;

    // View / Menu buttons
    const float view_x = gx - 85.f, menu_x = gx + 85.f;
    const float vm_y = gy + 5.f;

    auto pt_in_circ = [](float px, float py, float cx, float cy, float r) -> bool {
        return std::hypot(px - cx, py - cy) <= r;
    };

    // Face buttons — diamond layout (A bottom, B right, X left, Y top)
    // Offsets must match the spacing used in RenderVirtualGamepad (btn_R * 2.15f).
    if (pt_in_circ(sx, sy, bdx,                  bdy - br * 2.15f, br)) return VirtualZone::Y;
    if (pt_in_circ(sx, sy, bdx + br * 2.15f,     bdy,                         br)) return VirtualZone::B;
    if (pt_in_circ(sx, sy, bdx,                  bdy + br * 2.15f, br)) return VirtualZone::A;
    if (pt_in_circ(sx, sy, bdx - br * 2.15f,     bdy,                         br)) return VirtualZone::X;

    // D-pad (cross BELOW left stick)
    if (pt_in_circ(sx, sy, dpx, dpy - dr * 2.1f, dr * 0.8f)) return VirtualZone::DPadUp;
    if (pt_in_circ(sx, sy, dpx, dpy + dr * 2.1f, dr * 0.8f)) return VirtualZone::DPadDown;
    if (pt_in_circ(sx, sy, dpx - dr * 2.1f, dpy, dr * 0.8f)) return VirtualZone::DPadLeft;
    if (pt_in_circ(sx, sy, dpx + dr * 2.1f, dpy, dr * 0.8f)) return VirtualZone::DPadRight;

    // Left stick — TOP-LEFT
    if (pt_in_circ(sx, sy, lcx, lcy, R * 1.12f)) return VirtualZone::LeftStick;
    // Right stick — BOTTOM-RIGHT
    if (pt_in_circ(sx, sy, rcx, rcy, R * 1.12f)) return VirtualZone::RightStick;

    // Triggers (LT / RT)
    if (pt_in_circ(sx, sy, lt_x, lt_y, 52.f)) return VirtualZone::LT;
    if (pt_in_circ(sx, sy, rt_x, rt_y, 52.f)) return VirtualZone::RT;
    // Bumpers (LB / RB)
    if (pt_in_circ(sx, sy, lb_x, lb_y, 55.f)) return VirtualZone::LB;
    if (pt_in_circ(sx, sy, rb_x, rb_y, 55.f)) return VirtualZone::RB;

    // Guide button
    if (pt_in_circ(sx, sy, gx, gy, 30.f)) return VirtualZone::Guide;
    // View (left) and Menu (right)
    if ((sx >= view_x - 35.f && sx <= view_x + 35.f) && (sy >= vm_y - 19.f && sy <= vm_y + 19.f))
      return VirtualZone::View;
    if ((sx >= menu_x - 35.f && sx <= menu_x + 35.f) && (sy >= vm_y - 19.f && sy <= vm_y + 19.f))
      return VirtualZone::Menu;

    return VirtualZone::None;
}

void GamepadManager::UpdateVirtualState() {
    const float R = GetStickRadius();
    const float lcx = m_display_width  * 0.17f;
    const float lcy = m_display_height * 0.65f;
    const float rcx = m_display_width  * 0.83f;
    const float rcy = m_display_height * 0.73f;

    // Reset directional inputs each frame
    m_current_state.dpad_x = 0;
    m_current_state.dpad_y = 0;

    for (const auto& t : m_touches) {
        if (t.pointer_id < 0 || !t.down) continue;
        float tx = t.x, ty = t.y;

        VirtualZone z = HitTest(tx, ty);
        if (z == VirtualZone::None) continue;
        if (z != VirtualZone::LeftStick && z != VirtualZone::RightStick)
          __android_log_print(ANDROID_LOG_INFO, "MCLA_TOUCH",
                              "zone=%d t=(%.0f,%.0f)", (int)z, tx, ty);

        switch (z) {
        case VirtualZone::LeftStick: {
            float dx = (tx - lcx) / R;
            float dy = (ty - lcy) / R;
            dx = std::clamp(dx, -1.f, 1.f);
            dy = std::clamp(dy, -1.f, 1.f);
            m_current_state.left_x  = dx;
            m_current_state.left_y  = -dy;
            break;
        }
        case VirtualZone::RightStick: {
            float dx = (tx - rcx) / R;
            float dy = (ty - rcy) / R;
            dx = std::clamp(dx, -1.f, 1.f);
            dy = std::clamp(dy, -1.f, 1.f);
            m_current_state.right_x  = dx;
            m_current_state.right_y  = -dy;
            break;
        }
        case VirtualZone::A:       m_current_state.a = true;  break;
        case VirtualZone::B:       m_current_state.b = true;  break;
        case VirtualZone::X:       m_current_state.x = true;  break;
        case VirtualZone::Y:       m_current_state.y = true;  break;
        case VirtualZone::LB:      m_current_state.lb = true; break;
        case VirtualZone::RB:      m_current_state.rb = true; break;
        case VirtualZone::LT:      m_current_state.left_trigger  = 1.0f; break;
        case VirtualZone::RT:      m_current_state.right_trigger = 1.0f; break;
        case VirtualZone::Guide:   m_current_state.guide = true; break;
        case VirtualZone::View:    m_current_state.back = true; break;
        case VirtualZone::Menu:    m_current_state.start = true; break;
        case VirtualZone::DPadUp:      m_current_state.dpad_y =  1; break;
        case VirtualZone::DPadDown:    m_current_state.dpad_y = -1; break;
        case VirtualZone::DPadLeft:    m_current_state.dpad_x = -1; break;
        case VirtualZone::DPadRight:   m_current_state.dpad_x =  1; break;
        default: break;
        }
    }
}

void GamepadManager::UpdatePhysicalState(SDL_Gamepad* gamepad, GamepadState& state) {
    if (!gamepad) return;

    int16_t lx = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    int16_t ly = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    state.left_x  = lx / 32767.0f;
    state.left_y  = -ly / 32767.0f;

    int16_t rx = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    int16_t ry = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    state.right_x = rx / 32767.0f;
    state.right_y = -ry / 32767.0f;

    int16_t lt_raw = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    int16_t rt_raw = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
    state.left_trigger  = std::max(0.0f, static_cast<float>(lt_raw) / 32767.0f);
    state.right_trigger = std::max(0.0f, static_cast<float>(rt_raw) / 32767.0f);

    state.a    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    state.b    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
    state.x    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST);
    state.y    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH);
    state.lb   = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    state.rb   = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    state.back = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK);
    state.start= SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START);
    state.guide= SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
    state.lstick=SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
    state.rstick=SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);

    if (std::fabs(state.left_x) > 0.7f) state.dpad_x = state.left_x > 0 ? 1 : -1;
    if (std::fabs(state.left_y) > 0.7f) state.dpad_y = state.left_y > 0 ? -1 : 1;
}

void GamepadManager::ToggleVirtualGamepad() {
    m_virtual_gamepad_visible = !m_virtual_gamepad_visible;
    LOGI("Virtual gamepad %s", m_virtual_gamepad_visible ? "VISIBLE" : "hidden");
}

void GamepadManager::SetVirtualGamepadVisible(bool visible) {
    m_virtual_gamepad_visible = visible;
}

bool GamepadManager::IsPressed(SDL_Scancode key) const {
    switch (key) {
        case SDL_SCANCODE_W: return m_current_state.left_y > 0.5f;
        case SDL_SCANCODE_S: return m_current_state.left_y < -0.5f;
        case SDL_SCANCODE_A: return m_current_state.left_x < -0.5f;
        case SDL_SCANCODE_D: return m_current_state.left_x > 0.5f;
        case SDL_SCANCODE_UP:    return m_current_state.dpad_y > 0;
        case SDL_SCANCODE_DOWN:  return m_current_state.dpad_y < 0;
        case SDL_SCANCODE_LEFT:  return m_current_state.dpad_x < 0;
        case SDL_SCANCODE_RIGHT: return m_current_state.dpad_x > 0;
        default: return false;
    }
}

void GamepadManager::SetVibration(float left, float right) {
    m_current_state.left_motor  = left;
    m_current_state.right_motor = right;

    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);
    if (joysticks && count > 0) {
        SDL_Gamepad* gp = SDL_GetGamepadFromID(joysticks[0]);
        if (gp) {
            SDL_RumbleGamepad(gp,
                static_cast<uint16_t>(left  * 0xFFFF),
                static_cast<uint16_t>(right * 0xFFFF),
                1000);
        }
    }
}

GamepadManager& GetGamepadManager() {
    static GamepadManager instance;
    return instance;
}

void RegisterMclaInputDriver(rex::input::InputSystem* system) {
    if (!system) return;
    system->AddDriver(std::make_unique<MclaInputDriver>());
    LOGI("Registered MCLA virtual gamepad input driver");
}

} // namespace mcla