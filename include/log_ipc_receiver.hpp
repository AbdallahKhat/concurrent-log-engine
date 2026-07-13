#pragma once

#include "ipc_config.hpp"
#include "thread_safe_queue.hpp"
#include <fcntl.h>
#include <mqueue.h>
#include <string>

class LogIPCReceiver
{
public:
    LogIPCReceiver(ThreadSafeQueue<std::string>& log_queue, const char* message_queue_name);
    ~LogIPCReceiver();

    LogIPCReceiver(const LogIPCReceiver&) = delete;
    LogIPCReceiver& operator=(const LogIPCReceiver&) = delete;
    LogIPCReceiver(LogIPCReceiver&&) = delete;
    LogIPCReceiver& operator=(LogIPCReceiver&&) = delete;

    bool is_valid() { return m_valid_ipc; }

    void start_receiving(std::stop_token st);

private:
    ThreadSafeQueue<std::string>& m_log_queue;
    mqd_t m_mqdes{-1};
    bool m_valid_ipc{};
};