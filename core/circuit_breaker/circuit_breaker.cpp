#include "circuit_breaker.h"
#include <iostream>

CircuitBreaker::CircuitBreaker(int max_fails, int time_window_sec) 
    : max_failures(max_fails), time_window_seconds(time_window_sec) {}

void CircuitBreaker::CleanupOldEntries(const std::string& service_name) {
    auto now = std::chrono::system_clock::now();
    auto& history = failure_history[service_name];
    
    // Remove everything older than the time window
    history.erase(
        std::remove_if(history.begin(), history.end(),
            [this, now](const std::chrono::system_clock::time_point& tp) {
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - tp).count();
                return duration > time_window_seconds;
            }),
        history.end());
}

bool CircuitBreaker::CanRecover(const std::string& service_name) {
    CleanupOldEntries(service_name);

    auto& history = failure_history[service_name];
    
    if (history.size() >= static_cast<size_t>(max_failures)) {
        std::cerr << "[SHIA-CircuitBreaker] 🚨 LOCKOUT TRIGGERED for " << service_name 
                  << ". Failed >" << max_failures << " times in " << time_window_seconds << "s." << std::endl;
        std::cerr << "[SHIA-CircuitBreaker] Flagging for HUMAN INTERVENTION." << std::endl;
        return false;
    }

    // Record this failure
    history.push_back(std::chrono::system_clock::now());
    return true;
}
