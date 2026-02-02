#include <iostream>
#include <vector>

int main() {
    std::cout << "[Demo] Running Out Of Bounds..." << std::endl;
    std::vector<int> v = {1, 2, 3};
    // Access way out of bounds to trigger SEGV
    volatile int val = v[100000]; 
    std::cout << "[Demo] Survived! Val: " << val << std::endl;
    return 0;
}
