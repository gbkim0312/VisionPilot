#pragma once

#include "gaia_octstr.hpp"
#include <map>
#include <set>
#include <stddef.h>
#include <string>
#include <vector>

namespace vp
{

class ZipManager
{
public:
    bool loadFromMem(const OctStr &content);
    bool loadFromFile(const std::string &zip_filepath);
    bool saveToFile(const std::string &zip_filepath);
    OctStr saveToMem();

    uint32_t getLatestErrorCode();
    const std::string &getLatestErrorMessage();

    std::set<std::string> getDirectories();
    std::vector<std::string> getAllFileNames();
    OctStr getFile(const std::string &filename);

    void addFile(const std::string &filename, const OctStr &filedata);
    void removeFile(const std::string &filename);
    void clear();

    size_t size() const;

private:
    using zip_t = void;
    void parseLatestZipError(zip_t *zip_ptr);
    bool readAllFiles(zip_t *zip_ptr);
    bool writeAllFiles(zip_t *zip_ptr);

    std::map<std::string, OctStr> archive_{};
    uint32_t latest_error_code_ = 0;
    std::string latest_error_str_{};
};

} // namespace vp