#include "gaia_time.hpp"
#include "gtest/gtest.h"

namespace
{
constexpr auto kIValue502Time = 651396295;
constexpr auto ivalue = 502;
} // namespace

namespace autocrypt
{
TEST(Time, IValue)
{
    std::cout << "Time32: " << kIValue502Time << " | IValue: " << ivalue << std::endl;
    ASSERT_EQ(ivalue, time32ToIvalue(kIValue502Time));
}

} // namespace autocrypt