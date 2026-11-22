// MCLA - Configuration System
// Stores all user preferences and settings

#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

// Include nlohmann json from the SDK
#include <nlohmann/json.hpp>

namespace mcla {

enum class RendererType : uint8_t {
    XeniaBased = 0,   // Xenia-style recompiled renderer
    NativeVulkan = 1, // Native Vulkan renderer
    OpenGL = 2        // OpenGL fallback
};

enum class GraphicsQuality : uint8_t {
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3,
    Custom = 4
};

enum class Language : uint8_t {
    English = 0,
    Spanish = 1,
    French = 2,
    German = 3,
    Japanese = 4,
    Chinese = 5
};

struct Config {
    // Game settings
    std::string game_data_path;
    std::string save_path;
    bool skip_intro = false;

    // Graphics settings
    RendererType renderer = RendererType::NativeVulkan;
    GraphicsQuality quality = GraphicsQuality::High;
    int resolution_width = 1280;
    int resolution_height = 720;
    bool vsync = true;
    bool msaa_enabled = true;
    int msaa_samples = 4;
    bool motion_blur = true;
    bool imposter_shadows = true;
    bool rubber_banding = true;
    float fov_scale = 1.0f;
    float gamma = 1.0f;

    // Vulkan settings
    std::string vulkan_driver;
    bool use_custom_vulkan_driver = false;
    std::string custom_vulkan_driver_path;

    // Audio settings
    float master_volume = 1.0f;
    float music_volume = 0.8f;
    float sfx_volume = 0.9f;
    float voice_volume = 0.8f;

    // Input settings
    bool use_gamepad = true;
    float gamepad_sensitivity = 1.0f;
    bool vibration_enabled = true;
    bool virtual_gamepad_enabled = true;

    // Language
    Language language = Language::English;

    // Network
    std::string lan_game_tag;
    bool lan_mode_enabled = false;

    // Cheats
    bool cheats_enabled = false;

    // Mods
    bool mods_enabled = false;
    std::vector<std::string> active_mods;

    // Development
    bool dev_mode = false;
    bool show_fps = true;
    bool show_debug_info = false;

    // Paths
    std::string config_path;
    std::string log_path;
    std::string cache_path;

    Config() = default;

    // Load/save
    bool Save(const std::string& path);
    bool Load(const std::string& path);
    void Save();  // Save to default location
    void Load();  // Load from default location

    // Reset to defaults
    void Reset();
};

// Global config instance
Config& GetConfig();

} // namespace mcla