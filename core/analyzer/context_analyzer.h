#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct CrashContext {
    std::string signal_type;
    uintptr_t fault_address;
    uintptr_t instruction_pointer;
    uint32_t instruction_bytes;
    // Minimal register set
    uint64_t r_ax, r_bx, r_cx, r_dx, r_di, r_si, r_sp, r_bp;
};

class ContextAnalyzer {
public:
    static std::string SerializeContext(const CrashContext& ctx);
    static CrashContext Analyze(int pid, int signal_code, uintptr_t address, uintptr_t ip);
};
