// MCLA - Crash Logger Implementation
#include "crash_logger.h"
#include <SDL3/SDL.h>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace mcla {

CrashLogger::CrashLogger() {
    char* cache_dir = SDL_GetPrefPath("xboxrecomp", "crash_logs");
    if (cache_dir) {
        m_cache_dir = std::string(cache_dir);
        SDL_free(cache_dir);
    }
}

void CrashLogger::Initialize() {
    // Create crash log directory
    std::filesystem::create_directories(m_cache_dir);
    
    m_log_path = m_cache_dir + "/crash.log";
    
    SDL_Log("CrashLogger initialized, logs to: %s", m_log_path.c_str());
    m_initialized = true;
}

void CrashLogger::Shutdown() {
    m_initialized = false;
}

std::vector<CrashReport> CrashLogger::GetRecentCrashes(int count) {
    std::vector<CrashReport> reports;
    
    // In a real implementation, this would read crash logs from files
    // For now, return empty vector
    
    return reports;
}

void CrashLogger::ClearLogs() {
    if (!m_log_path.empty() && std::filesystem::exists(m_log_path)) {
        std::filesystem::remove(m_log_path);
        SDL_Log("Crash logs cleared");
    }
}

bool CrashLogger::SendReport(const CrashReport& report) {
    // In a real implementation, this would:
    // 1. Create email intent or HTTP POST
    // 2. Include stack trace and attachments
    // 3. Send to developer server
    
    SDL_Log("Would send crash report with timestamp: %s", report.timestamp.c_str());
    SDL_Log("Stack trace length: %zu bytes", report.stack_trace.length());
    
    // For now, just log it
    return true;
}

std::string CrashLogger::GetCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::string CrashLogger::GetDeviceInfo() {
    std::stringstream ss;
    
    int cores = SDL_GetNumLogicalCPUCores();
    
    ss << "CPU: ARM64\n";
    ss << "Cores: " << cores << "\n";
    ss << "Platform: Android\n";
    
    return ss.str();
}

CrashLogger& GetCrashLogger() {
    static CrashLogger instance;
    return instance;
}

} // namespace mcla