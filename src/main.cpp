#include "log_analyzer.hpp"
#include "log_reader.hpp"
#include "log_writer.hpp"
#include "thread_safe_queue.hpp"
#include <iostream>
#include <thread>

int main()
{
    ThreadSafeQueue<std::string> raw_log_queue;
    ThreadSafeQueue<std::string> filtered_issue_queue;

    std::jthread reader_thread(LogReader{raw_log_queue, "../input_file.txt"});
    std::jthread analyzer_thread_1(LogAnalyzer{raw_log_queue, filtered_issue_queue});
    std::jthread analyzer_thread_2(LogAnalyzer{raw_log_queue, filtered_issue_queue});
    std::jthread writer_thread(LogWriter{filtered_issue_queue, "./output.log"});

    while (true) { std::this_thread::sleep_for(std::chrono::seconds(10)); }

    return 0;
}
