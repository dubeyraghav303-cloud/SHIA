#include <iostream>
#include <thread>
#include <chrono>

void safe_thread() {
    for (int i = 0; i < 5; ++i) {
        std::cout << "[Thread-Safe] Running... " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "[Thread-Safe] Finished successfully." << std::endl;
}

void crashing_thread() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[Thread-Crash] About to crash..." << std::endl;
    int* ptr = nullptr;
    *ptr = 0xDEAD;
    std::cout << "[Thread-Crash] Resumed exec! (Should stop if thread killed)" << std::endl;
}

int main() {
    std::cout << "[Demo] Running Multi-threaded Fault..." << std::endl;
    
    std::thread t1(safe_thread);
    std::thread t2(crashing_thread);
    
    t1.join();
    t2.join();
    
    std::cout << "[Demo] Main finished." << std::endl;
    return 0;
}
