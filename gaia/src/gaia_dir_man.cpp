/* DirMan.cpp */

#include "gaia_dir_man.hpp"
#include "gaia_dir.hpp"                // for joinDir, saveFile, fileNameExt
#include "gaia_exception.hpp"          // for SysException, THROWLOG
#include "gaia_string_util.hpp"        // for stricmp
#include <algorithm>                   // for sort, max
#include <cctype>                      // for isxdigit
#include <cerrno>                      // for errno
#include <cstdint>                     // for uint16_t
#include <cstring>                     // for strerror, strcmp
#include <dirent.h>                    // for closedir, dirent, opendir
#include <spdlog/fmt/bundled/format.h> // for format
#include <sys/stat.h>                  // for stat, lstat, S_ISREG, S_ISDIR

namespace vp
{

static bool compName(const std::string &a, const std::string &b)
{
    return stricmp(a, b) < 0;
}

DirMan::DirMan(const std::string &dir)
{
    if (dir.length() == 0)
    {
        baseDir = ".";
    }
    else
    {
        baseDir = dir;
    }
    fileExt = "zip";
}

DirMan::DirMan(const std::string &dir, const std::string &fileExt)
{
    if (dir.length() == 0)
    {
        baseDir = ".";
    }
    else
    {
        baseDir = dir;
    }
    this->fileExt = fileExt;
}

int DirMan::allFiles()
{
    init();
    return _allFiles(files);
}

int DirMan::allFilenDirs()
{
    init();
    return _allFilenDirs(files, dirs);
}

int DirMan::_allFiles(std::vector<std::string> &all) // NOLINT
{
    DIR *dirp = opendir(baseDir.c_str());
    if (dirp == nullptr)
    {
        THROWLOG(SysException, "opendir({}) error({})[{}]", baseDir, errno, strerror(errno));
    }

    struct dirent *res = nullptr;
    std::string pathname;
    struct stat statBuf{};

    while ((res = readdir(dirp)) != nullptr)
    {
        pathname = joinDir(baseDir, res->d_name);
        if (stat(pathname.c_str(), &statBuf) < 0)
        {
            closedir(dirp);
            THROWLOG(SysException, "stat({}) error({})[{}]", pathname, errno, strerror(errno));
        }

        if (!S_ISREG(statBuf.st_mode)) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
        {
            continue;
        }

        all.push_back(static_cast<char *>(res->d_name));
    }
    closedir(dirp);
    std::sort(all.begin(), all.end(), compName);
    return all.size();
}

int DirMan::_allFilenDirs(std::vector<std::string> &files, std::vector<std::string> &dirs) // NOLINT
{
    DIR *dirp = opendir(baseDir.c_str());
    if (dirp == nullptr)
    {
        THROWLOG(SysException, "opendir({}) error({})[{}]", baseDir, errno, strerror(errno));
    }

    struct dirent *res = nullptr;
    std::string pathname;
    struct stat statBuf{};

    while ((res = readdir(dirp)) != nullptr)
    {
        pathname = joinDir(baseDir, res->d_name);
        if (lstat(pathname.c_str(), &statBuf) < 0)
        {
            closedir(dirp);
            THROWLOG(SysException, "stat({}) error({})[{}]", pathname, errno, strerror(errno));
        }

        if (S_ISREG(statBuf.st_mode)) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
        {
            files.push_back(static_cast<char *>(res->d_name));
        }
        else if (S_ISDIR(statBuf.st_mode)) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
        {
            if (strcmp(static_cast<char *>(res->d_name), ".") == 0 ||
                strcmp(static_cast<char *>(res->d_name), "..") == 0)
            {
                continue;
            }

            dirs.push_back(static_cast<char *>(res->d_name));
        }
    }
    closedir(dirp);
    std::sort(files.begin(), files.end(), compName);
    std::sort(dirs.begin(), dirs.end(), compName);
    return static_cast<int>(files.size() + dirs.size());
}

int DirMan::iValFiles(uint16_t fromI, uint16_t toI)
{
    init();

    std::vector<std::string> all; // NOLINT
    _allFiles(all);

    uint16_t iVal = 0;

    for (auto &it : all)
    {
        if (!isIValFile(it, iVal))
        {
            continue;
        }

        if (fromI <= iVal && iVal < toI)
        {
            files.push_back(it);
            values.insert(iVal);
        }
    }
    return files.size();
}

int DirMan::allIValFiles()
{
    init();

    std::vector<std::string> all; // NOLINT
    _allFiles(all);

    uint16_t iVal = 0;

    for (auto &it : all)
    {
        if (isIValFile(it, iVal))
        {
            files.push_back(it);
            values.insert(iVal);
        }
    }
    return files.size();
}

uint16_t DirMan::minI() const
{
    if (values.empty())
    {
        return 0;
    }

    auto it = values.begin();
    return *it;
}

uint16_t DirMan::maxI() const
{
    if (values.empty())
    {
        return 0;
    }

    auto it = values.end();
    --it;
    return *it;
}

// fromI <= iVal < toI
bool DirMan::isIValsIn(uint16_t fromI, uint16_t toI)
{
    for (; fromI < toI; fromI++)
    {
        if (values.find(fromI) == values.end())
        {
            return false;
        }
    }
    return true;
}

void DirMan::save(uint16_t iVal, const OctStr &content)
{
    // std::string filename = makeFilename(iVal);
    // std::string pathname = joinDir(baseDir, filename);

    // saveFile(pathname, str(content));
}

void DirMan::save(const std::string &filename, const OctStr &content) // NOLINT
{
    // std::string pathname = joinDir(baseDir, filename);
    // saveFile(pathname, str(content));
}

void DirMan::remove(uint16_t iVal)
{
    std::string filename = makeFilename(iVal);
    std::string pathname = joinDir(baseDir, filename);

    removeFile(pathname);
}

void DirMan::buildPath(std::string &pathname, uint16_t iVal)
{
    std::string filename = makeFilename(iVal);
    pathname = joinDir(baseDir, filename);
}

std::string DirMan::makeFilename(uint16_t iVal)
{
    return fmt::format("{:04X}.{}", iVal, fileExt);
}

bool DirMan::isIValFile(const std::string &fileName, uint16_t &iVal)
{
    std::string name;
    std::string ext;

    if (!fileNameExt(fileName, name, ext)) // no ext
    {
        return false;
    }

    if (stricmp(ext, fileExt) != 0)
    {
        return false;
    }

    if (name.length() != 4)
    {
        return false;
    }

    if (!static_cast<bool>(isxdigit(name[0])))
    {
        return false;
    }

    if (!static_cast<bool>(isxdigit(name[1])))
    {
        return false;
    }

    if (!static_cast<bool>(isxdigit(name[2])))
    {
        return false;
    }

    if (!static_cast<bool>(isxdigit(name[3])))
    {
        return false;
    }

    iVal = std::stoul(name, nullptr, 16);
    return true;
}

// void DirMan::load(uint16_t iVal, ZipMan &content)
// {
//     std::string pathname;
//     buildPath(pathname, iVal);

//     content.initFile(pathname);

//     return;
// }

bool DirMan::isFileExist(uint16_t iVal)
{
    std::string pathname;
    buildPath(pathname, iVal);

    return vp::isFileExist(pathname);
}

} // namespace vp