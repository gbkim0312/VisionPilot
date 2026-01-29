#pragma once

#include <stdint.h>    // for uint32_t, uint64_t, uint16_t
#include <string>      // for string
#include <sys/types.h> // for time_t

namespace vp
{
constexpr auto kDefaultTimeFormat = "%Y-%m-%dT%H:%M:%SZ";

#ifndef WIN32
std::string timeToString(time_t time, const std::string &format = kDefaultTimeFormat);
std::string time32ToString(uint32_t t32, const std::string &format = kDefaultTimeFormat);
std::string time64ToString(uint64_t t64, const std::string &format = kDefaultTimeFormat);
time_t stringToTime(const std::string &str_time, const std::string &format = kDefaultTimeFormat);
uint32_t stringToTime32(const std::string &str_time, const std::string &format = kDefaultTimeFormat);
uint32_t stringToTime64(const std::string &str_time, const std::string &format = kDefaultTimeFormat);
#endif

void setLeapSec(uint32_t _leapSec);

uint32_t timeToTime32(time_t time);
time_t time32Totime(uint32_t time);
time_t time64Totime(uint64_t time);

void setBaseTime(time_t e);
time_t getTimeFromBaseTime(time_t *t);

uint32_t getTime32();
uint64_t getTime64();
uint64_t timeToTime64(time_t time);
uint32_t time64ToTime32(uint64_t time64);
uint64_t time32ToTime64(uint32_t time32);

uint32_t iValueToTime32(uint16_t i_value);
uint16_t time32ToIvalue(uint32_t time32);

} // namespace vp