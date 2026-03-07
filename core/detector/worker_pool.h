#ifndef WORKER_POOL_H
#define WORKER_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class WorkerPool {
public:
    WorkerPool(size_t num_threads);
    ~WorkerPool();

    void EnqueueTask(std::function<void()> task);
    void Shutdown();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

#endif // WORKER_POOL_H
