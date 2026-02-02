#include "patch_apply.h"
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <iostream>

bool PatchTransformer::ApplyPatch(pid_t target_pid, uintptr_t address, const std::vector<uint8_t>& patch_bytes) {
    kern_return_t kr;
    mach_port_t task;

    kr = task_for_pid(mach_task_self(), target_pid, &task);
    if (kr != KERN_SUCCESS) {
        std::cerr << "Failed to get task for pid " << target_pid << ": " << kr << std::endl;
        return false;
    }

    // Must change protection to write
    kr = mach_vm_protect(task, address, patch_bytes.size(), FALSE, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    if (kr != KERN_SUCCESS) {
         std::cerr << "[SHCR-Error] mach_vm_protect (RW) failed: " << kr << " (" << mach_error_string(kr) << ") at " << std::hex << address << std::dec << std::endl;
         // Continue trying to write? Unlikely to succeed if protect failed.
    }

    kr = mach_vm_write(task, address, (vm_offset_t)patch_bytes.data(), patch_bytes.size());
    
    if (kr != KERN_SUCCESS) {
        std::cerr << "[SHCR-Error] mach_vm_write failed: " << kr << " (" << mach_error_string(kr) << ") at " << std::hex << address << std::dec << std::endl;
        return false;
    }

    // Verify Patch Logic: Read back
    {
        std::vector<uint8_t> read_buf(patch_bytes.size());
        mach_vm_size_t read_size = patch_bytes.size();
        kr = mach_vm_read_overwrite(task, address, read_size, (mach_vm_address_t)read_buf.data(), &read_size);
        if (kr != KERN_SUCCESS) {
             std::cerr << "[SHCR-Verify] Failed to read back patch: " << kr << std::endl;
        } else {
            bool matches = true;
            for(size_t i=0; i<patch_bytes.size(); ++i) {
                if(read_buf[i] != patch_bytes[i]) matches = false;
            }
            if(!matches) {
                std::cerr << "[SHCR-Verify] CRITICAL: Patch verification failed! Memory mismatch." << std::endl;
                return false;
            } else {
                std::cout << "[SHCR-Verify] Patch verified in memory." << std::endl;
            }
        }
    }

    // Restore Protection (Executable)
    mach_vm_protect(task, address, patch_bytes.size(), FALSE, VM_PROT_READ | VM_PROT_EXECUTE);

    // Flush instruction cache
    // There isn't a direct remote flush API easily accessible without dylib injection helper,
    // but usually mach_vm_write handles coherency reasonably well on x86. ARM64 might need help.
    // For this level of project, we assume write + protect update is sufficient for demo.
    
    std::cout << "[SHCR] Patch applied at " << std::hex << address << std::dec << std::endl;
    return true;
}
