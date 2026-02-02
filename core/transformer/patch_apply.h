#pragma once
#include <vector>
#include <cstdint>
#include <sys/types.h>

class PatchTransformer {
public:
    static bool ApplyPatch(pid_t target_pid, uintptr_t address, const std::vector<uint8_t>& patch_bytes);
};
