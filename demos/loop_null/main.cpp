#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdint>

int main() {
    std::cout << "[Demo] Running Zombie Null Dereference (Non-Progress Test)..." << std::endl;
    
    // Aligned buffer for machine code
    // We use static to ensure it's in data segment/BSS, but we need page alignment for mprotect
    static uint32_t code[1024] __attribute__((aligned(4096)));
    
    // 1. LDR X0, [X0] (Offset 0) -> F9400000
    // If X0 is 0, this crashes.
    code[0] = 0xF9400000;
    
    // 2. RET -> D65F03C0
    code[1] = 0xD65F03C0;
    
    // Make executable
    mprotect(code, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    using Func = void(*)();
    Func f = (Func)code;
    
    while(true) {
        std::cout << "[Demo] Invoking crash..." << std::endl;
        
        // Zero out X0
        asm volatile("mov x0, #0");
        
        f();
        
        std::cout << "[Demo] Healed! Restoring bad instruction..." << std::endl;
        code[0] = 0xF9400000; // Overwrite NOP
        
        // Fast loop to trigger history threshold quickly
        usleep(10000); 
    }
    
    return 0;
}
