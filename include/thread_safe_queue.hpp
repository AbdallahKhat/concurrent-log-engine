#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T> class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(size_t max_capacity);

    void push(T value);
    void pop(T& value);

    // delete copy and move constructor & operators
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

private:
    size_t m_max_capacity{10};
    std::queue<T> m_data_queue;
    mutable std::mutex m_queue_mutex;
    std::condition_variable m_cv_not_empty;
    std::condition_variable m_cv_not_full;
};

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(size_t max_capacity) : m_max_capacity{max_capacity}
{
}

template <typename T> void ThreadSafeQueue<T>::push(T value)
{
    {
        std::unique_lock lk{m_queue_mutex};

        m_cv_not_full.wait(lk, [this] { return m_data_queue.size() < m_max_capacity; });

        m_data_queue.push(std::move(value));
    }

    m_cv_not_empty.notify_one();
}

template <typename T> void ThreadSafeQueue<T>::pop(T& value)
{
    {
        std::unique_lock lk{m_queue_mutex};

        m_cv_not_empty.wait(lk, [this] { return !m_data_queue.empty(); });

        value = m_data_queue.front();

        m_data_queue.pop();
    }

    m_cv_not_full.notify_one();
}
