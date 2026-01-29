#include "gaia_time.hpp"
#include <gtest/gtest.h>
#include <sys/time.h>

namespace
{
// UTC 2004-01-01
constexpr uint32_t k16092BaseTime = 1072915200;
} // namespace

namespace autocrypt
{

TEST(Time, time32)
{
    auto cur_time32 = getTime32();
    auto cur_time = time(nullptr);
    ASSERT_EQ(cur_time32 + k16092BaseTime, cur_time);
}

TEST(Time, time64)
{
    constexpr uint64_t us_unit = 1000000;
    constexpr uint64_t base_time_64 = static_cast<uint64_t>(k16092BaseTime) * us_unit;
    constexpr uint64_t tolerance = 1000; // 1ms 까지 허용
    using timeval_struct = struct timeval;

    timeval_struct te{};
    auto cur_time64 = getTime64() + base_time_64;
    // auto cur_time = time(nullptr) * us_unit;

    gettimeofday(&te, nullptr);

    uint64_t cur_us64 = cur_time64 % us_unit;
    uint64_t cur_us = te.tv_usec;
    uint64_t diff = (cur_us > cur_us64) ? cur_us - cur_us64 : cur_us64 - cur_us;

    // sec. 단위 비교
    ASSERT_LE(diff, tolerance);
}

TEST(Time, timeToString)
{
    auto t = static_cast<time_t>(k16092BaseTime);
    auto time_str = timeToString(t);
    ASSERT_EQ(time_str, "2004-01-01T00:00:00Z");
}

TEST(Time, stringToTime)
{
    auto time = stringToTime("20040101T000000Z");
    ASSERT_EQ(time, k16092BaseTime);
}

TEST(Time, stringToTime32)
{
    auto time32 = stringToTime32("20040101T000000Z");
    ASSERT_EQ(time32, 0);
}

} // namespace autocrypt