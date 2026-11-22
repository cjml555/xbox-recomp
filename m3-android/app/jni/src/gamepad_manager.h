// MCLA - Gamepad Manager Header
// Manages physical and virtual gamepad support for Android

#pragma once

#include <android/log.h>
#include <SDL3/SDL.h>
#include <array>
#include <chrono>
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

#include <rex/input/input_driver.h>
#include <rex/input/input_system.h>
#include <rex/system/xtypes.h>

namespace mcla {

struct VirtualPadConfig;

using rex::X_RESULT;
using rex::X_STATUS;

enum class GamepadType {
    Unknown,
    Physical,
    Virtual
};

struct GamepadState {
    GamepadType type = GamepadType::Unknown;
    bool connected = false;

    // Analog sticks (-1.0 to 1.0)
    float left_x = 0.0f;
    float left_y = 0.0f;
    float right_x = 0.0f;
    float right_y = 0.0f;

    // Triggers (0.0 to 1.0)
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;

    // D-pad
    int dpad_x = 0;
    int dpad_y = 0;

    // Buttons
    bool a = false;
    bool b = false;
    bool x = false;
    bool y = false;
    bool lb = false;
    bool rb = false;
    bool back = false;
    bool start = false;
    bool guide = false;
    bool lstick = false;
    bool rstick = false;

    // Rumble
    float left_motor = 0.0f;
    float right_motor = 0.0f;
};

// Single source of truth for the virtual pad geometry. Both HitTest() and
// MenuSystem::RenderVirtualGamepad() use this so drawn controls and touch
// zones can never drift apart. Coordinates are surface pixels.
struct VirtualPadLayout {
    struct Point { float x = 0.0f, y = 0.0f; };
    Point left_stick, right_stick, face, dpad, lt, rt, lb, rb, guide, view, menu;

    float stick_r = 96.0f;        // drawn base radius
    float stick_hit_r = 112.0f;   // touch radius
    float face_r = 62.0f;         // face button radius
    float face_gap = 2.1f;        // multiplier of face_r
    float dpad_r = 44.0f;         // arm half-size
    float dpad_gap = 2.05f;       // multiplier of dpad_r
    float dpad_hit_r = 34.0f;
    float trigger_w = 130.0f, trigger_h = 62.0f;
    float bumper_w = 140.0f, bumper_h = 54.0f;
    float guide_r = 34.0f;
    float vm_w = 84.0f, vm_h = 44.0f;

    // Build for a given surface size, scaled from the 2400x1080 design and
    // with the user's live size/position overrides applied.
    static VirtualPadLayout Make(int width, int height, const VirtualPadConfig& cfg);
    static VirtualPadLayout Make(int width, int height);
};

// Logical control zones on the virtual gamepad (in surface-pixel space).
enum class VirtualZone {
    None,
    LeftStick,      // top-left drag region -> left analog
    RightStick,     // bottom-right drag region -> right analog
    A, B, X, Y,     // face buttons (diamond, right side)
    LB, RB,         // bumpers
    LT, RT,         // triggers
    Guide,          // Xbox guide button
    View,           // View / Back button
    Menu,           // Menu / Start button
    DPadUp, DPadDown, DPadLeft, DPadRight,
};

// A single active virtual-gamepad touch point (surface-pixel space).
struct VirtualTouch {
    int pointer_id = -1;   // -1 = inactive
    float x = 0.0f;
    float y = 0.0f;
    bool down = false;
    VirtualZone zone = VirtualZone::None;  // zone the pointer currently maps to
};

class GamepadManager {
public:
    GamepadManager();
    ~GamepadManager() = default;

    void Initialize();
    void Shutdown();
    void Update();

    // Accessors
    const GamepadState& GetState() const { return m_current_state; }
    bool IsPhysicalConnected() const { return m_current_state.type == GamepadType::Physical; }

    // Virtual gamepad
    void ToggleVirtualGamepad();
    bool IsVirtualGamepadVisible() const { return m_virtual_gamepad_visible; }
    void SetVirtualGamepadVisible(bool visible);

