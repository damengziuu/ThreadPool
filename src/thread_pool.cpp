#include "thread_pool.hpp"

namespace thread_utils {

/**
 * @brief Constructs a thread pool object.
 * 
 * @param initial_thread_count The initial number of threads in the thread pool.
 * @param max_task_count The maximum number of tasks that can be queued in the thread pool. 
 *                       If set to 0, there is no limit on the number of tasks.
 */
thread_pool::thread_pool(
    std::size_t initial_thread_count, 
    std::size_t max_task_count
) :
    max_task_count(max_task_count), 
    status(status_t::RUNNING)
{   
    for (std::size_t i = 0; i < initial_thread_count; ++i)
    {   // 创建线程并插入线程列表
        worker_list.emplace_back(this);     // 隐式调用worker_thread::worker_thread(thread_pool* pool)构造函数
    }
}

thread_pool::~thread_pool()
{
    terminate();    // 析构时终止线程池
}

/**
 * @brief Pauses the thread pool.
 * 
 * This function sets the status of the thread pool to "paused" and pauses all the worker threads.
 * 
 * @note This function is thread-safe.
 */
void thread_pool::pause_with_status_lock()
{
    switch (status.load())
    {
    case status_t::TERMINATED: // 线程池已终止
    case status_t::TERMINATING: // 线程池将终止
    case status_t::PAUSED: // 线程池被暂停
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
        return;     // 在这些状态下，不需要进一步操作
    case status_t::RUNNING: // 线程池正在运行
        status.store(status_t::PAUSED);    // 设置线程池的状态为暂停
        break;
    default:
        throw std::runtime_error("[thread_pool::pause][error]: unknown status");
    }
    std::unique_lock<std::shared_mutex> lock(worker_list_mutex);
    for (auto& worker : worker_list)
    {   // 暂停所有线程
        worker.pause();
    }
}

/**
 * Resumes the execution of the thread pool.
 * Sets the status of the thread pool to running and resumes all worker threads.
 */
void thread_pool::resume_with_status_lock()
{
    switch (status.load())
    {
    case status_t::TERMINATED: // 线程池已终止
    case status_t::TERMINATING: // 线程池将终止
    case status_t::RUNNING: // 线程池正在运行
        return;     // 在这些状态下，不需要恢复线程
    case status_t::PAUSED: // 线程池被暂停
        status.store(status_t::RUNNING);    // 设置线程池的状态为运行
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
        break;
    default:    // 未知状态
        throw std::runtime_error("[thread_pool::resume][error]: unknown status");
    }
    std::unique_lock<std::shared_mutex> lock(worker_list_mutex);
    for (auto& worker : worker_list)
    {   // 恢复所有线程
        worker.resume();
    }
}

void thread_pool::shutdown_with_status_lock()
{   // 等待所有任务执行完毕后再终止线程池   // 设置线程池的状态为等待任务完成
    switch (status.load())
    {
    case status_t::TERMINATED: // 线程池已终止
    case status_t::TERMINATING: // 线程池将终止
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
        return;     // 在这些状态下，不需要额外操作
    case status_t::PAUSED: // 线程池被暂停
        resume_with_status_lock();  // 恢复线程池运行，以便任务能够继续执行以消耗任务队列中的任务
    case status_t::RUNNING: // 线程池正在运行
        status.store(status_t::SHUTDOWN);    // 设置线程池的状态为等待任务完成
        break;
    default:    // 未知状态
        throw std::runtime_error("[thread_pool::shutdown][error]: unknown status");
    }
    std::shared_lock<std::shared_mutex> lock(task_queue_mutex);
    while (!task_queue.empty())
    {   // 等待任务队列为空
        task_queue_empty_cv.wait(lock);
    }
    terminate_with_status_lock();
}

void thread_pool::wait_with_status_lock()
{   // 等待所有任务执行完毕
    switch (status.load())
    {
    case status_t::TERMINATED:  // 线程池已终止
        return;     // 在这种状态下，不需要等待
    case status_t::TERMINATING: // 线程池将终止
    case status_t::PAUSED: // 线程池被暂停
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
    case status_t::RUNNING: // 线程池正在运行
        break;
    default:
        throw std::runtime_error("[thread_pool::wait][error]: unknown status");
    }
    std::shared_lock<std::shared_mutex> lock(task_queue_mutex);
    while (!task_queue.empty())
    {   // 等待任务队列为空
        task_queue_empty_cv.wait(lock);
    }
}

void thread_pool::wait()
{   // 等待所有任务执行完毕
    std::shared_lock<std::shared_mutex> lock(status_mutex); // wait操作不会改变线程池的状态，因此只需要共享锁
    wait_with_status_lock();
}

/**
 * @brief Terminates the thread pool.
 * 
 * This function sets the status of the thread pool to terminate and terminates all worker threads.
 * It also notifies all blocked threads waiting for tasks in the task queue.
 */
void thread_pool::terminate_with_status_lock()
{
    switch (status.load())
    {
    case status_t::TERMINATED:  // 线程池已终止
        return;     // 在这种状态下，不需要进一步操作
    case status_t::TERMINATING: // 线程池将终止
        break;
    case status_t::RUNNING: // 线程池正在运行
    case status_t::PAUSED: // 线程池被暂停
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
        status.store(status_t::TERMINATING);    // 设置线程池的状态为将终止
        break;
    default:    // 未知状态
        throw std::runtime_error("[thread_pool::terminate][error]: unknown status");
    }
    std::unique_lock<std::shared_mutex> lock(worker_list_mutex);
    for (auto& worker : worker_list)
    {   // 终止所有线程
        worker.terminate();
    }
    task_queue_cv.notify_all(); // 唤醒所有等待任务的阻塞线程
    status.store(status_t::TERMINATED);    // 设置线程池的状态为已终止
}

void thread_pool::shutdown()
{   // 等待所有任务执行完毕后再终止线程池
    std::unique_lock<std::shared_mutex> lock(status_mutex);
    shutdown_with_status_lock();
}

inline void thread_pool::shutdown_now_with_status_lock()
{   // 立即终止线程池
    terminate_with_status_lock();
}

void thread_pool::shutdown_now()
{   // 立即终止线程池
    std::unique_lock<std::shared_mutex> lock(status_mutex);
    shutdown_now_with_status_lock();
}

void thread_pool::pause()
{   // 暂停线程池
    std::unique_lock<std::shared_mutex> lock(status_mutex);
    pause_with_status_lock();
}

void thread_pool::resume()
{   // 恢复线程池
    std::unique_lock<std::shared_mutex> lock(status_mutex);
    resume_with_status_lock();
}

void thread_pool::terminate()
{   // 终止线程池
    std::unique_lock<std::shared_mutex> lock(status_mutex);
    terminate_with_status_lock();
}

/**
 * Adds a specified number of threads to the thread pool.
 *
 * @param count_to_add The number of threads to add.
 */
void thread_pool::add_thread(std::size_t count_to_add)
{   // 添加线程
    std::shared_lock<std::shared_mutex> lock_status(status_mutex);
    switch (status.load())
    {
    case status_t::TERMINATED: // 线程池已终止
    case status_t::TERMINATING: // 线程池将终止
    case status_t::PAUSED: // 线程池被暂停
        return;     // 在这些状态下，不允许添加线程
    case status_t::RUNNING: // 线程池正在运行
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
        break;
    default:
        throw std::runtime_error("[thread_pool::add_thread][error]: unknown status");
    }
    std::unique_lock<std::shared_mutex> lock_worker_list(worker_list_mutex);
    for (std::size_t i = 0; i < count_to_add; ++i)
    {
        worker_list.emplace_back(this);
    }
}

/**
 * Removes a specified number of threads from the thread pool.
 * 
 * @param count_to_remove The number of threads to remove.
 * 
 * This function removes the minimum of `count_to_remove` and the current number of threads in the thread pool.
 * It terminates the removed threads and removes them from the worker list.
 * After removing the threads, it notifies all waiting threads in the task queue to check if any threads have been terminated.
 */
void thread_pool::remove_thread(std::size_t count_to_remove)
{   // 移除min(count_to_remove, worker_list.size())个线程
    std::shared_lock<std::shared_mutex> lock_status(status_mutex);
    switch (status.load())
    {
    case status_t::TERMINATED: // 线程池已终止
    case status_t::TERMINATING: // 线程池将终止
    case status_t::PAUSED: // 线程池被暂停
        return;     // 在这些状态下，不允许移除线程
    case status_t::RUNNING: // 线程池正在运行
    case status_t::SHUTDOWN: // 线程池在等待任务完成，但不再接受新任务
        break;
    default:
        throw std::runtime_error("[thread_pool::remove_thread][error]: unknown status");
    }
    std::unique_lock<std::shared_mutex> lock_worker_list(worker_list_mutex);
    count_to_remove = std::min(count_to_remove, worker_list.size());
    auto it = worker_list.end();
    for (std::size_t i = 0; i < count_to_remove; ++i)
    {
        --it;
        it->terminate();
    }
    task_queue_cv.notify_all(); // 唤醒所有等待任务的阻塞线程，以便它们能够检查线程是否被终止
    worker_list.erase(it, worker_list.end());   // 从线程列表中移除工作线程对象并析构
}


std::size_t thread_pool::get_thread_count()
{   // 获取线程池中线程的数量
    std::shared_lock<std::shared_mutex> lock(worker_list_mutex);
    return worker_list.size();
}

std::size_t thread_pool::get_task_count()
{   // 获取任务队列中任务的数量
    std::shared_lock<std::shared_mutex> lock(task_queue_mutex);
    return task_queue.size();
}

} // namespace thread_utils