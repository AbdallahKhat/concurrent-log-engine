#include "ipc_config.hpp"
#include "log_analyzer.hpp"
#include "log_ipc_receiver.hpp"
#include "log_reader.hpp"
#include "log_writer.hpp"
#include "thread_safe_queue.hpp"
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <thread>

sigset_t block_shutdown_signals()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);

    // Block signals so they get queued instead of forcefully killing the app
    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    return set;
}

bool unlink_message_queue_if_exists()
{
    if (mq_unlink(ipc::config::queue_name.data()) == -1 && errno != ENOENT)
    {
        std::cerr << "Failed to unlink message queue: " << std::strerror(errno) << '\n';
        return false;
    }
    return true;
}

int main()
{
    sigset_t set = block_shutdown_signals();

    ThreadSafeQueue<std::string> filtered_issue_queue;

    // Clear stale message queue on startup
    if (!unlink_message_queue_if_exists()) { return EXIT_FAILURE; }

    LogIPCReceiver log_ipc_receiver{filtered_issue_queue, ipc::config::queue_name.data()};
    if (!log_ipc_receiver.is_valid()) { return EXIT_FAILURE; }

    std::stop_source shutdown_source;
    std::stop_token shutdown_tk = shutdown_source.get_token();

    std::jthread ipc_receiver_thread(&LogIPCReceiver::start_receiving, &log_ipc_receiver,
                                     shutdown_tk);

    std::jthread writer_thread(LogWriter{filtered_issue_queue, "./output.log"}, shutdown_tk);

    int signal_number;
    sigwait(&set, &signal_number);

    std::cerr << "\n[INFO] Signal (" << signal_number
              << ") received. Initiating graceful engine shutdown...\n";

    shutdown_source.request_stop();
    if (!unlink_message_queue_if_exists()) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}
