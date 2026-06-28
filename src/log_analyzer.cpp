#include "log_analyzer.hpp"
#include <iostream>

namespace
{
constexpr std::array issues = {"[WARNING]", "[ERROR]", "[CRITICAL]"};

bool check_for_issue(const std::string& line)
{
    for (const auto& issue : issues)
    {
        if (line.find(issue) != std::string::npos) return true;
    }

    return false;
}

} // Anonymous namespace

LogAnalyzer::LogAnalyzer(ThreadSafeQueue<std::string>& src_queue,
                         ThreadSafeQueue<std::string>& dest_queue)
    : m_src_queue{src_queue}, m_dest_queue{dest_queue}
{
}

void LogAnalyzer::operator()(std::stop_token st)
{

    std::string line;
    while (true)
    {
        if (!m_src_queue.pop(line, st)) { break; }

        if (check_for_issue(line))
        {
            if (!m_dest_queue.push(line, st)) { break; }
        }
    }

    // Graceful shutdown sequence when stop token is triggered
    std::cerr << "[LogAnalyzer] Stop token received. shutting down.\n";;
}
