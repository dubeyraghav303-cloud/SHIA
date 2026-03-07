#include "recovery_controller.h"
#include "../ai_bridge.h"
#include "../telemetry_bridge.h"
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <sstream>

static std::string GetISOTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

bool RecoveryController::ExecuteAction(const std::string& action, const std::string& service_name, const std::string& error_log, const std::string& file_context, bool enable_ai_repair) {
    std::cout << "[SHIA-Recoverer] Executing Action: " << action << " on service: " << service_name << std::endl;
    std::string timestamp = GetISOTimestamp();
    
    bool success = false;
    if (action == "RESTART_SERVICE") {
        success = RestartService(service_name);
    } else if (action == "FLUSH_CACHE") {
        success = FlushCache(service_name);
    } else if (action == "CLEAN_TMP") {
        success = CleanTmp();
    } else {
        std::cerr << "[SHIA-Recoverer] Unknown action: " << action << std::endl;
    }

    if (!success) {
        std::cerr << "[SHIA-Recoverer] ❌ PRIMARY ACTION FAILED." << std::endl;
        
        if (enable_ai_repair) {
            std::cout << "[SHIA-Recoverer] 🤖 Initiating AI Fallback Fix..." << std::endl;
            std::string fix_command = AIBridge::GetFallbackFix(error_log, file_context);
            
            if (!fix_command.empty()) {
                std::cout << "[SHIA-Recoverer] 💡 Applying AI Sugggestion:\n" << fix_command << std::endl;
                int ret = std::system(fix_command.c_str());
                if (ret == 0) {
                     std::cout << "[SHIA-Recoverer] ✅ AI Fallback succeeded." << std::endl;
                     TelemetryBridge::SendIncident(timestamp, service_name, error_log, action + " (AI: " + fix_command + ")", true, "healed");
                     return true;
                } else {
                     std::cerr << "[SHIA-Recoverer] ❌ AI Fallback failed." << std::endl;
                     TelemetryBridge::SendIncident(timestamp, service_name, error_log, action, true, "failed");
                     return false;
                }
            } else {
                std::cerr << "[SHIA-Recoverer] ❌ AI failed to provide a valid, safe fix." << std::endl;
                TelemetryBridge::SendIncident(timestamp, service_name, error_log, action, true, "failed");
            }
        } else {
            TelemetryBridge::SendIncident(timestamp, service_name, error_log, action, false, "failed");
        }
    } else {
        TelemetryBridge::SendIncident(timestamp, service_name, error_log, action, false, "healed");
    }
    
    return success;
}

bool RecoveryController::RestartService(const std::string& service_name) {
    // In a real SRE environment, this would be systemctl restart <service_name>
    // For macOS testing, we simulate it or restart a brew service.
    std::cout << "[SHIA-Recoverer] Simulating restart of " << service_name << "..." << std::endl;
    std::string cmd = "echo \"Restarting " + service_name + "\""; 
    int ret = std::system(cmd.c_str());
    return ret == 0;
}

bool RecoveryController::FlushCache(const std::string& service_name) {
    std::cout << "[SHIA-Recoverer] Simulating cache flush for " << service_name << "..." << std::endl;
    std::string cmd = "echo \"Flushing cache for " + service_name + "\""; 
    int ret = std::system(cmd.c_str());
    return ret == 0;
}

bool RecoveryController::CleanTmp() {
    std::cout << "[SHIA-Recoverer] Cleaning safe temp directories..." << std::endl;
    // Safe mock implementation for demo
    std::string cmd = "echo \"Cleaning /tmp/shia_mock_tmp\" && mkdir -p /tmp/shia_mock_tmp && rm -rf /tmp/shia_mock_tmp/*"; 
    int ret = std::system(cmd.c_str());
    return ret == 0;
}
