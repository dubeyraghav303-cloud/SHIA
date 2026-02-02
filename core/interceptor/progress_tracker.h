#ifndef PROGRESS_TRACKER_H
#define PROGRESS_TRACKER_H

#include <map>
#include <vector>
#include <iostream>
#include <mach/mach.h>
#include <mach/arm/thread_status.h>

class ProgressTracker {
private:
    // Tid -> RIP -> History of register states
    // Keeping last 5 snapshots for each RIP 
    struct Snapshot {
        arm_thread_state64_t regs;
    };
    
    std::map<uint64_t, std::map<uintptr_t, std::vector<Snapshot>>> history;
    const size_t HISTORY_LIMIT = 5;

    bool AreRegistersEqual(const arm_thread_state64_t& a, const arm_thread_state64_t& b) {
        // Compare General Purpose Registers X0-X28
        for(int i=0; i<29; ++i) {
            if(a.__x[i] != b.__x[i]) return false;
        }
        if(a.__fp != b.__fp) return false;
        if(a.__lr != b.__lr) return false;
        if(a.__sp != b.__sp) return false;
        if(a.__pc != b.__pc) return false;
        // cpsr changes (e.g. flags) might be progress, but usually loop has side effects on GPRs
        // Let's be strict: if GPRs match, it's non-progress.
        return true;
    }

public:
    // Returns TRUE if execution is PROGRESSING
    // Returns FALSE if execution is STALLED (Non-Progress)
    bool RecordAndCheckProgress(uint64_t thread_id, uintptr_t rip, const arm_thread_state64_t& current_state) {
        auto& snapshots = history[thread_id][rip];
        
        // Add current
        Snapshot s;
        s.regs = current_state;
        snapshots.push_back(s);
        
        if (snapshots.size() > HISTORY_LIMIT) {
            snapshots.erase(snapshots.begin()); // Keep last N
        }
        
        // Check if we have enough history to decide
        if (snapshots.size() < 2) return true; // Assume progress
        
        // Compare latest vs ALL previous in buffer?
        // Or just latest vs previous?
        // Prompt says "If identical for N consecutive iterations".
        // Let's check if ALL snapshots in history are identical.
        
        if (snapshots.size() >= HISTORY_LIMIT) {
            bool all_identical = true;
            for(size_t i=0; i < snapshots.size() - 1; ++i) {
                if (!AreRegistersEqual(snapshots[i].regs, current_state)) {
                    all_identical = false;
                    break;
                }
            }
            
            if (all_identical) {
                std::cerr << "[SHCR] NON-PROGRESSING EXECUTION DETECTED." << std::endl;
                std::cerr << "[SHCR] Thread " << thread_id << " at RIP " << std::hex << rip << std::dec << std::endl;
                std::cerr << "[SHCR] Registers have remained identical for " << HISTORY_LIMIT << " consecutive faults." << std::endl;
                return false; 
            }
        }
        
        return true;
    }
};

#endif
