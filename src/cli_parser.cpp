#include "cli_parser.hpp"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <span>
#include <string_view>

namespace cli
{

namespace
{
struct FlagSpec
{
    std::string_view short_flag;
    std::string_view long_flag;
    std::string_view default_path;
    std::string_view description;
};

void print_usage(std::string_view program_name, const FlagSpec& spec)
{
    std::cout << "Usage: " << program_name << " [" << spec.short_flag << "|" << spec.long_flag
              << " <path>] [-h|--help]\n"
              << "  " << spec.short_flag << ", " << spec.long_flag << " <path>  "
              << spec.description << " (default: " << spec.default_path << ")\n"
              << "  -h, --help          Show this help message\n";
}

PathConfig parse_path_arg(int argc, char* argv[], const FlagSpec& spec)
{
    std::span<char*> args(argv, static_cast<std::size_t>(argc));
    const std::string_view program_name = args.empty() ? "daemon" : args[0];

    PathConfig result{.path = spec.default_path};

    for (std::size_t i = 1; i < args.size(); ++i)
    {
        const std::string_view arg = args[i];

        if (arg == "-h" || arg == "--help")
        {
            print_usage(program_name, spec);
            return {.should_exit = true, .exit_code = EXIT_SUCCESS};
        }
        if (arg == spec.short_flag || arg == spec.long_flag)
        {
            if (i + 1 >= args.size())
            {
                std::cerr << "Error: " << arg << " requires a file path.\n";
                print_usage(program_name, spec);
                return {.should_exit = true, .exit_code = EXIT_FAILURE};
            }
            result.path = args[++i];
        }
        else
        {
            std::cerr << "Error: unknown argument '" << arg << "'.\n";
            print_usage(program_name, spec);
            return {.should_exit = true, .exit_code = EXIT_FAILURE};
        }
    }

    return result;
}

} // namespace

PathConfig parse_processor_args(int argc, char* argv[])
{
    constexpr FlagSpec kProcessorSpec{
        .short_flag = "-i",
        .long_flag = "--input",
        .default_path = "input_file.txt",
        .description = "Log file to monitor",
    };
    return parse_path_arg(argc, argv, kProcessorSpec);
}

PathConfig parse_sink_args(int argc, char* argv[])
{
    constexpr FlagSpec kSinkSpec{
        .short_flag = "-o",
        .long_flag = "--output",
        .default_path = "output.log",
        .description = "Destination log file",
    };
    return parse_path_arg(argc, argv, kSinkSpec);
}

bool validate_input_file(const std::filesystem::path& path)
{
    std::error_code ec;

    if (!std::filesystem::exists(path, ec))
    {
        if (ec)
        {
            std::cerr << "Error: cannot access '" << path.string() << "': " << ec.message() << '\n';
        }
        else
        {
            std::cerr << "Error: input file does not exist: '" << path.string() << "'\n";
        }
        return false;
    }

    if (!std::filesystem::is_regular_file(path, ec))
    {
        std::cerr << "Error: input path is not a regular file: '" << path.string() << "'\n";
        return false;
    }

    errno = 0;
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error: cannot read input file '" << path.string()
                  << "': " << std::strerror(errno) << '\n';
        return false;
    }

    return true;
}

bool validate_output_file(const std::filesystem::path& path)
{
    std::error_code ec;

    if (std::filesystem::is_directory(path, ec))
    {
        std::cerr << "Error: output path is a directory: '" << path.string() << "'\n";
        return false;
    }

    if (path.has_parent_path() && !std::filesystem::exists(path.parent_path(), ec))
    {
        std::cerr << "Error: output directory does not exist: '" << path.parent_path().string()
                  << "'\n";
        return false;
    }

    errno = 0;
    std::ofstream file(path, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Error: cannot write to output file '" << path.string()
                  << "': " << std::strerror(errno) << '\n';
        return false;
    }

    return true;
}

} // namespace cli