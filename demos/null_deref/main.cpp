#include <iostream>

void trigger_crash() {
    int* ptr = nullptr;
    volatile int val = *ptr; // Crash here
}

int main() {
    std::cout << "[Demo] Running Null Dereference..." << std::endl;
    trigger_crash();
    std::cout << "[Demo] Survived! Execution continued after crash function returned." << std::endl;
    return 0;
}
