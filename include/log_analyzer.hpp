#pragma once

#include "thread_safe_queue.hpp"
#include <string>

class LogAnalyzer
{
public:
    LogAnalyzer(ThreadSafeQueue<std::string>& src_queue, ThreadSafeQueue<std::string>& dest_queue);

    void operator()(std::stop_token st);

private:
    ThreadSafeQueue<std::string>& m_src_queue;
    ThreadSafeQueue<std::string>& m_dest_queue;
};
