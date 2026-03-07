#ifndef CIRCUIT_BREAKER_H
#define CIRCUIT_BREAKER_H

#include <string>
#include <map>
#include <vector>
#include <chrono>

class CircuitBreaker {
public:
    CircuitBreaker(int max_fails, int time_window_sec);

    // Returns true if the action is ALLOWED.
    // Returns false if the action is LOCKED (Threshold exceeded).
    bool CanRecover(const std::string& service_name);

private:
    void CleanupOldEntries(const std::string& service_name);

    int max_failures;
    int time_window_seconds;

    // Service Name -> List of failure timestamps
    std::map<std::string, std::vector<std::chrono::system_clock::time_point>> failure_history;
};

#endif // CIRCUIT_BREAKER_H
