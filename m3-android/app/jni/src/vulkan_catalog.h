// MCLA - Vulkan Driver Catalog
// Catalog of available Vulkan drivers

#pragma once

#include <string>
#include <vector>

namespace mcla {

enum class VulkanDriverType {
    Mesa,
    Turnip,
    AhuGPU,
    Unknown
};

struct VulkanDriver {
    std::string name;
    std::string description;
    VulkanDriverType type;
    bool installed = false;
    std::string path;
    
    VulkanDriver() = default;
    VulkanDriver(const std::string& n, const std::string& d, VulkanDriverType t, bool i = false)
        : name(n), description(d), type(t), installed(i) {}
};

class VulkanCatalog {
public:
    VulkanCatalog();
    ~VulkanCatalog() = default;
    
    void Initialize();
    void Refresh();
    
    const std::vector<VulkanDriver>& GetDrivers() const { return m_drivers; }
    const VulkanDriver* GetCurrentDriver() const;
    
    bool InstallDriver(const VulkanDriver& driver);
    bool UninstallDriver(const VulkanDriver& driver);
    
    void SetCurrentDriver(const std::string& name);
    
private:
    std::vector<VulkanDriver> m_drivers;
    std::string m_current_driver;
    
    void PopulateStandardDrivers();
    void DetectInstalledDrivers();
};

// Global Vulkan catalog
VulkanCatalog& GetVulkanCatalog();

} // namespace mcla