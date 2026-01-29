#pragma once

#include <algorithm> // for find_if, lexicographical_compare
#include <string>
#include <vector>

namespace autocrypt
{

std::string to_upper(std::string str);
std::string to_lower(std::string str);

bool ci_equals(const std::string &a, const std::string &b);

struct ci_less
{
    // case-independent (ci) compare_less binary function
    struct nocase_compare
    {
        bool operator()(const unsigned char &c1, const unsigned char &c2) const
        {
            return tolower(c1) < tolower(c2);
        }
    };
    bool operator()(const std::string &s1, const std::string &s2) const
    {
        return std::lexicographical_compare(s1.begin(), s1.end(), // source range
                                            s2.begin(), s2.end(), // dest range
                                            nocase_compare());    // comparison
    }
};

std::string base64EncStr(const std::string &in);
std::string base64DecStr(const std::string &in);

bool startsWith(const std::string &input, const std::string &test);
bool endsWith(const std::string &input, const std::string &test);

int stricmp(const std::string &s1, const std::string &s2);

std::string ltrim(const std::string &str, const std::string &trimming_str = {});
std::string rtrim(const std::string &str, const std::string &trimming_str = {});
std::string trim(const std::string &str, const std::string &trimming_str = {});
std::vector<std::string> split(const std::string &str, const std::string &delimiter);

} // namespace autocrypt