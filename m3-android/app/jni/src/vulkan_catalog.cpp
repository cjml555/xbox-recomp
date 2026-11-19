// MCLA - Vulkan Driver Catalog Implementation
#include "vulkan_catalog.h"
#include <SDL3/SDL.h>

namespace mcla {

VulkanCatalog::VulkanCatalog() {
    PopulateStandardDrivers();
}

void VulkanCatalog::Initialize() {
    SDL_Log("VulkanCatalog initialized");
    DetectInstalledDrivers();
}

void VulkanCatalog::Refresh() {
    DetectInstalledDrivers();
}

const VulkanDriver* VulkanCatalog::GetCurrentDriver() const {
    for (const auto& driver : m_drivers) {
        if (driver.name == m_current_driver) {
            return &driver;
        }
    }
    return nullptr;
}

bool VulkanCatalog::InstallDriver(const VulkanDriver& driver) {
    SDL_Log("Would install driver: %s", driver.name.c_str());
    // TODO: Implement driver installation
    return true;
}

bool VulkanCatalog::UninstallDriver(const VulkanDriver& driver) {
    SDL_Log("Would uninstall driver: %s", driver.name.c_str());
    // TODO: Implement driver uninstallation
    return true;
}

void VulkanCatalog::SetCurrentDriver(const std::string& name) {
    m_current_driver = name;
    SDL_Log("Current Vulkan driver set to: %s", name.c_str());
}

void VulkanCatalog::PopulateStandardDrivers() {
    m_drivers.emplace_back("Mesa Gallium3D", 
        "Open-source Mesa 3D graphics library with Adreno support",
        VulkanDriverType::Mesa, true);
        
    m_drivers.emplace_back("Turnip",
        "Free and open-source Vulkan driver for Qualcomm Adreno GPUs",
        VulkanDriverType::Turnip, true);
        
    m_drivers.emplace_back("AHU GPU",
        "Community-maintained Vulkan driver for high-end emulation",
        VulkanDriverType::AhuGPU, false);
}

void VulkanCatalog::DetectInstalledDrivers() {
    // Mark drivers as installed based on system detection
    for (auto& driver : m_drivers) {
        driver.installed = true; // For now, assume all are available
    }
}

VulkanCatalog& GetVulkanCatalog() {
    static VulkanCatalog instance;
    return instance;
}

} // namespace mcla