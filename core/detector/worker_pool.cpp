#include "worker_pool.h"

WorkerPool::WorkerPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop.load() || !this->tasks.empty();
                    });
                    
                    if (this->stop.load() && this->tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

void WorkerPool::EnqueueTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop.load()) return;
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void WorkerPool::Shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop.store(true);
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

WorkerPool::~WorkerPool() {
    Shutdown();
}
