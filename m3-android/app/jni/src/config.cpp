// MCLA - Configuration System Implementation
#include "config.h"
#include "updater.h"
#include <fstream>

namespace mcla {

static std::string ConfigPath = "";

Config& GetConfig() {
    static Config instance;
    return instance;
}

bool Config::Save(const std::string& path) {
    try {
        nlohmann::json j;
        j["game_data_path"] = game_data_path;
        j["save_path"] = save_path;
        j["skip_intro"] = skip_intro;
        
        // Graphics settings
        j["renderer"] = static_cast<int>(renderer);
        j["quality"] = static_cast<int>(quality);
        j["resolution_width"] = resolution_width;
        j["resolution_height"] = resolution_height;
        j["vsync"] = vsync;
        j["msaa_enabled"] = msaa_enabled;
        j["msaa_samples"] = msaa_samples;
        j["motion_blur"] = motion_blur;
        j["imposter_shadows"] = imposter_shadows;
        j["rubber_banding"] = rubber_banding;
        j["fov_scale"] = fov_scale;
        j["gamma"] = gamma;
        
        // Vulkan settings
        j["vulkan_driver"] = vulkan_driver;
        j["use_custom_vulkan_driver"] = use_custom_vulkan_driver;
        
        // Audio settings
        j["master_volume"] = master_volume;
        j["music_volume"] = music_volume;
        j["sfx_volume"] = sfx_volume;
        j["voice_volume"] = voice_volume;
        
        // Input settings
        j["use_gamepad"] = use_gamepad;
        j["gamepad_sensitivity"] = gamepad_sensitivity;
        j["vibration_enabled"] = vibration_enabled;
        j["virtual_gamepad_enabled"] = virtual_gamepad_enabled;
        
        // Language
        j["language"] = static_cast<int>(language);
        
        // Network
        j["lan_game_tag"] = lan_game_tag;
        j["lan_mode_enabled"] = lan_mode_enabled;
        
        // Cheats
        j["cheats_enabled"] = cheats_enabled;
        
        // Mods
        j["mods_enabled"] = mods_enabled;
        j["active_mods"] = active_mods;
        
        // Development
        j["dev_mode"] = dev_mode;
        j["show_fps"] = show_fps;
        j["show_debug_info"] = show_debug_info;
        
        std::ofstream file(path);
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        SDL_Log("Failed to save config: %s", e.what());
        return false;
    }
}

bool Config::Load(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        game_data_path = j.value("game_data_path", game_data_path);
        save_path = j.value("save_path", save_path);
        skip_intro = j.value("skip_intro", skip_intro);
        
        renderer = static_cast<RendererType>(j.value("renderer", static_cast<int>(renderer)));
        quality = static_cast<GraphicsQuality>(j.value("quality", static_cast<int>(quality)));
        resolution_width = j.value("resolution_width", resolution_width);
        resolution_height = j.value("resolution_height", resolution_height);
        vsync = j.value("vsync", vsync);
        msaa_enabled = j.value("msaa_enabled", msaa_enabled);
        msaa_samples = j.value("msaa_samples", msaa_samples);
        motion_blur = j.value("motion_blur", motion_blur);
        imposter_shadows = j.value("imposter_shadows", imposter_shadows);
        rubber_banding = j.value("rubber_banding", rubber_banding);
        fov_scale = j.value("fov_scale", fov_scale);
        gamma = j.value("gamma", gamma);
        
        vulkan_driver = j.value("vulkan_driver", vulkan_driver);
        use_custom_vulkan_driver = j.value("use_custom_vulkan_driver", use_custom_vulkan_driver);
        
        master_volume = j.value("master_volume", master_volume);
        music_volume = j.value("music_volume", music_volume);
        sfx_volume = j.value("sfx_volume", sfx_volume);
        voice_volume = j.value("voice_volume", voice_volume);
        
        use_gamepad = j.value("use_gamepad", use_gamepad);
        gamepad_sensitivity = j.value("gamepad_sensitivity", gamepad_sensitivity);
        vibration_enabled = j.value("vibration_enabled", vibration_enabled);
        virtual_gamepad_enabled = j.value("virtual_gamepad_enabled", virtual_gamepad_enabled);
        
        language = static_cast<Language>(j.value("language", static_cast<int>(language)));
        
        lan_game_tag = j.value("lan_game_tag", lan_game_tag);
        lan_mode_enabled = j.value("lan_mode_enabled", lan_mode_enabled);
        
        cheats_enabled = j.value("cheats_enabled", cheats_enabled);
        mods_enabled = j.value("mods_enabled", mods_enabled);
        active_mods = j.value("active_mods", active_mods);
        
        dev_mode = j.value("dev_mode", dev_mode);
        show_fps = j.value("show_fps", show_fps);
        show_debug_info = j.value("show_debug_info", show_debug_info);
        
        return true;
    } catch (const std::exception& e) {
        SDL_Log("Failed to load config: %s", e.what());
        return false;
    }
}

void Config::Reset() {
    *this = Config();
}

void Config::Save() {
    // Save to default location
    char* pref_path = SDL_GetPrefPath("xboxrecomp", "mcla");
    if (pref_path) {
        std::string path = std::string(pref_path) + "config.json";
        Save(path);
        SDL_free(pref_path);
    }
}

void Config::Load() {
    // Load from default location
    char* pref_path = SDL_GetPrefPath("xboxrecomp", "mcla");
    if (pref_path) {
        std::string path = std::string(pref_path) + "config.json";
        if (std::filesystem::exists(path)) {
            Load(path);
        }
        SDL_free(pref_path);
    }
}

} // namespace mcla