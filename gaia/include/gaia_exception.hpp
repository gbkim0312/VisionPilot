#pragma once

#include "gaia_log.hpp"
#include <cstdarg>
#include <exception>
#include <iostream>
#include <sstream>

#define THROW(EXCEPTION, ...) \
    throw EXCEPTION(static_cast<const char *>(__FUNCTION__), __LINE__, false, fmt::format(__VA_ARGS__))

#define THROWLOG(EXCEPTION, ...) \
    throw EXCEPTION(static_cast<const char *>(__FUNCTION__), __LINE__, true, fmt::format(__VA_ARGS__))

#define THROWVAL(EXCEPTION, EVALUE, ...) \
    throw EXCEPTION(static_cast<const char *>(__FUNCTION__), __LINE__, false, EVALUE, fmt::format(__VA_ARGS__))

namespace autocrypt
{

class ExceptionBase : public std::exception
{
public:
    const char *func;
    int line;
    std::runtime_error msg{std::string{}};
    uint32_t eVal;

    ExceptionBase(const char *func, int line, bool bLog, const std::ostream &_msg)
    {
        std::stringstream ss;
        ss << _msg.rdbuf();
        msg = std::runtime_error{ss.str()};

        this->func = func;
        this->line = line;
        this->eVal = 0;

        if (bLog)
        {
            log();
        }
    }

    ExceptionBase(const char *func, int line, bool bLog, uint32_t eVal, std::string _msg) : msg{_msg}
    {
        this->func = func;
        this->line = line;
        this->eVal = eVal;

        if (bLog)
        {
            log();
        }
    }

    ExceptionBase(const char *func, int line, bool bLog, std::string _msg) : msg{_msg}
    {
        this->func = func;
        this->line = line;

        if (bLog)
        {
            log();
        }
    }

    ExceptionBase(const char *func, int line, bool bLog)
    {
        this->func = func;
        this->line = line;
        if (bLog)
        {
            log();
        }
    }

    virtual ~ExceptionBase() throw()
    {
    }

    virtual const char *what() const throw()
    {
        return msg.what();
    }

    void log()
    {
        LOG_ERR_EX("", line, func, msg.what());
    }
};

class SysException : public ExceptionBase
{
public:
    SysException(const char *func, int line, bool bLog, const char *fmt, ...)
        : ExceptionBase(func, line, false)
    {
        va_list ap;
        va_start(ap, fmt);
        format(fmt, ap);
        va_end(ap);

        if (bLog)
        {
            log();
        }
    }
    SysException(const char *func, int line, bool bLog, std::string _msg)
        : ExceptionBase(func, line, bLog, _msg)
    {
    }

    virtual ~SysException()
    {
    }

private:
    void format(const char *fmt, va_list ap)
    {
        char *ptr;
        int n;
        n = vasprintf(&ptr, fmt, ap);
        if (n < 0)
        {
            return;
        }
        msg = std::runtime_error{ptr};
        free(ptr);
    }

#if defined(_WIN32) || defined(WIN32)

    int vasprintf(char **strp, const char *fmt, va_list ap)
    {
        // _vscprintf tells you how big the buffer needs to be
        int len = _vscprintf(fmt, ap);
        if (len == -1)
        {
            return -1;
        }

        size_t size = (size_t)len + 1;
        char *str = (char *)malloc(size);
        if (!str)
        {
            return -1;
        }

        // _vsprintf_s is the "secure" version of vsprintf
        int r = _vsnprintf_s(str, size, len + 1, fmt, ap);
        if (r == -1)
        {
            free(str);
            return -1;
        }

        *strp = str;
        return r;
    }

#endif
};

} // namespace autocrypt
