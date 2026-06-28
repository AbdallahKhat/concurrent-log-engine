#include "log_reader.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

LogReader::LogReader(ThreadSafeQueue<std::string>& queue, std::string input_file_path)
    : m_queue(queue), m_file_path(std::move(input_file_path))
{
}

void LogReader::operator()(std::stop_token st)
{
    std::ifstream log_file{m_file_path};

    if (!log_file.is_open())
    {
        std::cerr << "Error: Unable to open the log file!\n";
        return;
    }

    std::string line;
    std::string incomplete_line;

    while (!st.stop_requested())
    {
        if (std::getline(log_file, line))
        {
            // If getline reached EOF but managed to extract a fragment (eofbit: true)
            if (log_file.eof())
            {
                // keep the fragment until the newline arrives.
                if (!line.empty()) incomplete_line += line;
            }
            else
            {
                // Successfully extracted a full line terminated by \n.
                if (!incomplete_line.empty())
                {
                    line = incomplete_line + line;
                    incomplete_line.clear();
                }

                if (!m_queue.push(line, st)) { break; }
                continue;
            }
        }
        else if (log_file.bad())
        {
            // A catastrophic stream error occurred (ex. file deleted from disk)
            std::cerr << "[LogReader] Stream fatal error. Shutting down reader.\n";
            return;
        }

        // Reached End of file, no extracted chars (eofbit & failbit: true)
        // Sleep for 1 second untill new logs are appended to the log file
        log_file.clear();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Graceful shutdown sequence when stop token is triggered
    std::cerr << "[LogReader] Stop token received. Closing input file stream.\n";
    log_file.close();
}
