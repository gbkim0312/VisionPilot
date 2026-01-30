#include "gaia_time.hpp"
#include "gaia_exception.hpp" // for SysException, THROWLOG
#include <array>              // for array
#include <ctime>              // for gmtime_r, strftime, time, strptime
#include <string>             // for string
#include <sys/time.h>         // for gettimeofday, timeval

namespace
{

// 1609.2 epoch time 2004/01/01 00:00:00(UTC) : time_t(1072915200)
// iValue 0 :
// 2015/01/06 04:00:00(KST) (UTC+09)
// 2015/01/05 19:00:00(UTC)
// 1420484400(time_t)
//  347569200(1609.2)
// 우리나라도 화요일 새벽4시가 교통사고 발생이 가장 적다는 가정.
// but 확인결과
// 우리나라는 목요일 새벽3시경이 교통사고 사망자가 가장 적음
// http://taas.koroad.or.kr/ 2015년 통계 참조
// 2015/01/08 03:00:00(KST) (UTC+09)
// 2015/01/07 18:00:00(UTC)
// 1420653600(time_t)
//  347738400(1609.2)
// 수정 2017/7/28
// SCMS와의 호환성(?)을 위해서 2015/1/6(Tue) 04:00(EST)로 변경
// EST는 UTC-05, KST는 UTC+09 : 14시간 차이
// 기준 시간인 04:00(EST)는 18:00(KST)가 된다.
// 즉 우리나라의 인증서 교체 기준 시간은 화요일 퇴근시간이 된다.
// 2015/01/06 04:00:00(EST) (UTC-05) SCMS i=0 clock time
// 2015/01/06 09:00:00(UTC)
// 2015/01/06 18:00:00(KST)
// 1420534800(time_t)
// 수정 2017/9/27
// 사내 서버와의 test를 위하여
// 2017/1/1(UTC)로 변경
// 1483228800(time_t)
//  410313600(1609.2)
// plugfest test를 위하여
// https://wiki.campllc.org/display/SCP/Special+Cryptographic+Primitives+in+SCMS
// 347,616,003 seconds since 1609.2 epoch
// 347616003(1609.2)
// 4:00 am Eastern Time on Tuesday, January 6, 2015
// i.e., 4023 days and 8 hours or 347,616,003 seconds since 1609.2 epoch
// 4023일 8시간이 아니라 9시간이다. 담당자가 서머타임을 착각한듯.
// 그럼 1609.2 epoch부터 347,616,003초가 아니라 347,619,603초가 맞다.
// 347619603(1609.2)

// timestamp(UTC)
constexpr uint32_t ts20040101 = 1072915200;

uint32_t leapSec = 0;
int epoch_time_base_gap = 0;

} // namespace

namespace vp
{

#ifndef WIN32
std::string timeToString(time_t t, const std::string &format)
{
    using tm_struct = struct tm;

    tm_struct tm_time{};
    gmtime_r(&t, &tm_time);

    std::array<char, 32> time_arr{};
    auto str_size = strftime(time_arr.data(), 32, format.c_str(), &tm_time);
    return {time_arr.data(), str_size};
}

std::string time32ToString(uint32_t t32, const std::string &format)
{
    return timeToString(time32Totime(t32), format);
}

std::string time64ToString(uint64_t t64, const std::string &format)
{
    return timeToString(time64Totime(t64), format);
}

time_t stringToTime(const std::string &str_time, const std::string &format)
{
    using tm_struct = struct tm;

    tm_struct tm_time{};
    if (strptime(str_time.c_str(), format.c_str(), &tm_time) == nullptr)
    {
        return 0;
    }
    return timegm(&tm_time);
}

uint32_t stringToTime32(const std::string &str_time, const std::string &format)
{
    return timeToTime32(stringToTime(str_time, format));
}

uint32_t stringToTime64(const std::string &str_time, const std::string &format)
{
    return timeToTime64(stringToTime(str_time, format));
}
#endif

void setLeapSec(uint32_t _leapSec)
{
    leapSec = _leapSec;
}

uint32_t timeToTime32(time_t time)
{
    auto t = static_cast<uint64_t>(time < 0 ? 0 : time);
    t = t < ts20040101 ? ts20040101 : t;
    t = t - ts20040101 + leapSec;
    return t > std::numeric_limits<uint32_t>::max()
               ? std::numeric_limits<uint32_t>::max()
               : static_cast<uint32_t>(t);
}

time_t time32Totime(uint32_t time)
{
    uint32_t t = static_cast<time_t>(time) + ts20040101;

    return static_cast<time_t>(t - leapSec);
}

time_t time64Totime(uint64_t time)
{
    auto time32 = time64ToTime32(time);
    return time32Totime(time32);
}

// 테스트 전용 시간 변경 API
void setBaseTime(time_t e)
{
    epoch_time_base_gap = static_cast<int>(time(nullptr) - e);
}

// GetTime32, GetTime64에 사용됨
time_t getTimeFromBaseTime(time_t *t)
{
    return time(t) - epoch_time_base_gap;
}

// This type gives the number of (TAI) seconds since 00:00:00 UTC, 1 January, 2004.
uint32_t getTime32()
{
    return timeToTime32(getTimeFromBaseTime(nullptr));
}

// This data structure is a 64-bit integer giving an estimate of the number of (TAI) microseconds since
// 00:00:00 UTC, 1 January, 2004.
uint64_t getTime64()
{
    using timeval_struct = struct timeval;

    timeval_struct te{};
    gettimeofday(&te, nullptr);

    auto gapped_sec = te.tv_sec - epoch_time_base_gap;
    auto sec = static_cast<uint32_t>(gapped_sec < 0 ? 0 : gapped_sec);
    sec = sec < ts20040101 ? ts20040101 : sec;
    sec = sec - ts20040101 + leapSec;
    return (1000000 * (static_cast<uint64_t>(sec))) + te.tv_usec;
}

uint64_t timeToTime64(time_t time)
{
    return time32ToTime64(timeToTime32(time));
}

uint32_t time64ToTime32(uint64_t time64)
{
    return static_cast<uint32_t>(time64 / 1000000);
}

uint64_t time32ToTime64(uint32_t time32)
{
    return (static_cast<uint64_t>(time32)) * 1000000;
}

} // namespace vp