#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "[Demo] Running Infinite Loop..." << std::endl;
    int counter = 0;
    while(true) {
        counter++;
        if (counter % 1000000 == 0) std::cout << "." << std::flush;
        // In real demo, this runs forever until patched
    }
    std::cout << "[Demo] Loop Broken!" << std::endl;
    return 0;
}
