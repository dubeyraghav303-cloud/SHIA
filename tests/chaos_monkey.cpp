#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <string>

void StartScenarioA() {
    std::cout << "[Chaos Monkey] 🐒 Executing Scenario A (Service Crash)..." << std::endl;
    // We write "Segmentation Fault" to our simulated syslog.
    std::ofstream file("/tmp/test_syslog.log", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open /tmp/test_syslog.log" << std::endl;
        return;
    }
    file << "[CRITICAL] dummy_service: Segmentation Fault (core dumped)" << std::endl;
    std::cout << "[Chaos Monkey] 💀 Fatal crash injected." << std::endl;
}

void StartScenarioB() {
    std::cout << "[Chaos Monkey] 🐒 Executing Scenario B (Complex Configuration Poison)..." << std::endl;
    // Writes a convoluted missing dependency signature to trigger Gemini API rules
    std::ofstream file("/tmp/test_syslog.log", std::ios::app);
    if (!file.is_open()) return;
    file << "[ERROR] dummy_service: Config parsing failed. Found syntax error in JSON structure on Line 22." << std::endl;
    std::cout << "[Chaos Monkey] ☠️ Poison config written." << std::endl;
}

void StartScenarioC() {
    std::cout << "[Chaos Monkey] 🐒 Executing Scenario C (AI Patch Repair)..." << std::endl;
    // Writes an unknown error that forces SHIA to use AI for recovery
    std::ofstream file("/tmp/test_syslog.log", std::ios::app);
    if (!file.is_open()) return;
    file << "[FATAL] ai_demo_service: Unknown memory leak detected in module X." << std::endl;
    std::cout << "[Chaos Monkey] 🤖 Unknown error injected. Waiting for AI..." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./chaos_monkey [scenario_a | scenario_b]" << std::endl;
        return 1;
    }

    std::string scenario = argv[1];
    if (scenario == "scenario_a") {
        StartScenarioA();
    } else if (scenario == "scenario_b") {
        StartScenarioB();
    } else if (scenario == "scenario_c") {
        StartScenarioC();
    } else {
        std::cout << "Unknown scenario." << std::endl;
    }
    
    return 0;
}
