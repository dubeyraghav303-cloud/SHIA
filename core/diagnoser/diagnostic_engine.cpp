#include "diagnostic_engine.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <memory>
#include <array>

std::string ExecCmd(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return "Command execution failed";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string IncidentReport::ToJson() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"timestamp\": \"" << timestamp << "\",\n";
    ss << "  \"service\": \"" << trigger_service << "\",\n";
    ss << "  \"trigger_pattern\": \"" << trigger_pattern << "\",\n";
    ss << "  \"raw_log_line\": \"" << raw_log_line << "\",\n";
    ss << "  \"target_action\": \"" << target_action << "\",\n";
    ss << "  \"cpu_usage\": \"" << system_cpu_usage << "\",\n";
    ss << "  \"ram_usage\": \"" << system_ram_usage << "\",\n";
    ss << "  \"recent_logs\": [\n";
    for (size_t i = 0; i < recent_logs.size(); ++i) {
        ss << "    \"" << recent_logs[i] << "\"";
        if (i < recent_logs.size() - 1) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

IncidentReport DiagnosticEngine::Diagnose(
    const std::string& log_file, 
    const std::string& matched_line, 
    const std::string& pattern_str,
    const std::string& target_action,
    const std::string& service_name
) {
    IncidentReport report;
    report.timestamp = GetCurrentTimestamp();
    report.trigger_service = service_name;
    report.trigger_pattern = pattern_str;
    report.raw_log_line = matched_line;
    report.target_action = target_action;
    
    report.recent_logs = GetRecentLogs(log_file, 50);
    report.system_cpu_usage = GetCpuUsage();
    report.system_ram_usage = GetRamUsage();
    
    return report;
}

std::vector<std::string> DiagnosticEngine::GetRecentLogs(const std::string& log_file, int num_lines) {
    std::vector<std::string> lines;
    std::string cmd = "tail -n " + std::to_string(num_lines) + " " + log_file;
    std::string output = ExecCmd(cmd.c_str());
    
    std::stringstream ss(output);
    std::string line;
    while (std::getline(ss, line)) {
        // Simple escape quotes for JSON 
        size_t pos = 0;
        while ((pos = line.find("\"", pos)) != std::string::npos) {
            line.replace(pos, 1, "\\\"");
            pos += 2;
        }
        lines.push_back(line);
    }
    return lines;
}

std::string DiagnosticEngine::GetCpuUsage() {
    // macOS specific top command for CPU snapshot
    std::string cmd = "top -l 1 | grep -E '^CPU'";
    std::string output = ExecCmd(cmd.c_str());
    if (!output.empty() && output.back() == '\n') output.pop_back();
    return output.empty() ? "N/A" : output;
}

std::string DiagnosticEngine::GetRamUsage() {
    // macOS specific vm_stat or top for RAM snapshot
    std::string cmd = "top -l 1 | grep -E '^PhysMem'";
    std::string output = ExecCmd(cmd.c_str());
    if (!output.empty() && output.back() == '\n') output.pop_back();
    return output.empty() ? "N/A" : output;
}

std::string DiagnosticEngine::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
