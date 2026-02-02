#include <iostream>

int main() {
    std::cout << "[Demo] Running Divide by Zero..." << std::endl;
    int a = 10;
    volatile int b = 0;
    int result = a / b; // Crash here
    std::cout << "[Demo] Survived! Result: " << result << std::endl;
    return 0;
}
