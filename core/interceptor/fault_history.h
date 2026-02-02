#ifndef FAULT_HISTORY_H
#define FAULT_HISTORY_H

#include <map>
#include <iostream>
#include <cstdint>
#include <mach/mach.h>
#include <mach/arm/thread_status.h>
#include "progress_tracker.h"

class FaultHistory {
public:
    // Returns true if we should ABORT (Escalate)
    bool CheckEscalation(uint64_t thread_id, uintptr_t rip, const arm_thread_state64_t& state) {
        fault_counts[rip]++;
        
        bool is_progressing = tracker.RecordAndCheckProgress(thread_id, rip, state);
        
        // Phase 2 Rule: > 3 Repeats
        if (fault_counts[rip] > 3) {
            if (!is_progressing) {
                 std::cerr << "[SHCR] Escalation: Non-Progressing Fault Loop." << std::endl;
                 return true;
            }
            
            // If progressing, allow more retries (up to 100)
            if (is_progressing) {
                if (fault_counts[rip] > 100) {
                     std::cerr << "[SHCR] Escalation: Excessive Fault Frequency (>100) despite progress." << std::endl;
                     return true;
                }
                return false; 
            }
        }
        return false;
    }

private:
    std::map<uintptr_t, int> fault_counts;
    ProgressTracker tracker;
};

#endif
