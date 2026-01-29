#include "gaia_time.hpp"
#include <gtest/gtest.h>

namespace
{
constexpr auto kIValue241Time = 651396841;
constexpr auto ivalue = 241;
} // namespace

namespace autocrypt
{
TEST(Time, IValue)
{
    std::cout << "Time32: " << kIValue241Time << " | IValue: " << ivalue << std::endl;
    ASSERT_EQ(ivalue, time32ToIvalue(kIValue241Time));
}

} // namespace autocrypt