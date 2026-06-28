#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T> class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(size_t max_capacity);

    bool push(T value, std::stop_token st = {});
    bool pop(T& value, std::stop_token st = {});

    // delete copy and move constructor & operators
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

private:
    size_t m_max_capacity{10};
    std::queue<T> m_data_queue;
    mutable std::mutex m_queue_mutex;
    std::condition_variable_any m_cv_not_empty;
    std::condition_variable_any m_cv_not_full;
};

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(size_t max_capacity) : m_max_capacity{max_capacity}
{
}

// returns true if push was a success, returns false if st.stop_requested() is true
template <typename T> bool ThreadSafeQueue<T>::push(T value, std::stop_token st)
{
    {
        std::unique_lock lk{m_queue_mutex};

        m_cv_not_full.wait(lk, st, [this] { return m_data_queue.size() < m_max_capacity; });

        if (st.stop_requested()) { return false; }

        m_data_queue.push(std::move(value));
    }

    m_cv_not_empty.notify_one();
    return true;
}

// returns true if pop was a success, returns false if st.stop_requested() is true
template <typename T> bool ThreadSafeQueue<T>::pop(T& value, std::stop_token st)
{
    {
        std::unique_lock lk{m_queue_mutex};

        m_cv_not_empty.wait(lk, st, [this] { return !m_data_queue.empty(); });

        if (st.stop_requested()) { return false; }

        value = m_data_queue.front();

        m_data_queue.pop();
    }

    m_cv_not_full.notify_one();
    return true;
}
