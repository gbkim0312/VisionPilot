#include "gaia_json_util.hpp"
#include "nlohmann/json.hpp" // for basic_json<>::object_t
#include <iostream>          // for cerr

namespace autocrypt
{

std::string jsonFileToStr(const std::string &file_name)
{
    if (!isFileExist(file_name))
    {
        LOG_ERR("File not found: {}", file_name);
        return "";
    }

    try
    {
        std::ifstream file(file_name);

        json j = json::parse(file, nullptr, true, true); // 코멘트 무시를 위한 코드
        return j.dump(4);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Print data from file failed: " << e.what() << '\n';
        std::cerr << "Missing conversion functions?" << '\n';
        return "";
    }
}

std::string jsonToStr(const json &json)
{
    return json.dump(4);
}

json stringToJson(const std::string &jsonStr)
{
    return json::parse(jsonStr, nullptr, true, true); // 코멘트 무시를 위한 코드
}

json fileToJson(const std::string &file_name)
{
    if (!isFileExist(file_name))
    {
        LOG_ERR("File not found: {}", file_name);
        return json();
    }

    try
    {
        std::ifstream file(file_name);

        return json::parse(file, nullptr, true, true); // 코멘트 무시를 위한 코드
    }
    catch (const std::exception &e)
    {
        std::cerr << "Parse data from file failed: " << e.what() << '\n';
        std::cerr << "Missing conversion functions?" << '\n';
        return json();
    }
}

} // namespace autocrypt