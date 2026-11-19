// MCLA - Gamepad Manager Implementation
#include "gamepad_manager.h"

namespace mcla {

GamepadManager::GamepadManager() {
    m_current_state.type = GamepadType::Virtual;
}

void GamepadManager::Initialize() {
    if (m_initialized) return;

    SDL_Log("GamepadManager initialized");
    m_initialized = true;
}

void GamepadManager::Update() {
    // Clear previous state
    m_current_state = GamepadState();
    m_current_state.type = GamepadType::Unknown;

    // Check for physical connection
    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);
    
    if (joysticks && count > 0) {
        SDL_JoystickID instance_id = joysticks[0];
        SDL_Gamepad* gp = SDL_GetGamepadFromID(instance_id);
        if (gp) {
            UpdatePhysicalState(gp, m_current_state);
            m_current_state.type = GamepadType::Physical;
            m_current_state.connected = true;
        }
    }

    // If no physical gamepad found, use virtual
    if (m_current_state.type == GamepadType::Unknown) {
        m_current_state.type = GamepadType::Virtual;
        UpdateVirtualState();
        m_current_state.connected = true;
    }
}

void GamepadManager::Shutdown() {
    SDL_Log("GamepadManager shutdown");
    m_initialized = false;
}

void GamepadManager::UpdatePhysicalState(SDL_Gamepad* gamepad, GamepadState& state) {
    if (!gamepad) return;

    // Left analog stick
    int16_t lx = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    int16_t ly = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    state.left_x = lx / 32767.0f;
    state.left_y = ly / 32767.0f;

    // Right analog stick
    int16_t rx = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    int16_t ry = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    state.right_x = rx / 32767.0f;
    state.right_y = ry / 32767.0f;

    // Triggers (using left/right trigger axes)
    int16_t lt_raw = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    int16_t rt_raw = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
    // SDL returns -32767 to 32767 for triggers, map to 0..1
    state.left_trigger = std::max(0.0f, static_cast<float>(lt_raw) / 32767.0f);
    state.right_trigger = std::max(0.0f, static_cast<float>(rt_raw) / 32767.0f);

    // Buttons (using SDL3 button names)
    state.a = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    state.b = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
    state.x = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST);
    state.y = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH);
    state.lb = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    state.rb = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    state.back = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK);
    state.start = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START);
    state.guide = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
    state.lstick = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
    state.rstick = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
}

void GamepadManager::UpdateVirtualState() {
    // Virtual gamepad is controlled via touch/keyboard
    // This is a placeholder - implementation would handle touch input for virtual gamepad
}

void GamepadManager::ToggleVirtualGamepad() {
    m_virtual_gamepad_visible = !m_virtual_gamepad_visible;
    SDL_Log("Virtual gamepad %s", m_virtual_gamepad_visible ? "visible" : "hidden");
}

bool GamepadManager::IsPressed(SDL_Scancode key) const {
    // Map keyboard keys to virtual gamepad buttons
    switch (key) {
        case SDL_SCANCODE_W: return m_current_state.left_y > 0.5f;
        case SDL_SCANCODE_S: return m_current_state.left_y < -0.5f;
        case SDL_SCANCODE_A: return m_current_state.left_x < -0.5f;
        case SDL_SCANCODE_D: return m_current_state.left_x > 0.5f;
        case SDL_SCANCODE_UP: return m_current_state.dpad_y > 0;
        case SDL_SCANCODE_DOWN: return m_current_state.dpad_y < 0;
        case SDL_SCANCODE_LEFT: return m_current_state.dpad_x < 0;
        case SDL_SCANCODE_RIGHT: return m_current_state.dpad_x > 0;
        default: return false;
    }
}

void GamepadManager::SetVibration(float left, float right) {
    m_current_state.left_motor = left;
    m_current_state.right_motor = right;
    
    // Apply to physical gamepads if connected
    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetGamepads(&count);
    
    if (joysticks && count > 0) {
        SDL_Gamepad* gp = SDL_GetGamepadFromID(joysticks[0]);
        if (gp) {
            SDL_RumbleGamepad(gp, 
                static_cast<uint16_t>(left * 0xFFFF),
                static_cast<uint16_t>(right * 0xFFFF),
                1000);
        }
    }
}

GamepadManager& GetGamepadManager() {
    static GamepadManager instance;
    return instance;
}

} // namespace mcla