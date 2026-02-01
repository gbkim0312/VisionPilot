#include "assembly.hpp"
#include "config_loader.hpp"
#include "gaia_log.hpp"
#include <iostream>

namespace
{
struct CliOptions
{
    std::string config_path{};
};

std::vector<std::string> toArgs(int argc, char *argv[]) // NOLINT(modernize-avoid-c-arrays,-warnings-as-errors)
{
    std::vector<std::string> args{};
    args.reserve(static_cast<std::size_t>(argc));

    for (int i = 0; i < argc; ++i)
    {
        args.emplace_back(argv[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    return args;
}

void printUsage(const std::string &program_name)
{
    std::cerr
        << "Usage: " << program_name << " [-c <config_path>] [LOG_LEVEL] [-h]\n"
        << "  -c <config_path>   Path to config file (default: etc/obu.conf)\n"
        << "  -h                 Show this help\n"
        << "  LOG_LEVEL            [trace|debug|info|warning|error|critical|off] Set the log level\n"
        << "example: " << program_name << " -c vision_pilot/res/etc/assembly_config.json info\n";
}

bool isOption(const std::string &s)
{
    if (s.empty())
    {
        return false;
    }
    return s[0] == '-';
}

bool parseArgs(const std::vector<std::string> &args,
               CliOptions &out,
               std::string &error)
{
    const std::size_t argc = args.size();
    if (argc <= 1)
    {
        error = "Empty argv";
        printUsage(args.at(0));
        return false;
    }

    for (std::size_t i = 1; i < argc; ++i)
    {
        const std::string &arg = args.at(i);

        if (arg == "-h" || arg == "--help")
        {
            printUsage(args.at(0));
            return false;
        }

        if (arg == "-c" || arg == "--config")
        {
            const std::size_t next = i + 1;
            if (next >= argc)
            {
                error = "Missing value for -c/--config";
                printUsage(args.at(0));
                return false;
            }

            const std::string &value = args.at(next);
            if (!value.empty() && value.at(0) == '-')
            {
                error = "Invalid value for -c/--config: " + value;
                printUsage(args.at(0));
                return false;
            }

            out.config_path = value;
            i = next;
            continue;
        }

        if (arg == "trace" || arg == "debug" || arg == "info" || arg == "warning" || arg == "error" || arg == "critical" || arg == "off")
        {
            // skip log level args
            continue;
        }
        error = "Unknown argument: " + arg;
        printUsage(args.at(0));
        return false;
    }

    return true;
}
} // namespace

std::atomic<bool> g_running{false};

void signalHandler([[maybe_unused]] int signum)
{
    g_running.store(false, std::memory_order_release);
}

int main(int argc, char **argv)
{
    CliOptions options{};
    std::string error{};

    auto args = toArgs(argc, argv);
    const bool ok = parseArgs(args, options, error);
    if (!ok)
    {
        if (!error.empty())
        {
            std::cerr << "Error: " << error << "\n";
            return 1;
        }

        return 0;
    }

    vp::logger::logInitFromMain(argc, argv);

    vp::config::ConfigLoader configLoader;
    auto loaded = configLoader.loadConfig(options.config_path.empty()
                                              ? "vision_pilot/res/etc/assembly_config.json"
                                              : options.config_path);

    if (!loaded || !configLoader.isLoaded())
    {
        std::cerr << "Failed to load configuration.\n";
        return 1;
    }

    vp::assembly::Assembly assembly(configLoader.getAssemblyConfig());
    assembly.startService();

    return 0;
}