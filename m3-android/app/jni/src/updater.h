// MCLA - Updater/Verificator
// Checks for updates and verifies game files

#pragma once

#include <string>
#include <functional>

namespace mcla {

enum class UpdateStatus {
    UpToDate,
    UpdateAvailable,
    DownloadFailed,
    VerificationFailed,
    NoInternet
};

struct UpdateInfo {
    std::string current_version;
    std::string latest_version;
    std::string download_url;
    int file_size = 0;
    bool is_update = false;
};

class Updater {
public:
    Updater();
    ~Updater() = default;
    
    void Initialize();
    void Shutdown();
    
    // Check for updates
    UpdateStatus CheckForUpdates();
    
    // Download update
    bool DownloadUpdate();
    
    // Verify local files
    UpdateStatus VerifyGameFiles();
    
    // Get update info
    const UpdateInfo& GetUpdateInfo() const { return m_update_info; }
    
    // Callbacks
    void SetUpdateCallback(std::function<void(UpdateStatus)> callback) {
        m_update_callback = callback;
    }
    
private:
    UpdateInfo m_update_info;
    std::function<void(UpdateStatus)> m_update_callback;
    std::string m_server_url;
    bool m_initialized = false;
    
    bool CheckNetworkConnectivity();
    UpdateInfo ParseUpdateInfo(const std::string& json);
};

// Global updater
Updater& GetUpdater();

} // namespace mcla