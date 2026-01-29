#pragma once

#include "gaia_octstr.hpp" // for OctStr
#include <cstdint>         // for uint16_t
#include <set>             // for set, operator!=, set<>::iterator
#include <string>          // for string
#include <vector>          // for vector

namespace autocrypt
{

class DirMan
{
public:
    DirMan(const std::string &dir);
    DirMan(const std::string &dir, const std::string &fileExt);
    ~DirMan() = default;

    std::vector<std::string> files;
    std::set<uint16_t> values;

    std::vector<std::string> dirs;

    void init()
    {
        files.clear();
        values.clear();
        dirs.clear();
    }

    // For listing iVal files ex>> 010A.zip

    // fromI <= iVal < toI
    int iValFiles(uint16_t fromI, uint16_t toI);
    int iValFile(uint16_t i) { return iValFiles(i, i + 1); }
    int allIValFiles();
    uint16_t minI() const;
    uint16_t maxI() const;

    bool isIValIn(uint16_t iVal)
    {
        return values.find(iVal) != values.end();
    }

    // fromI <= iVal < toI
    bool isIValsIn(uint16_t fromI, uint16_t toI);
    void save(uint16_t iVal, const OctStr &content);
    void save(const std::string &filename, const OctStr &content);
    void remove(uint16_t iVal);

    void buildPath(std::string &pathName, uint16_t iVal);
    std::string makeFilename(uint16_t iVal);

    bool isIValFile(const std::string &fileName, uint16_t &iVal);
    int allFiles();
    int allFilenDirs();

    // void load(uint16_t iVal, ZipMan &content);
    bool isFileExist(uint16_t iVal);

private:
    std::string baseDir;
    std::string fileExt;
    int _allFiles(std::vector<std::string> &all);
    int _allFilenDirs(std::vector<std::string> &files, std::vector<std::string> &dirs);
};

} // namespace autocrypt