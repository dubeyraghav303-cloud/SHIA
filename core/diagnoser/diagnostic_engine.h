#ifndef DIAGNOSTIC_ENGINE_H
#define DIAGNOSTIC_ENGINE_H

#include <string>
#include <vector>

struct IncidentReport {
    std::string timestamp;
    std::string trigger_service;
    std::string trigger_pattern;
    std::string raw_log_line;
    std::string target_action;
    std::vector<std::string> recent_logs;
    std::string system_cpu_usage;
    std::string system_ram_usage;
    
    std::string ToJson() const;
};

class DiagnosticEngine {
public:
    static IncidentReport Diagnose(
        const std::string& log_file, 
        const std::string& matched_line, 
        const std::string& pattern_str,
        const std::string& target_action,
        const std::string& service_name
    );

private:
   static std::vector<std::string> GetRecentLogs(const std::string& log_file, int num_lines = 50);
   static std::string GetCpuUsage();
   static std::string GetRamUsage();
   static std::string GetCurrentTimestamp();
};

#endif // DIAGNOSTIC_ENGINE_H
