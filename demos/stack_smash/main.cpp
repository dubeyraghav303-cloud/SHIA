#include <iostream>
#include <vector>

void infinite_recursion(int depth) {
    if (depth % 1000 == 0) {
        // std::cout << "Depth: " << depth << std::endl; 
    }
    char buffer[1024]; // Consume stack
    buffer[0] = 'A'; 
    infinite_recursion(depth + 1);
}

int main() {
    std::cout << "[Demo] Running Stack Smash (Infinite Recursion)..." << std::endl;
    std::cout << "[Demo] This triggers SIGSEGV (SEGV_ACCERR) at stack guard page." << std::endl;
    
    // expected: Runtime detects stack pointer near limit or mapped region end?
    // Or simply, this is hard to "Patch". NOPing the push/sub won't help much if SP is exhausted.
    // Runtime should probably Abort.
    infinite_recursion(0);
    
    return 0;
}
