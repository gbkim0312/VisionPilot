#include "gaia_string_util.hpp"
#include <gtest/gtest.h>

namespace autocrypt
{

TEST(ToUpper, CompareWithLower)
{
    ASSERT_EQ(to_upper("autocrypt"), "AUTOCRYPT");
}

TEST(ToLower, CompareWithUpper)
{
    ASSERT_EQ(to_lower("AUTOCRYPT"), "autocrypt");
}

TEST(CiEquals, Positive)
{
    ASSERT_TRUE(ci_equals("AUTOCRYPT", "autocrypt"));
}

TEST(CiEquals, Negative)
{
    ASSERT_FALSE(ci_equals("AUTOCRYPT", "AUTOBAHNN"));
}

TEST(Base64, codec)
{
    std::string org = "ABCDEF";
    std::string base64 = base64EncStr(org);

    ASSERT_EQ(base64, "QUJDREVG");

    std::string out = base64DecStr(base64);
    ASSERT_EQ(out, org);
}

TEST(StartsWith, Positive)
{
    std::string input = "https://curl.haxx.se/libcurl/c/CURLOPT_CAINFO.html";
    ASSERT_TRUE(startsWith(input, "https://"));
}

TEST(StartsWith, Negative)
{
    std::string input = "https://curl.haxx.se/libcurl/c/CURLOPT_CAINFO.html";
    ASSERT_FALSE(startsWith(input, "http://"));
}

TEST(EndsWith, Positive)
{
    std::string input = "https://curl.haxx.se/libcurl/c/CURLOPT_CAINFO.html";
    ASSERT_TRUE(endsWith(input, ".html"));
}

TEST(EndsWith, Negative)
{
    std::string input = "https://curl.haxx.se/libcurl/c/CURLOPT_CAINFO.html";
    ASSERT_FALSE(endsWith(input, ".xml"));
}

TEST(Stricmp, EqualUpperAndLower)
{
    std::string s1 = "abc00";
    std::string s2 = "ABC00";

    ASSERT_EQ(stricmp(s1, s2), 0);
}

TEST(Stricmp, UnequalFirstCharacter)
{
    std::string s1 = "crystal";
    std::string s2 = "weed";

    ASSERT_LE(stricmp(s1, s2), 0);
}

TEST(Stricmp, UnequalsucceedingCharacter)
{
    std::string s1 = "Autocrypt";
    std::string s2 = "Autobahnn";

    ASSERT_GT(stricmp(s1, s2), 0);
}

} // namespace autocrypt