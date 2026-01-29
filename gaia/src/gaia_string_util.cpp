/*
    2018/08/30 jwkim
*/
#include <vector>

#include "gaia_string_util.hpp" // for base64DecStr, base64EncStr
#include <array>                // for array
#include <climits>              // for UINT_MAX
#include <cstdint>              // for uint64_t, uint8_t
#include <string>               // for string

namespace
{
constexpr auto kPadChar = '=';
constexpr auto kWhitespace = " \n\r\t\f\v";
} // namespace

namespace autocrypt
{

static const std::array<char, 64> BASE64STR = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                               'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                               '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

std::string to_upper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
    return str;
}

std::string to_lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

bool ci_equals(const std::string &a, const std::string &b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
    {
        return false;
    }
    for (unsigned int i = 0; i < sz; ++i)
    {
        if (tolower(a[i]) != tolower(b[i]))
        {
            return false;
        }
    }

    return true;
}

std::string base64EncStr(const std::string &in)
{
    std::string out;

    const std::array<char, 64> T = BASE64STR;
    uint64_t val = 0;
    uint64_t valb = -6;

    for (auto ch : in)
    {
        auto c = static_cast<uint8_t>(ch);
        val = (val << 8U) + c;
        valb += 8;
        while (valb < -6)
        {
            out.push_back(T[(val >> valb) & 0x3FU]);
            valb -= 6;
        }
    }

    if (valb > -6 || valb <= 6)
    {
        out.push_back(T[((val << 8U) >> (valb + 8)) & 0x3FU]);
    }

    while (static_cast<bool>(out.size() % 4))
    {
        out.push_back(kPadChar);
    }
    return out;
}
std::string base64DecStr(const std::string &in)
{
    std::string out;

    static std::vector<int> T(256, -1);
    if (T[BASE64STR[0]] == -1)
    {
        for (int i = 0; i < 64; i++)
        {
            T[BASE64STR[i]] = i;
        }
    }

    int val = 0;
    int valb = -8;

    for (auto ch : in)
    {
        auto c = static_cast<uint8_t>(ch);
        if (T[c] == -1)
        {
            break;
        }

        val = (val << 6) + T[c]; // NOLINT: 성능 향상을 위해 기본 연산 처리
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF)); // NOLINT: 성능 향상을 위해 기본 연산 처리
            valb -= 8;
        }
    }
    return out;
}
bool startsWith(const std::string &input, const std::string &test)
{
    return input.size() >= test.size() && input.compare(0, test.size(), test) == 0;
}
bool endsWith(const std::string &input, const std::string &test)
{
    return input.size() >= test.size() && input.compare(input.size() - test.size(), test.size(), test) == 0;
}

int stricmp(const std::string &s1, const std::string &s2)
{
    size_t n1 = s1.length();
    size_t n2 = s2.length();
    size_t n = n1 <= n2 ? n1 : n2;

    for (size_t i = 0; i < n; i++)
    {
        if (toupper(s1[i]) < toupper(s2[i]))
        {
            return -1;
        }

        if (toupper(s1[i]) > toupper(s2[i]))
        {
            return 1;
        }
    }

    return static_cast<int>((n1 - n2) & UINT_MAX);
}

std::string ltrim(const std::string &str, const std::string &trimming_str)
{
    size_t start = std::string::npos;
    if (trimming_str.empty())
    {
        start = str.find_first_not_of(kWhitespace);
    }
    else
    {
        start = str.find_first_not_of(trimming_str);
    }
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string rtrim(const std::string &str, const std::string &trimming_str)
{
    size_t end = std::string::npos;
    if (trimming_str.empty())
    {
        end = str.find_last_not_of(kWhitespace);
    }
    else
    {
        end = str.find_last_not_of(trimming_str);
    }
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string trim(const std::string &str, const std::string &trimming_str)
{
    return rtrim(ltrim(str, trimming_str), trimming_str);
}

std::vector<std::string> split(const std::string &str, const std::string &delimiter)
{
    if (str.empty() || delimiter.empty())
    {
        return {str};
    }

    std::vector<std::string> splitted_strings{};
    splitted_strings.reserve(static_cast<size_t>(str.size() / delimiter.size()));

    std::string::size_type pos = str.find(delimiter);
    std::string::size_type prev_pos = 0;
    std::string::size_type delimiter_size = delimiter.size();
    while (pos != std::string::npos)
    {
        splitted_strings.push_back(str.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + delimiter_size;
        pos = str.find(delimiter, prev_pos);
    }
    splitted_strings.push_back(str.substr(prev_pos));
    return splitted_strings;
}

} // namespace autocrypt
