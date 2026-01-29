#pragma once

#include "gaia_string_util.hpp"
#include <map>
#include <string>
#include <vector>

#ifndef DEFINE_ENUM_CLASS
#define DEFINE_ENUM_CLASS(enum_name, ...)                                    \
    enum class enum_name                                                     \
    {                                                                        \
        __VA_ARGS__                                                          \
    };                                                                       \
    inline const std::string &toString(enum_name value)                      \
    {                                                                        \
        using Pair = std::pair<enum_name, std::string>;                      \
        static const auto &entries = []() -> const std::vector<Pair> & {     \
            static std::vector<Pair> v;                                      \
            auto items = autocrypt::split(std::string{#__VA_ARGS__}, ",");   \
            std::map<std::string, int> name_to_val{};                        \
            v.reserve(items.size());                                         \
            int next_val = 0;                                                \
            for (const auto &item : items)                                   \
            {                                                                \
                std::string name = item;                                     \
                std::string value_str;                                       \
                auto pos = name.find('=');                                   \
                if (pos != std::string::npos)                                \
                {                                                            \
                    value_str = autocrypt::trim(name.substr(pos + 1));       \
                    name = name.substr(0, pos);                              \
                }                                                            \
                name = autocrypt::trim(name);                                \
                int enum_val = next_val;                                     \
                if (!value_str.empty())                                      \
                {                                                            \
                    auto it = name_to_val.find(value_str);                   \
                    if (it != name_to_val.end())                             \
                    {                                                        \
                        enum_val = it->second;                               \
                    }                                                        \
                    else                                                     \
                    {                                                        \
                        enum_val = std::stoi(value_str);                     \
                    }                                                        \
                }                                                            \
                name_to_val[name] = enum_val;                                \
                next_val = enum_val + 1;                                     \
                v.emplace_back(static_cast<enum_name>(enum_val), name);      \
            }                                                                \
            return v;                                                        \
        }();                                                                 \
        static const std::string not_found_str{"<Not Defined ENUM string>"}; \
        for (const auto &[e, s] : entries)                                   \
        {                                                                    \
            if (e == value)                                                  \
            {                                                                \
                return s;                                                    \
            }                                                                \
        }                                                                    \
        return not_found_str;                                                \
    }
#endif
