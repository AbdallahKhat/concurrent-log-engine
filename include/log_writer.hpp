#pragma once

#include "thread_safe_queue.hpp"
#include <string>

class LogWriter
{
public:
    LogWriter(ThreadSafeQueue<std::string>& queue, std::string output_file_path);

    void operator()(std::stop_token st);

private:
    ThreadSafeQueue<std::string>& m_queue;
    std::string m_file_path;
};