    // Touch input from JNI (surface-pixel coords).
    void FeedTouch(int pointer_id, bool down, float x, float y);

    // Control geometry (surface pixels) — used by menu to draw + map touches.
    void SetDisplaySize(int w, int h);
    VirtualPadLayout GetLayout() const;
    VirtualZone HitTest(float x, float y) const;
    static float GetStickRadius() { return 96.0f; }

    // Keyboard mapping to virtual gamepad
    bool IsPressed(SDL_Scancode key) const;

    // Vibration
    void SetVibration(float left, float right);

private:
    GamepadState m_current_state;
    bool m_initialized = false;
    bool m_virtual_gamepad_visible = false;

    std::array<VirtualTouch, 8> m_touches;
    // A tap is shorter than the guest's input polling interval (~300 ms under
    // load). Keep latching buttons down for at least this long after lift.
    static constexpr int64_t kMinPressMs = 300;
    static constexpr size_t kZoneCount = static_cast<size_t>(VirtualZone::DPadRight) + 1;
    std::array<int64_t, kZoneCount> m_release_at_ms{};
    static int64_t NowMs();

    int m_display_width = 0;
    int m_display_height = 0;

    void UpdatePhysicalState(SDL_Gamepad* gamepad, GamepadState& state);
    void UpdateVirtualState();
    void FlushExpiredReleases();
    void ClearState();
};

GamepadManager& GetGamepadManager();

// Bridges the mcla virtual/physical gamepad state into the rexglue InputSystem
// that feeds XInputGetState on the guest. The default rexglue SDL input driver
// requires a real window (SDL_INIT_GAMEPAD) which is absent on Android, so we
// register our own driver that reads GetGamepadManager().GetState() directly.
class MclaInputDriver final : public rex::input::InputDriver {
 public:
  MclaInputDriver() : InputDriver(nullptr, 0) {}
  ~MclaInputDriver() override = default;

  rex::X_STATUS Setup() override { return X_STATUS_SUCCESS; }

  void EnumerateDevices(std::vector<rex::input::DeviceInfo>& out) override {
    rex::input::DeviceInfo info;
    info.id = kMclaDevice;
    info.name = "MCLA Virtual Gamepad";
    info.synthetic = false;
    out.push_back(info);
  }

  rex::X_RESULT GetDeviceCapabilities(
      rex::input::DeviceId id, uint32_t flags,
      rex::input::X_INPUT_CAPABILITIES* out_caps) override {
    if (id != kMclaDevice) return X_ERROR_DEVICE_NOT_CONNECTED;
    if (out_caps) {
      std::memset(out_caps, 0, sizeof(*out_caps));
      out_caps->type = 0x01;      // XINPUT_DEVTYPE_GAMEPAD
      out_caps->sub_type = 0x01;  // XINPUT_DEVSUBTYPE_GAMEPAD
      out_caps->flags = 0;
      out_caps->gamepad.buttons = 0xFFFF;
      out_caps->gamepad.left_trigger = 0xFF;
      out_caps->gamepad.right_trigger = 0xFF;
      out_caps->gamepad.thumb_lx = static_cast<int16_t>(0x7FFF);
      out_caps->gamepad.thumb_ly = static_cast<int16_t>(0x7FFF);
      out_caps->gamepad.thumb_rx = static_cast<int16_t>(0x7FFF);
      out_caps->gamepad.thumb_ry = static_cast<int16_t>(0x7FFF);
      out_caps->vibration.left_motor_speed = 0xFFFF;
      out_caps->vibration.right_motor_speed = 0xFFFF;
    }
    return X_ERROR_SUCCESS;
  }

