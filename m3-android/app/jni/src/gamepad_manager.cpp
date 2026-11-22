// MCLA - Gamepad Manager Implementation
#include "gamepad_manager.h"
#include "config.h"
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
    FlushExpiredReleases();
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

static bool IsLatchZone(VirtualZone z);
static bool SetZoneButton(GamepadState& s, VirtualZone z, bool on);

int64_t GamepadManager::NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

void GamepadManager::FlushExpiredReleases() {
    const int64_t now = NowMs();
    for (size_t i = 0; i < kZoneCount; ++i) {
        int64_t& deadline = m_release_at_ms[i];
        if (!deadline || now < deadline) continue;
        bool still_held = false;
        for (const auto& t : m_touches) {
            if (t.down && static_cast<size_t>(t.zone) == i) {
                still_held = true;
                break;
            }
        }
        if (!still_held) {
            SetZoneButton(m_current_state, static_cast<VirtualZone>(i), false);
        }
        deadline = 0;
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

static bool IsLatchZone(VirtualZone z) {
    return z >= VirtualZone::A && z <= VirtualZone::Menu;
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
        if (t.zone != VirtualZone::None) {
            if (IsLatchZone(t.zone)) {
                int64_t& prev_deadline = m_release_at_ms[static_cast<size_t>(t.zone)];
                if (!prev_deadline) prev_deadline = NowMs() + kMinPressMs;
            } else {
                SetZoneButton(m_current_state, t.zone, false);
            }
        }
        t.pointer_id = pointer_id;
        t.x = x;
        t.y = y;
        t.down = true;
        VirtualZone z = HitTest(x, y);
        t.zone = z;
        if (IsLatchZone(z)) {
            m_release_at_ms[static_cast<size_t>(z)] = 0;  // cancel a pending auto-release
        }
        if (z != VirtualZone::None && z != VirtualZone::LeftStick && z != VirtualZone::RightStick)
            __android_log_print(ANDROID_LOG_INFO, "MCLA_TOUCH",
                                "down zone=%d t=(%.0f,%.0f)", (int)z, x, y);
        // Latch the button immediately — persists until this pointer lifts.
        SetZoneButton(m_current_state, z, true);
    } else {
        // Lift: keep latching buttons down for at least kMinPressMs so the
        // guest (which polls slowly under load) cannot miss a quick tap.
        if (IsLatchZone(t.zone)) {
            int64_t& deadline = m_release_at_ms[static_cast<size_t>(t.zone)];
            if (!deadline) deadline = NowMs() + kMinPressMs;
        } else if (t.zone != VirtualZone::None) {
            SetZoneButton(m_current_state, t.zone, false);
        }
        t.pointer_id = -1;
        t.x = 0.f; t.y = 0.f;
        t.down = false;
        t.zone = VirtualZone::None;
    }
}

VirtualPadLayout VirtualPadLayout::Make(int width, int height) {
    const VirtualPadConfig defaults;
    return Make(width, height, defaults);
}

VirtualPadLayout VirtualPadLayout::Make(int width, int height, const VirtualPadConfig& cfg) {
    VirtualPadLayout L;
    const float w = width  > 0 ? static_cast<float>(width)  : 2400.0f;
    const float h = height > 0 ? static_cast<float>(height) : 1080.0f;
    const float k = std::clamp(std::min(w / 2400.0f, h / 1080.0f), 0.65f, 1.30f);

    // Clamp again here so the draw and hit-test are safe even for config
    // written by an older build or edited by hand on the device.
    const auto scale = [](float v) { return std::clamp(v, 0.6f, 1.8f); };
    L.stick_r = 96.0f * k * scale(cfg.stick_scale);
    L.stick_hit_r = L.stick_r * (112.0f / 96.0f);
    L.face_r  = 62.0f * k * scale(cfg.face_scale);
    L.face_gap = 2.10f;
    L.dpad_r  = 44.0f * k * scale(cfg.dpad_scale);
    L.dpad_gap = 2.05f;
    L.dpad_hit_r = L.dpad_r * (34.0f / 44.0f);
    L.trigger_w = 130.0f * k;   L.trigger_h = 62.0f * k;
    L.bumper_w  = 140.0f * k;   L.bumper_h  = 54.0f * k;
    L.guide_r   = 34.0f * k;
    L.vm_w = 84.0f * k;         L.vm_h = 44.0f * k;

    const float top = 26.0f * k;
    L.lt = {w * 0.14f, top + L.trigger_h * 0.5f};
    L.rt = {w * 0.86f, top + L.trigger_h * 0.5f};
    L.lb = {w * 0.14f, top + L.trigger_h + 12.0f * k + L.bumper_h * 0.5f};
    L.rb = {w * 0.86f, L.lb.y};
    L.guide = {w * 0.5f, L.lb.y + L.bumper_h * 0.5f + 16.0f * k + L.guide_r};
    L.view = {L.guide.x - 112.0f * k, L.guide.y};
    L.menu = {L.guide.x + 112.0f * k, L.guide.y};

    // Asymmetric Xbox layout: stick top-left, D-pad below it; face buttons
    // high on the right, right stick low on the right so they never overlap.
    L.left_stick  = {w * 0.18f, h * 0.62f};
    L.right_stick = {w * 0.80f, h * 0.72f};
    L.face        = {w * 0.90f, h * 0.52f};
    L.dpad        = {w * 0.18f, L.left_stick.y + L.stick_r + 100.0f * k};

    // User offsets are applied last and equally to drawing and hit-testing.
    const auto off = [](float v) { return std::clamp(v, -600.0f, 600.0f); };
    L.left_stick.x  += off(cfg.stick_left_x);  L.left_stick.y  += off(cfg.stick_left_y);
    L.right_stick.x += off(cfg.stick_right_x); L.right_stick.y += off(cfg.stick_right_y);
    L.face.x += off(cfg.face_x);               L.face.y += off(cfg.face_y);
    L.dpad.x += off(cfg.dpad_x);               L.dpad.y += off(cfg.dpad_y);
    return L;
}

VirtualPadLayout GamepadManager::GetLayout() const {
    return VirtualPadLayout::Make(m_display_width, m_display_height, GetConfig().pad);
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
    // The virtual pad only exists while no physical controller is detected.
    if (m_current_state.type == GamepadType::Physical) return VirtualZone::None;

    const VirtualPadLayout L = GetLayout();
    auto in_circ = [](float px, float py, VirtualPadLayout::Point c, float r) {
        return std::hypot(px - c.x, py - c.y) <= r;
    };
    auto in_rect = [](float px, float py, VirtualPadLayout::Point c, float w, float h) {
        return std::fabs(px - c.x) <= w * 0.5f && std::fabs(py - c.y) <= h * 0.5f;
    };

    // Face buttons first: they are drawn on top of every other control.
    const float fr = L.face_r, fg = L.face_r * L.face_gap;
    if (in_circ(sx, sy, {L.face.x, L.face.y - fg}, fr)) return VirtualZone::Y;
    if (in_circ(sx, sy, {L.face.x + fg, L.face.y}, fr)) return VirtualZone::B;
    if (in_circ(sx, sy, {L.face.x, L.face.y + fg}, fr)) return VirtualZone::A;
    if (in_circ(sx, sy, {L.face.x - fg, L.face.y}, fr)) return VirtualZone::X;

    // D-pad arms (same centres and radii used when drawing).
    const float dg = L.dpad_r * L.dpad_gap, dr = L.dpad_hit_r;
    if (in_circ(sx, sy, {L.dpad.x, L.dpad.y - dg}, dr)) return VirtualZone::DPadUp;
    if (in_circ(sx, sy, {L.dpad.x, L.dpad.y + dg}, dr)) return VirtualZone::DPadDown;
    if (in_circ(sx, sy, {L.dpad.x - dg, L.dpad.y}, dr)) return VirtualZone::DPadLeft;
    if (in_circ(sx, sy, {L.dpad.x + dg, L.dpad.y}, dr)) return VirtualZone::DPadRight;

    // Sticks.
    if (in_circ(sx, sy, L.left_stick, L.stick_hit_r))  return VirtualZone::LeftStick;
    if (in_circ(sx, sy, L.right_stick, L.stick_hit_r)) return VirtualZone::RightStick;

    // Triggers and bumpers are rectangles: hit-test the actual drawn shape.
    if (in_rect(sx, sy, L.lt, L.trigger_w, L.trigger_h)) return VirtualZone::LT;
    if (in_rect(sx, sy, L.rt, L.trigger_w, L.trigger_h)) return VirtualZone::RT;
    if (in_rect(sx, sy, L.lb, L.bumper_w, L.bumper_h)) return VirtualZone::LB;
    if (in_rect(sx, sy, L.rb, L.bumper_w, L.bumper_h)) return VirtualZone::RB;

    if (in_circ(sx, sy, L.guide, L.guide_r)) return VirtualZone::Guide;
    if (in_rect(sx, sy, L.view, L.vm_w, L.vm_h)) return VirtualZone::View;
    if (in_rect(sx, sy, L.menu, L.vm_w, L.vm_h)) return VirtualZone::Menu;

    return VirtualZone::None;
}

void GamepadManager::UpdateVirtualState() {
    const VirtualPadLayout L = GetLayout();

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
            float dx = (tx - L.left_stick.x) / L.stick_r;
            float dy = (ty - L.left_stick.y) / L.stick_r;
            dx = std::clamp(dx, -1.f, 1.f);
            dy = std::clamp(dy, -1.f, 1.f);
            m_current_state.left_x  = dx;
            m_current_state.left_y  = -dy;
            break;
        }
        case VirtualZone::RightStick: {
            float dx = (tx - L.right_stick.x) / L.stick_r;
            float dy = (ty - L.right_stick.y) / L.stick_r;
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