// MCLA - Shader Generator
// Generates optimized shaders for multiple devices

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace mcla {

enum class ShaderOptimizationLevel {
    Low,
    Medium,
    High,
    Ultra
};

enum class ShaderDeviceProfile {
    Adreno,
    Mali,
    PowerVR,
    Generic
};

struct ShaderVariant {
    std::string name;
    std::string source_path;
    ShaderDeviceProfile target_device;
    ShaderOptimizationLevel optimization;
    std::string hash;
    
    ShaderVariant() = default;
    ShaderVariant(const std::string& n, ShaderDeviceProfile d, ShaderOptimizationLevel o)
        : name(n), target_device(d), optimization(o) {}
};

class ShaderGenerator {
public:
    ShaderGenerator();
    ~ShaderGenerator() = default;
    
    void Initialize(const std::string& device_name);
    
    // Generate shaders for current device
    bool GenerateForCurrentDevice(ShaderOptimizationLevel level = ShaderOptimizationLevel::High);
    
    // Export generated shaders
    bool ExportShaders(const std::string& output_dir);
    
    // Import a custom shader
    bool ImportShader(const std::string& path);
    
    // Get available variants
    const std::vector<ShaderVariant>& GetVariants() const { return m_variants; }
    
    // Cache management
    void ClearCache();
    size_t GetCacheSize() const;
    
private:
    std::string m_device_name;
    std::string m_cache_dir;
    std::vector<ShaderVariant> m_variants;
    
    std::string GenerateHash(const std::string& source, ShaderDeviceProfile device, ShaderOptimizationLevel level);
    bool ProcessShader(const std::string& source, ShaderVariant& variant);
};

// Global shader generator
ShaderGenerator& GetShaderGenerator();

} // namespace mcla