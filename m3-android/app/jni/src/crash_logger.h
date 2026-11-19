// MCLA - Crash Logger
// Handles crash logs and sends them to developers

#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace mcla {

struct CrashReport {
    std::string timestamp;
    std::string stack_trace;
    std::string device_info;
    std::string app_version;
    std::string build_info;
    std::vector<std::string> attachments;
};

class CrashLogger {
public:
    CrashLogger();
    ~CrashLogger() = default;
    
    void Initialize();
    void Shutdown();
    
    // Get available crash logs
    std::vector<CrashReport> GetRecentCrashes(int count = 10);
    
    // Clear crash logs
    void ClearLogs();
    
    // Send crash report to developers
    bool SendReport(const CrashReport& report);
    
    // Get log file path
    std::string GetLogFilePath() const { return m_log_path; }
    
private:
    std::string m_log_path;
    std::string m_cache_dir;
    bool m_initialized = false;
    
    std::string GetCurrentTimestamp();
    std::string GetDeviceInfo();
};

// Global crash logger
CrashLogger& GetCrashLogger();

} // namespace mcla