#include "log_analyzer.hpp"
#include "log_reader.hpp"
#include "log_writer.hpp"
#include "thread_safe_queue.hpp"
#include <csignal>
#include <iostream>
#include <pthread.h>
#include <thread>

int main()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);

    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    std::stop_source shutdown_source;

    ThreadSafeQueue<std::string> raw_log_queue;
    ThreadSafeQueue<std::string> filtered_issue_queue;

    std::stop_token shutdown_tk = shutdown_source.get_token();

    std::jthread reader_thread(LogReader{raw_log_queue, "../input_file.txt"}, shutdown_tk);
    std::jthread analyzer_thread_1(LogAnalyzer{raw_log_queue, filtered_issue_queue}, shutdown_tk);
    std::jthread analyzer_thread_2(LogAnalyzer{raw_log_queue, filtered_issue_queue}, shutdown_tk);
    std::jthread writer_thread(LogWriter{filtered_issue_queue, "./output.log"}, shutdown_tk);

    int signal_number;
    sigwait(&set, &signal_number);

    std::cerr << "\n[INFO] Signal (" << signal_number
              << ") received. Initiating graceful engine shutdown...\n";

    shutdown_source.request_stop();

    return 0;
}
