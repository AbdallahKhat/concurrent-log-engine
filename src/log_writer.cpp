#include "log_writer.hpp"
#include <fstream>
#include <iostream>

LogWriter::LogWriter(ThreadSafeQueue<std::string>& queue, std::string output_file_path)
    : m_queue{queue}, m_file_path{std::move(output_file_path)}
{
}

void LogWriter::operator()(std::stop_token st)
{
    std::ofstream output_file{m_file_path, std::ios::out | std::ios::app};

    if (!output_file.is_open())
    {
        std::cerr << "Error: Unable to create the output log file!\n";
        return;
    }

    std::string line;
    while (!st.stop_requested())
    {
        m_queue.pop(line);
        line += '\n';
        output_file << line;
        output_file.flush(); // Don't wait for large buffer before writing to disk
    }

    // Graceful shutdown sequence when stop token is triggered
    std::cout << "[LogWriter] Shutting down. Closing file.\n";
    output_file.close();
}
