#include "log_ipc_sender.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

LogIPCSender::LogIPCSender(ThreadSafeQueue<std::string>& log_queue, const char* message_queue_name)
    : m_log_queue{log_queue}
{
    m_mqdes = mq_open(message_queue_name, O_WRONLY);

    if (m_mqdes == (mqd_t)-1)
    {
        std::cerr << "Failed to create message queue: " << std::strerror(errno) << '\n';
        m_valid_ipc = false;
        return;
    }

    m_valid_ipc = true;
}

LogIPCSender::~LogIPCSender()
{
    if (m_mqdes != (mqd_t)-1) { mq_close(m_mqdes); };
}

static struct timespec get_future_timeout(int seconds_to_add)
{
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += seconds_to_add; // Add exactly 1 second to the time target

    return timeout;
}

void LogIPCSender::start_sending(std::stop_token st)
{
    std::string log_line;
    while (true)
    {
        if (!m_log_queue.pop(log_line, st)) { break; }

        struct timespec timeout = get_future_timeout(1);

        while (!st.stop_requested())
        {
            if (mq_timedsend(m_mqdes, log_line.c_str(), log_line.size(), 0, &timeout) == -1)
            {
                if (errno == ETIMEDOUT || errno == EINTR)
                {
                    timeout = get_future_timeout(1);
                    continue;
                }

                std::cerr << "mq_send failed to dispatch log line package\n";
                return;
            }

            break;
        }
    }

    // Graceful shutdown sequence when stop token is triggered
    std::cerr << "[LogIPCSender] Stop token received. Halting IPC message dispatch.\n";
}