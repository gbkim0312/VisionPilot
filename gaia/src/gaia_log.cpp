int log_level;

#include "gaia_log.hpp"
#include "gaia_string_util.hpp"              // for to_upper
#include "spdlog/logger.h"                   // for logger
#include "spdlog/sinks/basic_file_sink.h"    // for basic_file_sink_mt
#include "spdlog/sinks/daily_file_sink.h"    // for daily_file_sink_mt
#include "spdlog/sinks/rotating_file_sink.h" // for rotating_file_sink_mt
#include "spdlog/sinks/sink.h"               // for sink
#include "spdlog/sinks/stdout_color_sinks.h" // for stdout_color_sink_mt
#include "spdlog/sinks/syslog_sink.h"        // for syslog_sink_mt
#include "spdlog/spdlog.h"                   // for set_default_logger, shu...
#include <algorithm>                         // for find, max
#include <memory>                            // for make_shared, __shared_p...
#include <stdexcept>                         // for runtime_error

#if defined(__ANDROID__)
#include "spdlog/sinks/android_sink.h"
#endif

namespace
{
std::string loggerName = "gaialog";
} // namespace

namespace autocrypt::logger
{
namespace _global
{
int spdlog_level = spdlog::level::info;
spdlog::logger *logger = spdlog::default_logger_raw();
} // namespace _global

spdlog::level::level_enum stringToLevel(const std::string &level_str)
{
    spdlog::level::level_enum level = spdlog::level::off;
    auto level_uppercase_str = to_upper(level_str);
    if (level_uppercase_str == "TRACE")
    {
        level = spdlog::level::trace;
    }
    else if (level_uppercase_str == "DEBUG")
    {
        level = spdlog::level::debug;
    }
    else if (level_uppercase_str == "INFO")
    {
        level = spdlog::level::info;
    }
    else if (level_uppercase_str == "WARNING")
    {
        level = spdlog::level::warn;
    }
    else if (level_uppercase_str == "ERROR")
    {
        level = spdlog::level::err;
    }
    else if (level_uppercase_str == "CRITICAL")
    {
        level = spdlog::level::critical;
    }
    else
    {
        throw std::runtime_error("Unknown log level: " + level_str);
    }
    return level;
}

void logInit(const std::vector<LogConfig> &cnfs, const std::function<spdlog::sink_ptr(const std::string &)> &sink_generator_cb)
{
    std::vector<spdlog::sink_ptr> sinks;
    for (const auto &cnf : cnfs)
    {
        spdlog::sink_ptr sink;
        std::string pattern = cnf.pattern;
        auto log_type = to_upper(cnf.type);

        if (log_type == "STDOUT")
        {
            sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        }
        else if (log_type == "SYSLOG")
        {
            sink = std::make_shared<spdlog::sinks::syslog_sink_mt>("autocryptv2x", 0, 0, false);
        }
        else if (log_type == "FILE")
        {
            sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(cnf.filename + ".log");
        }
        else if (log_type == "ROTATING_FILE")
        {
            sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(cnf.filename + ".rot.log", cnf.max_size * 1024, cnf.max_files, false);
        }
        else if (log_type == "DAILY_FILE")
        {
            sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(cnf.filename + ".daily.log", cnf.hour, cnf.minute, false, cnf.max_files);
        }
        else if (sink_generator_cb && log_type.find("CUSTOM_") != std::string::npos)
        {
            sink = sink_generator_cb(log_type);
            if (sink == nullptr)
            {
                throw std::runtime_error("Custom logger not implemented.");
            }
        }
#if defined(__ANDROID__)
        else if (log_type == "ANDROID")
        {
            sink = std::make_shared<spdlog::sinks::android_sink_mt>("autocryptv2x");
        }
#endif
        else
        {
            throw std::runtime_error("Unknown log type: " + cnf.type);
        }

        sink->set_pattern(pattern);

        auto level = stringToLevel(cnf.level);
        sink->set_level(level);
        if (_global::spdlog_level > level)
        {
            _global::spdlog_level = level;
        }

        sinks.push_back(sink);
    }

    auto log = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
    log->set_level(spdlog::level::trace);
    log->flush_on(spdlog::level::warn);
    spdlog::drop(loggerName);
    spdlog::register_logger(log);
    _global::logger = spdlog::get(loggerName).get();
}

void logDeinit()
{
    _global::logger = spdlog::default_logger_raw();
    _global::spdlog_level = spdlog::level::off; // reset to off on each init
    spdlog::drop(loggerName);
}

void logInitFromMain(int argc, char *argv[], const std::function<spdlog::sink_ptr(const std::string &)> &sink_generator_cb) // NOLINT: main argv 처리는 C++20 span 없이는 어려움
{
    LogConfig conf;
    conf.level = "WARNING";
    if (argc > 1)
    {
        std::vector<std::string> options(&argv[1], &argv[argc]); // NOLINT: main argv 처리는 C++20 span 없이는 어려움
        std::vector<std::string> loglevels{"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};

        for (const auto &option : options)
        {
            if (std::find(loglevels.begin(), loglevels.end(), to_upper(option)) != loglevels.end())
            {
                conf.level = option;
            }
        }
    }
    logInit(std::vector<LogConfig>{conf}, sink_generator_cb);
}

} // namespace autocrypt::logger