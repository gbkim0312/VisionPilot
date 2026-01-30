#include "gaia_macro.hpp"
#include "gtest/gtest.h"

namespace
{
DEFINE_ENUM_CLASS(
    TestEnum,
    TEST_1,
    TEST_2 = 10,
    TEST_3);
}

namespace vp
{

TEST(MacroTest, DefineEnumClass)
{
    constexpr auto test_enum_1_str = "TEST_1";
    constexpr auto test_enum_2_str = "TEST_2";
    constexpr auto test_enum_3_str = "TEST_3";

    auto test_enum_1 = TestEnum::TEST_1;
    auto test_enum_2 = static_cast<TestEnum>(static_cast<uint32_t>(test_enum_1) + 10);
    auto test_enum_3 = static_cast<TestEnum>(static_cast<uint32_t>(test_enum_2) + 1);

    ASSERT_EQ(test_enum_1, TestEnum::TEST_1);
    ASSERT_EQ(test_enum_2, TestEnum::TEST_2);
    ASSERT_EQ(test_enum_3, TestEnum::TEST_3);
    ASSERT_EQ(toString(test_enum_1), test_enum_1_str);
    ASSERT_EQ(toString(test_enum_2), test_enum_2_str);
    ASSERT_EQ(toString(test_enum_3), test_enum_3_str);
}

} // namespace vp