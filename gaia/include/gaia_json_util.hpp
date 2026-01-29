#pragma once

#include "gaia_dir.hpp" // for isFileExist
#include "gaia_log.hpp" // for LOG_WRN, LOG_ERR
#include "spdlog/spdlog.h"
#include <fstream>           // for ifstream
#include <initializer_list>  // for initializer_list
#include <nlohmann/json.hpp> // for NLOHMANN_JSON_EXPAND, NLOHMANN_JSON_PASTE
#include <string>            // for string
#include <typeinfo>          // for type_info
#include <vector>            // for vector

using json = nlohmann::json;
namespace autocrypt
{

template <class T>
T jsonFileToStruct(const std::string &file_name)
{
    if (!isFileExist(file_name))
    {
        LOG_ERR("File not found: {}", file_name);
        return T{};
    }

    try
    {
        std::ifstream file(file_name);

        json j = json::parse(file, nullptr, true, true); // 코멘트 무시를 위한 코드
        return j.get<T>();
    }
    catch (const std::exception &e)
    {
        LOG_WRN("Loading data from file failed (missing conversion function for {}?): {}", typeid(T).name(), e.what());
        return T{};
    }
}

template <class T>
std::string structToJsonStr(const T &t)
{
    try
    {
        json j = t;
        return j.dump(4);
    }
    catch (const std::exception &e)
    {
        LOG_WRN("Converting data to json string failed (missing conversion function for {}?): {}", typeid(T).name(), e.what());
        return "";
    }
}

template <class T>
T jsonToStruct(const json &j)
{
    try
    {
        if (j.is_null())
        {
            LOG_WRN("Input json is null. Using default value for {}", typeid(T).name());
            return T{};
        }
        return j.get<T>();
    }
    catch (const std::exception &e)
    {
        LOG_WRN("Loading data from json failed (missing conversion function for {}?): {}", typeid(T).name(), e.what());
        return T{};
    }
}

std::string jsonFileToStr(const std::string &file_name);

std::string jsonToStr(const json &json);

json stringToJson(const std::string &jsonStr);

json fileToJson(const std::string &file_name);

} // namespace autocrypt