  rex::X_RESULT GetDeviceState(rex::input::DeviceId id,
                                      rex::input::X_INPUT_STATE* out_state) override {
    if (id != kMclaDevice) return X_ERROR_DEVICE_NOT_CONNECTED;
    if (!out_state) return X_ERROR_SUCCESS;
    std::memset(out_state, 0, sizeof(*out_state));

    static uint32_t packet = 1;
    const auto& s = GetGamepadManager().GetState();

    static uint32_t polls = 0;
    static uint16_t last_buttons = 0xFFFF;
    if (++polls % 5 == 0 || polls <= 3)
      __android_log_print(ANDROID_LOG_INFO, "MCLA_GPAD",
                          "GetDeviceState #%u id=%d s.a=%d", polls, id, s.a ? 1 : 0);
    (void)last_buttons;

    uint16_t buttons = 0;
    if (s.a)       buttons |= rex::input::X_INPUT_GAMEPAD_A;
    if (s.b)       buttons |= rex::input::X_INPUT_GAMEPAD_B;
    if (s.x)       buttons |= rex::input::X_INPUT_GAMEPAD_X;
    if (s.y)       buttons |= rex::input::X_INPUT_GAMEPAD_Y;
    if (s.lb)      buttons |= rex::input::X_INPUT_GAMEPAD_LEFT_SHOULDER;
    if (s.rb)      buttons |= rex::input::X_INPUT_GAMEPAD_RIGHT_SHOULDER;
    if (s.back)    buttons |= rex::input::X_INPUT_GAMEPAD_BACK;
    if (s.start)   buttons |= rex::input::X_INPUT_GAMEPAD_START;
    if (s.guide)   buttons |= rex::input::X_INPUT_GAMEPAD_GUIDE;
    if (s.lstick)  buttons |= rex::input::X_INPUT_GAMEPAD_LEFT_THUMB;
    if (s.rstick)  buttons |= rex::input::X_INPUT_GAMEPAD_RIGHT_THUMB;
    if (s.dpad_y > 0)  buttons |= rex::input::X_INPUT_GAMEPAD_DPAD_UP;
    if (s.dpad_y < 0)  buttons |= rex::input::X_INPUT_GAMEPAD_DPAD_DOWN;
    if (s.dpad_x < 0)  buttons |= rex::input::X_INPUT_GAMEPAD_DPAD_LEFT;
    if (s.dpad_x > 0)  buttons |= rex::input::X_INPUT_GAMEPAD_DPAD_RIGHT;

    out_state->packet_number = packet++;
    out_state->gamepad.buttons = buttons;
    if (buttons != last_buttons) {
      __android_log_print(ANDROID_LOG_INFO, "MCLA_GPAD",
                          "BTN_CHANGE buttons=0x%04X", buttons);
      last_buttons = buttons;
    }
    out_state->gamepad.left_trigger =
        static_cast<uint8_t>(s.left_trigger * 255.0f);
    out_state->gamepad.right_trigger =
        static_cast<uint8_t>(s.right_trigger * 255.0f);
    out_state->gamepad.thumb_lx = static_cast<int16_t>(s.left_x * 32767.0f);
    out_state->gamepad.thumb_ly = static_cast<int16_t>(-s.left_y * 32767.0f);
    out_state->gamepad.thumb_rx = static_cast<int16_t>(s.right_x * 32767.0f);
    out_state->gamepad.thumb_ry = static_cast<int16_t>(-s.right_y * 32767.0f);
    return X_ERROR_SUCCESS;
  }

  rex::X_RESULT SetDeviceVibration(rex::input::DeviceId id,
                                          rex::input::X_INPUT_VIBRATION* vibration) override {
    if (id != kMclaDevice) return X_ERROR_DEVICE_NOT_CONNECTED;
    return X_ERROR_SUCCESS;
  }

  rex::X_RESULT GetDeviceKeystroke(rex::input::DeviceId id, uint32_t flags,
                                          rex::input::X_INPUT_KEYSTROKE* out_keystroke) override {
    if (id != kMclaDevice) return X_ERROR_DEVICE_NOT_CONNECTED;
    return X_ERROR_EMPTY;
  }

 private:
  static constexpr rex::input::DeviceId kMclaDevice =
      static_cast<rex::input::DeviceId>(0x4D434C41);  // "MCLA"
};

// Register the MCLA input driver into the given rexglue InputSystem.
void RegisterMclaInputDriver(rex::input::InputSystem* system);

} // namespace mcla