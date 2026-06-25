#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(size_t max_capacity);

    void push(std::string log_line);
    void pop(std::string& value);

    // delete copy and move constructor & operators
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

private:
    size_t m_max_capacity{10};
    std::queue<std::string> m_data_queue;
    mutable std::mutex m_queue_mutex;
    std::condition_variable m_cv_not_empty;
    std::condition_variable m_cv_not_full;
};
