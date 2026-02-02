#include "context_analyzer.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mach/mach.h>
#include <mach/mach_vm.h>

std::string ContextAnalyzer::SerializeContext(const CrashContext& ctx) {
    std::stringstream ss;
    ss << "{";
    ss << "\"signal\": \"" << ctx.signal_type << "\",";
    ss << "\"address\": " << ctx.fault_address << ",";
    ss << "\"rip\": " << ctx.instruction_pointer << ",";
    ss << "\"instruction\": " << ctx.instruction_bytes;
    ss << "}";
    return ss.str();
}

CrashContext ContextAnalyzer::Analyze(int pid, int signal_code, uintptr_t address, uintptr_t ip) {
    CrashContext ctx;
    ctx.fault_address = address;
    ctx.instruction_pointer = ip;
    
    // Determine signal name
    if (signal_code == 11) ctx.signal_type = "SIGSEGV"; // Mapping raw BSD signal
    else if (signal_code == 8) ctx.signal_type = "SIGFPE";
    else if (signal_code == 4) ctx.signal_type = "SIGILL";
    else ctx.signal_type = "UNKNOWN";

    // Read memory at RIP to get instruction bytes
    // We need 'task_for_pid' but we only have 'pid'. 
    // In a real design, we'd pass the task port. 
    // For now, we unfortunately have to get the task port again (overhead) or change signature.
    // Let's re-get it for simplicity as we are in "Enhancement" mode.
    
    mach_port_t task;
    kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
    if (kr == KERN_SUCCESS) {
        uint32_t instruction = 0;
        mach_vm_size_t size = sizeof(instruction);
        kr = mach_vm_read_overwrite(task, ip, size, (mach_vm_address_t)&instruction, &size);
        if (kr == KERN_SUCCESS) {
            ctx.instruction_bytes = instruction;
        } else {
             std::cerr << "[SHCR-Analyzer] Failed to read instruction bytes: " << kr << std::endl;
        }
    } else {
        std::cerr << "[SHCR-Analyzer] Failed to get task for pid " << pid << ": " << kr << std::endl;
    }

    return ctx;
}
