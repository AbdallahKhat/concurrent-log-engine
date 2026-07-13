#pragma once

#include "thread_safe_queue.hpp"
#include <fcntl.h>
#include <mqueue.h>
#include <string>

class LogIPCSender
{
public:
    LogIPCSender(ThreadSafeQueue<std::string>& log_queue, const char* message_queue_name);
    ~LogIPCSender();

    LogIPCSender(const LogIPCSender&) = delete;
    LogIPCSender& operator=(const LogIPCSender&) = delete;
    LogIPCSender(LogIPCSender&&) = delete;
    LogIPCSender& operator=(LogIPCSender&&) = delete;

    bool is_valid() { return m_valid_ipc; }

    void start_sending(std::stop_token st);

private:
    ThreadSafeQueue<std::string>& m_log_queue;
    mqd_t m_mqdes{-1};
    bool m_valid_ipc{};
};