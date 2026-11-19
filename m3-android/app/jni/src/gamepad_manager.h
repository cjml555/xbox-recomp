// MCLA - Gamepad Manager Header
// Manages physical and virtual gamepad support for Android

#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <string>
#include <vector>
#include <cstdint>

namespace mcla {

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

    // Keyboard mapping to virtual gamepad
    bool IsPressed(SDL_Scancode key) const;

    // Vibration
    void SetVibration(float left, float right);

private:
    GamepadState m_current_state;
    bool m_initialized = false;
    bool m_virtual_gamepad_visible = false;

    void UpdatePhysicalState(SDL_Gamepad* gamepad, GamepadState& state);
    void UpdateVirtualState();
};

GamepadManager& GetGamepadManager();

} // namespace mcla