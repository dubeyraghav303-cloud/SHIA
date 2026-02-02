#include <iostream>

int main() {
    std::cout << "[Demo] Running Bad Jump (Control Flow Violation)..." << std::endl;
    
    // Define a function pointer to NULL or garbage
    void (*bad_func)() = (void(*)())0x0;
    
    std::cout << "[Demo] Jumping to invalid address..." << std::endl;
    
    // expected: Runtime should see this is a Branch/Call instruction (BLR/RET etc)
    // and REFUSE to skip it.
    bad_func();
    
    std::cout << "[Demo] Survived? (Should NOT happen)" << std::endl;
    return 0;
}
