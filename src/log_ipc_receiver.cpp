#include "log_ipc_receiver.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

LogIPCReceiver::LogIPCReceiver(ThreadSafeQueue<std::string>& log_queue,
                               const char* message_queue_name)
    : m_log_queue{log_queue}
{
    struct mq_attr attr{.mq_maxmsg = ipc::config::MAX_MSG_NUM,
                        .mq_msgsize = ipc::config::MAX_MSG_SIZE};

    m_mqdes = mq_open(message_queue_name, O_RDONLY | O_CREAT, ipc::config::QUEUE_PERMS, &attr);

    if (m_mqdes == (mqd_t)-1)
    {
        std::cerr << "Failed to create message queue: " << std::strerror(errno) << '\n';
        m_valid_ipc = false;
        return;
    }

    m_valid_ipc = true;
}

LogIPCReceiver::~LogIPCReceiver()
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

void LogIPCReceiver::start_receiving(std::stop_token st)
{
    std::string message;
    ssize_t bytes_read{};

    while (true)
    {
        message.resize(ipc::config::MAX_MSG_SIZE);

        struct timespec timeout = get_future_timeout(1);

        while (!st.stop_requested())
        {
            bytes_read =
                mq_timedreceive(m_mqdes, message.data(), ipc::config::MAX_MSG_SIZE, 0, &timeout);

            if (bytes_read == -1)
            {
                if (errno == ETIMEDOUT || errno == EINTR)
                {
                    timeout = get_future_timeout(1);
                    continue;
                }

                std::cerr << "mq_receive failed to receive log line package\n";
                return;
            }

            message.resize(bytes_read);
            break;
        }

        if (!m_log_queue.push(message, st)) { break; }
    }

    // Graceful shutdown sequence when stop token is triggered
    std::cerr << "[LogIPCReceiver] Stop token received. Halting IPC message Reception.\n";
}