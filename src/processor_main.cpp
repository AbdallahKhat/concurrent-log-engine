#include "cli_parser.hpp"
#include "ipc_config.hpp"
#include "log_analyzer.hpp"
#include "log_ipc_sender.hpp"
#include "log_reader.hpp"
#include "log_writer.hpp"
#include "thread_safe_queue.hpp"
#include <csignal>
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

int main(int argc, char* argv[])
{
    const auto cli_cfg = cli::parse_processor_args(argc, argv);
    if (cli_cfg.should_exit) { return cli_cfg.exit_code; }
    if (!cli::validate_input_file(cli_cfg.path)) { return EXIT_FAILURE; }

    sigset_t set = block_shutdown_signals();

    ThreadSafeQueue<std::string> raw_log_queue;
    ThreadSafeQueue<std::string> filtered_issue_queue;

    LogIPCSender log_ipc_sender{filtered_issue_queue, ipc::config::queue_name.data()};
    if (!log_ipc_sender.is_valid()) { return EXIT_FAILURE; }

    std::stop_source shutdown_source;
    std::stop_token shutdown_tk = shutdown_source.get_token();

    std::jthread reader_thread(LogReader{raw_log_queue, cli_cfg.path.string()}, shutdown_tk);

    std::jthread analyzer_thread_1(LogAnalyzer{raw_log_queue, filtered_issue_queue}, shutdown_tk);
    std::jthread analyzer_thread_2(LogAnalyzer{raw_log_queue, filtered_issue_queue}, shutdown_tk);

    std::jthread ipc_sender_thread(&LogIPCSender::start_sending, &log_ipc_sender, shutdown_tk);

    int signal_number;
    sigwait(&set, &signal_number);

    std::cerr << "\n[INFO] Signal (" << signal_number
              << ") received. Initiating graceful engine shutdown...\n";

    shutdown_source.request_stop();

    return EXIT_SUCCESS;
}
