#include <iostream>

int main() {
    std::cout << "[Demo] Running Use After Free..." << std::endl;
    int* ptr = new int(42);
    delete ptr;
    volatile int val = *ptr; // Undefined Behavior, possibly crash if page unmapped
    std::cout << "[Demo] Survived! Val: " << val << std::endl;
    return 0;
}
