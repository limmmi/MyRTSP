#include "utils/thread_pool.h"
#include <spdlog/spdlog.h>

ThreadPool::ThreadPool(size_t thread_count)
    : stop_(false), thread_count_(thread_count) {
    
    if (thread_count == 0) {
        throw std::runtime_error("Thread count must be greater than 0");
    }
    
    // 创建工作线程
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back(&ThreadPool::worker, this);
    }
    
    spdlog::info("ThreadPool created with {} threads", thread_count);
}

ThreadPool::~ThreadPool() {
    // 停止线程池
    stop();
    
    // 等待所有线程完成
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    spdlog::info("ThreadPool destroyed");
}

size_t ThreadPool::size() const {
    return thread_count_;
}

size_t ThreadPool::pending_tasks() const {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    
    // 通知所有线程唤醒并退出
    condition_.notify_all();
}

void ThreadPool::worker() {
    run();
}

void ThreadPool::run() {
    while (true) {
        std::unique_ptr<Task> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // 等待任务或停止信号
            condition_.wait(lock, [this] {
                return stop_ || !tasks_.empty();
            });
            
            // 如果线程池已停止且任务队列为空，退出线程
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            // 取出任务
            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
            }
        }
        
        // 执行任务
        if (task) {
            try {
                task->execute();
            } catch (const std::exception& e) {
                spdlog::error("ThreadPool task execution error: {}", e.what());
            } catch (...) {
                spdlog::error("ThreadPool task execution error: unknown exception");
            }
        }
    }
}
