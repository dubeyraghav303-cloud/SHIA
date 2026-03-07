#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <chrono>
#include <thread>
#include <random>
#include "detector/log_tailer.h"
#include "diagnoser/diagnostic_engine.h"
#include "recoverer/recovery_controller.h"
#include "circuit_breaker/circuit_breaker.h"
#include "telemetry_bridge.h"
#include "integrity_manager.h"
#include "blockchain_bridge.h"
#include "detector/worker_pool.h"
std::string GenerateSessionUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);
    const char* v = "0123456789abcdef";
    std::string res;
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) res += '-';
        else if (i == 14) res += '4';
        else if (i == 19) res += v[dis2(gen)];
        else res += v[dis(gen)];
    }
    return res;
}

// Simple JSON parser for the config (since we don't have nlohmann/json setup by default)
// For a production app, we'd use a real JSON library.
void LoadConfig(const std::string& config_path, LogTailer& tailer, std::shared_ptr<CircuitBreaker>& cb, bool& enable_ai_repair) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "[SHIA-Engine] Failed to load config: " << config_path << std::endl;
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Very naive "parsing" for the demo
    MonitorContext ctx1;
    ctx1.log_file = "/var/log/syslog";
    ctx1.rules.push_back({".*Out of memory.*", std::regex(".*Out of memory.*"), "FLUSH_CACHE", "system"});
    tailer.AddMonitor(ctx1);

    MonitorContext ctx2;
    ctx2.log_file = "/tmp/test_syslog.log";
    ctx2.rules.push_back({".*Segmentation Fault.*", std::regex(".*Segmentation Fault.*", std::regex_constants::icase), "RESTART_SERVICE", "dummy_service"});
    ctx2.rules.push_back({".*Out of Memory.*", std::regex(".*Out of Memory.*", std::regex_constants::icase), "FLUSH_CACHE", "dummy_service"});
    ctx2.rules.push_back({".*Connection refused.*", std::regex(".*Connection refused.*", std::regex_constants::icase), "RESTART_SERVICE", "dummy_service"});
    tailer.AddMonitor(ctx2);
    
    // Default Circuit Breaker limits 3 fails in 10 mins (600s)
    cb = std::make_shared<CircuitBreaker>(3, 600);
    
    if (content.find("\"enable_ai_repair\": true") != std::string::npos) {
        enable_ai_repair = true;
    } else {
        enable_ai_repair = false;
    }
    
    // Very naive extraction
    std::string supabase_url = "";
    std::string supabase_key = "";
    
    size_t url_pos = content.find("\"supabase_url\": \"");
    if (url_pos != std::string::npos) {
        url_pos += 17;
        size_t end_pos = content.find("\"", url_pos);
        if (end_pos != std::string::npos) supabase_url = content.substr(url_pos, end_pos - url_pos);
    }
    
    size_t key_pos = content.find("\"supabase_key\": \"");
    if (key_pos != std::string::npos) {
        key_pos += 17;
        size_t end_pos = content.find("\"", key_pos);
        if (end_pos != std::string::npos) supabase_key = content.substr(key_pos, end_pos - key_pos);
    }
    
    if (supabase_url == "YOUR_SUPABASE_URL" || supabase_url.empty() || supabase_url.find("supabase.com/dashboard") != std::string::npos) {
        std::cerr << "\n[SHIA-Engine] ⚠️  WARNING ⚠️" << std::endl;
        std::cerr << " -> Valid Supabase REST API URL not found in config." << std::endl;
        std::cerr << " -> Telemetry sync is DISABLED." << std::endl;
    } else if (supabase_key == "YOUR_SUPABASE_ANON_KEY" || supabase_key.empty()) {
        std::cerr << "\n[SHIA-Engine] ⚠️  WARNING ⚠️" << std::endl;
        std::cerr << " -> Supabase Anon Key not found in config." << std::endl;
        std::cerr << " -> Telemetry sync is DISABLED." << std::endl;
    } else {
        std::string session_id = GenerateSessionUUID();
        TelemetryBridge::Initialize(supabase_url, supabase_key, session_id);
        std::cout << "[SHIA-Engine] Telemetry sync enabled to " << supabase_url << "\n[SHIA-Engine] Active Session ID: " << session_id << std::endl;
    }
    
    std::cout << "[SHIA-Engine] Config loaded. AI Repair Enabled: " << (enable_ai_repair ? "true" : "false") << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "===========================================" << std::endl;
    std::cout << "   SHIA: Self-Healing Infrastructure Agent " << std::endl;
    std::cout << "===========================================" << std::endl;

    std::shared_ptr<CircuitBreaker> cb;
    bool enable_ai_repair = false;
    
    // Create a WorkerPool for asynchronous tasks like Blockchain anchoring
    auto worker_pool = std::make_shared<WorkerPool>(2);
    
    // Initialize Log Tailer with the unified callback
    LogTailer tailer([&cb, &enable_ai_repair, worker_pool](const std::string& log_file, const std::string& matched_line, const LogRule& rule) {
        std::cout << "\n[SHIA-Detector] ⚠️ ALERT TRIGGERED ⚠️" << std::endl;
        std::cout << " -> File: " << log_file << std::endl;
        std::cout << " -> Match: " << matched_line << std::endl;

        // 1. Diagnose
        std::cout << "[SHIA-Diagnoser] Gathering telemetry..." << std::endl;
        IncidentReport report = DiagnosticEngine::Diagnose(
            log_file, matched_line, rule.pattern_str, rule.action, rule.service);
        
        std::cout << "\n--- INCIDENT REPORT ---" << std::endl;
        std::string json_report = report.ToJson();
        std::cout << json_report << std::endl;
        std::cout << "-----------------------\n" << std::endl;

        // 2. Circuit Breaker Check
        if (cb && !cb->CanRecover(rule.service)) {
            return; // Locked out
        }

        // 3. Recover
        // Ensure "file_context" sent to Gemini is concise. We use the incident report JSON.
        RecoveryController::ExecuteAction(rule.action, rule.service, matched_line, json_report, enable_ai_repair);

        // 4. Audit & Verification (Phase 6)
        std::string hash = IntegrityManager::ComputeSHA256(json_report);
        std::cout << "[SHIA-Integrity] Computed SHA-256 Fingerprint: " << hash << std::endl;

        // Dispatch blockchain anchoring to background pool
        // Using a mock incident ID based on timestamp for dummy correlation
        std::string mock_incident_id = report.timestamp + "_" + rule.service;
        BlockchainBridge::SubmitAuditHash(mock_incident_id, hash, worker_pool.get());

    });

    // Load Configuration
    std::string config_path = "core/config/shia_config.json"; // Default or read from args
    LoadConfig(config_path, tailer, cb, enable_ai_repair);

    // Start Monitoring
    tailer.Start();

    std::cout << "[SHIA-Engine] Agent running. Press Ctrl+C to exit." << std::endl;

    // Keep main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    worker_pool->Shutdown();
    return 0;
}
