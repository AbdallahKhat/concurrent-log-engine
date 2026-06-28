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

LogAnalyzer::LogAnalyzer(ThreadSafeQueue<std::string>& src_queue, ThreadSafeQueue<std::string>& dest_queue)
    : m_src_queue{src_queue}, m_dest_queue{dest_queue}
{
}

void LogAnalyzer::operator()(std::stop_token st)
{

    std::string line;
    while (!st.stop_requested())
    {
        m_src_queue.pop(line);

        if (check_for_issue(line)) { m_dest_queue.push(line); }
    }

    // Graceful shutdown sequence when stop token is triggered
    std::cout << "[LogAnalyzer] Shutting down.\n";
}
