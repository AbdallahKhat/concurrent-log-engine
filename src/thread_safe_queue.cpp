#include "thread_safe_queue.hpp"

ThreadSafeQueue::ThreadSafeQueue(size_t max_capacity) : m_max_capacity{max_capacity} {}

void ThreadSafeQueue::push(std::string log_line)
{
    std::unique_lock lk{m_queue_mutex};

    m_cv_not_full.wait(lk, [this] { return m_data_queue.size() < m_max_capacity; });

    m_data_queue.push(std::move(log_line));

    m_cv_not_empty.notify_one();
}

void ThreadSafeQueue::pop(std::string& value)
{
    std::unique_lock lk{m_queue_mutex};

    m_cv_not_empty.wait(lk, [this] { return !m_data_queue.empty(); });

    value = m_data_queue.front();

    m_data_queue.pop();
}
