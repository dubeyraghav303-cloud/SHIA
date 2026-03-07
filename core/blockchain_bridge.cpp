#include "blockchain_bridge.h"
#include <iostream>
#include <chrono>
#include <thread>

void BlockchainBridge::SubmitAuditHash(const std::string& incident_id, const std::string& hash, WorkerPool* pool) {
    if (!pool) return;
    
    pool->EnqueueTask([incident_id, hash]() {
        // Simulating async network delay and JSON-RPC block inclusion in the background thread.
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        
        std::cout << "\n[SHIA-Blockchain] Anchoring Hash: " << hash 
                  << " to simulated chain (Polygon Amoy)..." << std::endl;
                  
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        std::cout << "[SHIA-Blockchain] ✅ Audit anchored successfully for Incident ID: " << incident_id << std::endl;
        std::cout << "[SHIA-Blockchain] 🔗 Dummy TxHash: 0x811bda31af1cf2cb3c" << std::endl;
    });
}
