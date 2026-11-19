// MCLA - Shader Generator Implementation
#include "shader_generator.h"
#include <SDL3/SDL.h>
#include <sstream>
#include <iomanip>

namespace mcla {

ShaderGenerator::ShaderGenerator() {
    // Initialize cache directory
    char* cache_dir = SDL_GetPrefPath("xboxrecomp", "shader_cache");
    if (cache_dir) {
        m_cache_dir = std::string(cache_dir);
        SDL_free(cache_dir);
    }
}

void ShaderGenerator::Initialize(const std::string& device_name) {
    m_device_name = device_name;
    SDL_Log("ShaderGenerator initialized for device: %s", device_name.c_str());
    
    // Populate with base shader variants
    m_variants.clear();
    m_variants.emplace_back("vs_main", ShaderDeviceProfile::Generic, ShaderOptimizationLevel::Medium);
    m_variants.emplace_back("fs_main", ShaderDeviceProfile::Generic, ShaderOptimizationLevel::Medium);
    m_variants.emplace_back("vs_shadow", ShaderDeviceProfile::Generic, ShaderOptimizationLevel::Low);
    m_variants.emplace_back("fs_shadow", ShaderDeviceProfile::Generic, ShaderOptimizationLevel::Low);
}

bool ShaderGenerator::GenerateForCurrentDevice(ShaderOptimizationLevel level) {
    SDL_Log("Generating shaders with level %d for %s", 
            static_cast<int>(level), m_device_name.c_str());
    
    for (auto& variant : m_variants) {
        variant.optimization = level;
        // Generate hash for this variant
        variant.hash = GenerateHash(variant.name, variant.target_device, variant.optimization);
    }
    
    return true;
}

bool ShaderGenerator::ExportShaders(const std::string& output_dir) {
    std::filesystem::create_directories(output_dir);
    
    for (const auto& variant : m_variants) {
        std::string output_path = output_dir + "/" + variant.name + ".spv";
        SDL_Log("Would export shader: %s", output_path.c_str());
        // TODO: Actually generate and write shader binary
    }
    
    return true;
}

bool ShaderGenerator::ImportShader(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    ShaderVariant variant("imported", ShaderDeviceProfile::Generic, ShaderOptimizationLevel::Medium);
    variant.source_path = path;
    m_variants.push_back(variant);
    
    SDL_Log("Imported shader: %s", path.c_str());
    return true;
}

void ShaderGenerator::ClearCache() {
    if (!m_cache_dir.empty() && std::filesystem::exists(m_cache_dir)) {
        std::filesystem::remove_all(m_cache_dir);
        SDL_Log("Shader cache cleared");
    }
}

size_t ShaderGenerator::GetCacheSize() const {
    if (!m_cache_dir.empty() && std::filesystem::exists(m_cache_dir)) {
        return std::filesystem::file_size(m_cache_dir);
    }
    return 0;
}

std::string ShaderGenerator::GenerateHash(const std::string& source, ShaderDeviceProfile device, ShaderOptimizationLevel level) {
    // Simple hash generation
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    size_t hash = 0;
    for (char c : source + std::to_string(static_cast<int>(device)) + std::to_string(static_cast<int>(level))) {
        hash = hash * 31 + static_cast<unsigned char>(c);
    }
    
    ss << std::setw(16) << hash;
    return ss.str();
}

ShaderGenerator& GetShaderGenerator() {
    static ShaderGenerator instance;
    return instance;
}

} // namespace mcla