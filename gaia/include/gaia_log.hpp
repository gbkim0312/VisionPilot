#pragma once

#include "spdlog/spdlog.h"
#include <string> // for string
#include <vector> // for vector

namespace autocrypt
{

struct LogConfig
{
    std::string type = "stdout";
    std::string filename = "logs/vss_security";
    int max_size = 1000;
    int max_files = 10;
    int hour = 10;
    int minute = 10;
#if DEBUG
    // DEBUG 기본값:
    // - 로그에 파일경로 표시
    // - 로그레벨 Debug
    std::string pattern = "%^[%H:%M:%S.%e | tid:%t | %@ %! %l] %v%$";
    std::string level = "debug";
#else
    // DEBUG 외 기본값:
    // - 로그에 파일경로 미표시
    // - 로그레벨 Info
    std::string pattern = "%^[%H:%M:%S.%e | tid:%t | %! %l] %v%$";
    std::string level = "info";
#endif
};

namespace logger
{
namespace _global
{
extern int spdlog_level;
extern spdlog::logger *logger;
} // namespace _global
// main 종료 전, 즉 deinitialize 호출 시 등에 logDeinit을 불러 spdlog를 정리해야 함
// https://github.com/gabime/spdlog/issues/1738
// https://github.com/gabime/spdlog/issues/1050
void logInit(const std::vector<LogConfig> &cnfs, const std::function<spdlog::sink_ptr(const std::string &)> &sink_generator_cb = nullptr);

void logDeinit();

// init log from argc and argv
void logInitFromMain(int argc, char *argv[], const std::function<spdlog::sink_ptr(const std::string &)> &sink_generator_cb = nullptr);
} // namespace logger
} // namespace autocrypt

#if !defined(LOG_ACTIVE_LEVEL)
#define LOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#define PRINTLN(...)          \
    std::cout << "\033[1;4m"; \
    fmt::print(__VA_ARGS__);  \
    std::cout << "\033[0m\n";

#if LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#define LOG_TRA(...)                                                                                                                           \
    do                                                                                                                                         \
    {                                                                                                                                          \
        using autocrypt::logger::_global::spdlog_level;                                                                                        \
        using autocrypt::logger::_global::logger;                                                                                              \
        if (spdlog_level <= SPDLOG_LEVEL_TRACE)                                                                                                \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::trace, __VA_ARGS__); \
    } while (0)
#define LOG_TRA_EX(file, line, function, ...) (autocrypt::logger::_global::logger)->log(spdlog::source_loc{file, line, static_cast<const char *>(function)}, spdlog::level::trace, __VA_ARGS__)
#else
#define LOG_TRA(...) (void)0
#define LOG_TRA_EX(...) (void)0
#endif

#if LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#define LOG_DBG(...)                                                                                                                           \
    do                                                                                                                                         \
    {                                                                                                                                          \
        using autocrypt::logger::_global::spdlog_level;                                                                                        \
        using autocrypt::logger::_global::logger;                                                                                              \
        if (spdlog_level <= SPDLOG_LEVEL_DEBUG)                                                                                                \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::debug, __VA_ARGS__); \
    } while (0)
#define LOG_DBG_EX(file, line, function, ...) (autocrypt::logger::_global::logger)->log(spdlog::source_loc{file, line, static_cast<const char *>(function)}, spdlog::level::debug, __VA_ARGS__)
#else
#define LOG_DBG(...) (void)0
#define LOG_DBG_EX(...) (void)0
#endif

#if LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#define LOG_INF(...)                                                                                                                          \
    do                                                                                                                                        \
    {                                                                                                                                         \
        using autocrypt::logger::_global::spdlog_level;                                                                                       \
        using autocrypt::logger::_global::logger;                                                                                             \
        if (spdlog_level <= SPDLOG_LEVEL_INFO)                                                                                                \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::info, __VA_ARGS__); \
    } while (0)
#define LOG_INF_EX(file, line, function, ...) (autocrypt::logger::_global::logger)->log(spdlog::source_loc{file, line, static_cast<const char *>(function)}, spdlog::level::info, __VA_ARGS__)
#else
#define LOG_INF(...) (void)0
#define LOG_INF_EX(...) (void)0
#endif

#if LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#define LOG_WRN(...)                                                                                                                          \
    do                                                                                                                                        \
    {                                                                                                                                         \
        using autocrypt::logger::_global::spdlog_level;                                                                                       \
        using autocrypt::logger::_global::logger;                                                                                             \
        if (spdlog_level <= SPDLOG_LEVEL_WARN)                                                                                                \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::warn, __VA_ARGS__); \
    } while (0)
#define LOG_WRN_EX(file, line, function, ...) (autocrypt::logger::_global::logger)->log(spdlog::source_loc{file, line, static_cast<const char *>(function)}, spdlog::level::warn, __VA_ARGS__)
#else
#define LOG_WRN(...) (void)0
#define LOG_WRN_EX(...) (void)0
#endif

#if LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#define LOG_ERR(...)                                                                                                                         \
    do                                                                                                                                       \
    {                                                                                                                                        \
        using autocrypt::logger::_global::spdlog_level;                                                                                      \
        using autocrypt::logger::_global::logger;                                                                                            \
        if (spdlog_level <= SPDLOG_LEVEL_ERROR)                                                                                              \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::err, __VA_ARGS__); \
    } while (0)
#define LOG_ERR_EX(file, line, function, ...) (autocrypt::logger::_global::logger)->log(spdlog::source_loc{file, line, static_cast<const char *>(function)}, spdlog::level::err, __VA_ARGS__)
#else
#define LOG_ERR(...) (void)0
#define LOG_ERR_EX(...) (void)0
#endif

#if LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define LOG_CRI(...)                                                                                                                              \
    do                                                                                                                                            \
    {                                                                                                                                             \
        using autocrypt::logger::_global::spdlog_level;                                                                                           \
        using autocrypt::logger::_global::logger;                                                                                                 \
        if (spdlog_level <= SPDLOG_LEVEL_CRITICAL)                                                                                                \
            (logger)->log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::critical, __VA_ARGS__); \
    } while (0)
#define LOG_CRI_EX(file, line, function, ...) (autocrypt::logger::_global::logger)->log(spdlog::source_loc{file, line, static_cast<const char *>(function)}, spdlog::level::critical, __VA_ARGS__)
#else
#define LOG_CRI(...) (void)0
#define LOG_CRI_EX(...) (void)0
#endif