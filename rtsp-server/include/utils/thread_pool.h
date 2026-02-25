#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <atomic>

/**
 * ThreadPool 类：通用线程池实现
 * 支持异步任务提交和结果获取
 */
class ThreadPool {
public:
    /**
     * 构造函数：创建线程池
     * @param thread_count 线程数量，默认为4
     */
    explicit ThreadPool(size_t thread_count = 4);
    
    /**
     * 析构函数：等待所有任务完成后销毁线程池
     */
    ~ThreadPool();

    // 禁止拷贝构造和拷贝赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * 提交任务到线程池
     * @param func 可调用对象（函数、lambda等）
     * @param Args 函数参数
     * @return 返回future对象，用于获取任务结果
     */
    template<typename Func, typename... Args>
    auto enqueue(Func&& func, Args&&... args) 
        -> std::future<typename std::result_of<Func(Args...)>::type>;

    /**
     * 获取线程池大小
     * @return 线程数量
     */
    size_t size() const;

    /**
     * 获取当前待处理任务数量
     * @return 任务队列长度
     */
    size_t pending_tasks() const;

    /**
     * 停止线程池（不再接受新任务）
     */
    void stop();

private:
    /**
     * 工作线程函数
     */
    void worker();

    /**
     * 执行任务队列中的任务
     */
    void run();

private:
    // 工作线程列表
    std::vector<std::thread> workers_;
    
    // 任务队列（使用std::function实现通用任务）
    struct Task {
        virtual ~Task() {}
        virtual void execute() = 0;
    };
    
    template<typename Func>
    struct TaskImpl : Task {
        Func func_;
        TaskImpl(Func&& func) : func_(std::forward<Func>(func)) {}
        void execute() override { func_(); }
    };
    
    std::queue<std::unique_ptr<Task>> tasks_;
    
    // 任务队列互斥锁
    mutable std::mutex queue_mutex_;
    
    // 条件变量（用于任务通知）
    std::condition_variable condition_;
    
    // 线程池停止标志
    std::atomic<bool> stop_;
    
    // 线程数量
    size_t thread_count_;
};

// 模板方法实现
template<typename Func, typename... Args>
auto ThreadPool::enqueue(Func&& func, Args&&... args)
    -> std::future<typename std::result_of<Func(Args...)>::type>
{
    // 定义返回类型
    using ReturnType = typename std::result_of<Func(Args...)>::type;
    
    // 创建共享状态（promise + task）
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );
    
    std::future<ReturnType> result = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // 如果线程池已停止，抛出异常
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        
        // 将任务添加到队列
        tasks_.emplace([task]() {
            (*task)();
        });
    }
    
    // 通知一个工作线程
    condition_.notify_one();
    
    return result;
}

#endif // THREADPOOL_H
