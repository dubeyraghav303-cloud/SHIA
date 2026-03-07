#ifndef LOG_TAILER_H
#define LOG_TAILER_H

#include <string>
#include <vector>
#include <regex>
#include <atomic>
#include <functional>
#include "worker_pool.h"

struct LogRule {
    std::string pattern_str;
    std::regex pattern;
    std::string action;
    std::string service;
};

struct MonitorContext {
    std::string log_file;
    std::vector<LogRule> rules;
};

class LogTailer {
public:
    using AlertCallback = std::function<void(const std::string& log_file, const std::string& matched_line, const LogRule& rule)>;

    LogTailer(AlertCallback cb, size_t pool_size = 4);
    ~LogTailer();

    void AddMonitor(const MonitorContext& context);
    void Start();
    void Stop();

private:
    void TailFile(const MonitorContext& context);

    std::vector<MonitorContext> monitors;
    WorkerPool worker_pool;
    std::atomic<bool> running;
    AlertCallback alert_cb;
};

#endif // LOG_TAILER_H
