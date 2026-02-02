#include <iostream>

// Const global variable is typically placed in __TEXT const or __DATA const, which is read-only.
const int global_const = 100;

int main() {
    std::cout << "[Demo] Running Read-Only Memory Write..." << std::endl;
    
    // Cast away constness to attempt write
    int* ptr = (int*)&global_const;
    
    std::cout << "[Demo] Attempting to write to const memory address: " << ptr << std::endl;
    
    // This should trigger SIGSEGV (SEGV_ACCERR)
    *ptr = 200;
    
    std::cout << "[Demo] Survived! New value: " << global_const << std::endl;
    return 0;
}
