#include "log_tailer.h"
#include <fstream>
#include <iostream>
#include <chrono>

LogTailer::LogTailer(AlertCallback cb, size_t pool_size) : worker_pool(pool_size), running(false), alert_cb(cb) {}

LogTailer::~LogTailer() {
    Stop();
}

void LogTailer::AddMonitor(const MonitorContext& context) {
    monitors.push_back(context);
}

void LogTailer::Start() {
    running = true;
    for (const auto& monitor : monitors) {
        worker_pool.EnqueueTask([this, monitor]() {
            this->TailFile(monitor);
        });
    }
}

void LogTailer::Stop() {
    running = false;
    worker_pool.Shutdown();
}

void LogTailer::TailFile(const MonitorContext& context) {
    std::ifstream file;
    
    // Attempt to open the file, creating it if it doesn't exist for test purposes
    file.open(context.log_file);
    if (!file.is_open()) {
        std::ofstream create_file(context.log_file);
        create_file.close();
        file.open(context.log_file);
    }

    if (!file.is_open()) {
        std::cerr << "[SHIA-Detector] Failed to open log file: " << context.log_file << std::endl;
        return;
    }

    // Seek to the end to only read new lines
    file.seekg(0, std::ios_base::end);
    std::streampos last_pos = file.tellg();

    std::cout << "[SHIA-Detector] Started monitoring: " << context.log_file << std::endl;

    std::string line;
    while (running) {
        file.clear(); // Clear EOF flag
        
        // Save current position
        std::streampos current_pos = file.tellg();
        
        // Seek to end to check if file grew
        file.seekg(0, std::ios_base::end);
        std::streampos end_pos = file.tellg();
        
        if (end_pos > current_pos) {
            // File grew, read the new lines
            file.seekg(current_pos);
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                for (const auto& rule : context.rules) {
                    if (std::regex_search(line, rule.pattern)) {
                        if (alert_cb) {
                            alert_cb(context.log_file, line, rule);
                        }
                    }
                }
            }
        } else {
            // No new data, restore position
            file.seekg(current_pos);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    file.close();
}
