#pragma once

#include <cstdlib>
#include <filesystem>

namespace cli
{

struct PathConfig
{
    std::filesystem::path path;
    bool should_exit{false};
    int exit_code{EXIT_SUCCESS};
};

PathConfig parse_processor_args(int argc, char* argv[]);
PathConfig parse_sink_args(int argc, char* argv[]);

bool validate_input_file(const std::filesystem::path& path);
bool validate_output_file(const std::filesystem::path& path);

} // namespace cli