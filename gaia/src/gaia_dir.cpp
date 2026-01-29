#include "gaia_dir.hpp"
#include "gaia_exception.hpp"   // for SysException, THROWLOG
#include "gaia_string_util.hpp" // for startsWith, stricmp
#include <algorithm>            // for max, sort
#include <array>                // for array
#include <cerrno>               // for errno, EEXIST, ENOENT
#include <cstdio>               // for size_t, remove, snprintf
#include <cstring>              // for strerror, strlen
#include <dirent.h>             // for closedir, dirent, opendir, readdir, DIR
#include <fstream>              // for ifstream, ofstream, basic_ostream, endl
#include <iterator>             // for istreambuf_iterator, operator!=
#include <sys/stat.h>           // for stat, mkdir, S_ISREG, S_IRWXU, S_ISDIR
#include <unistd.h>             // for rmdir

namespace vp
{

enum
{
    eReadDir_File = 0x01,
    eReadDir_Dir = 0x02,
    eReadDir_ExcludeDot = 0x04,
};

static bool compName(const std::string &a, const std::string &b)
{
    return stricmp(a, b) < 0;
}

static size_t _readDir(const std::string &dirPath, const std::string &prefix, std::vector<std::string> &list, uint64_t flag)
{
    DIR *dirp = nullptr;
    dirp = opendir(dirPath.c_str());
    if (dirp == nullptr)
    {
        THROWLOG(SysException, "opendir({}) error({})[{}]",
                 dirPath, errno, strerror(errno));
    }

    struct dirent *ent = nullptr;
    while ((ent = readdir(dirp)) != nullptr)
    {
        if (static_cast<bool>(flag & eReadDir_ExcludeDot) && ent->d_name[0] == '.')
        {
            continue;
        }

        struct stat statBuf{};
        std::string pathname = joinDir(dirPath.c_str(), ent->d_name);
        if (stat(pathname.c_str(), &statBuf) < 0)
        {
            closedir(dirp);
            THROWLOG(SysException, "stat({}) error({})[{}]",
                     pathname, errno, strerror(errno));
        }

        if (((flag & eReadDir_File) && S_ISREG(statBuf.st_mode)) || ((flag & eReadDir_Dir) && S_ISDIR(statBuf.st_mode))) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
        {
            if (prefix.empty() || startsWith(const_cast<const char *>(ent->d_name), prefix))
            {
                list.push_back(const_cast<const char *>(ent->d_name));
            }
        }
    }
    closedir(dirp);
    sort(list.begin(), list.end(), compName);
    return list.size();
}

size_t readDirFiles(const std::string &dirPath, std::vector<std::string> &list, bool excludeDot)
{
    uint64_t flag = eReadDir_File;
    if (excludeDot)
    {
        flag |= eReadDir_ExcludeDot;
    }

    return _readDir(dirPath, "", list, flag);
}

size_t readDirDirs(const std::string &dirPath, std::vector<std::string> &list, bool excludeDot)
{
    uint64_t flag = eReadDir_Dir;
    if (excludeDot)
    {
        flag |= eReadDir_ExcludeDot;
    }

    return _readDir(dirPath, "", list, flag);
}

bool isDirExist(const std::string &dirName)
{
    struct stat statBuf{};
    if (stat(dirName.c_str(), &statBuf) == -1)
    {
        return false;
    }

    if (!S_ISDIR(statBuf.st_mode)) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
    {
        return false;
    }

    return true;
}

bool isFileExist(const std::string &fileName)
{
    struct stat statBuf{};
    if (stat(fileName.c_str(), &statBuf) == -1)
    {
        return false;
    }

    if (!S_ISREG(statBuf.st_mode)) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
    {
        return false;
    }

    return true;
}

bool isFileExist(const std::string &fileName, size_t &filesize)
{
    struct stat statBuf{};
    if (stat(fileName.c_str(), &statBuf) == -1)
    {
        return false;
    }

    if (!S_ISREG(statBuf.st_mode)) // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
    {
        return false;
    }

    filesize = statBuf.st_size;
    return true;
}

void makeDir(const std::string &dirName)
{
    if (isDirExist(dirName))
    {
        return;
    }

    if (mkdir(dirName.c_str(), 0755) == -1)
    {
        THROWLOG(SysException, "mkdir({}) error({})[{}]",
                 dirName, errno, strerror(errno));
    }
}

void makeDirRecursive(const std::string &dirname)
{
    std::array<char, 256> tmp{};
    size_t len = 0;

    snprintf(tmp.data(), tmp.size(), "%s", dirname.c_str()); // NOLINT: sprintf 대체 방식은 나중에 재고려
    len = strlen(tmp.data());

    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = 0;
    }

    for (char &ch : tmp)
    {
        if (ch == 0)
        {
            break;
        }

        if (ch == '/')
        {
            ch = 0;
            mkdir(tmp.data(), S_IRWXU); // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
            ch = '/';
        }
    }

    mkdir(tmp.data(), S_IRWXU); // NOLINT: 해당 매크로의 비트 연산 피연산자 중에 stat.h에서 이미 #define으로 정의된 int타입이 존재
}

bool makePath(const std::string &path)
{
    bool bSuccess = false;
    int nRC = ::mkdir(path.c_str(), 0775);
    if (nRC == -1)
    {
        switch (errno)
        {
        case ENOENT:
            // parent didn't exist, try to create it
            if (makePath(path.substr(0, path.find_last_of('/'))))
            {
                // Now, try to create again.
                bSuccess = 0 == ::mkdir(path.c_str(), 0775);
            }
            else
            {
                bSuccess = false;
            }
            break;
        case EEXIST:
            // Done!
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
        }
    }
    else
    {
        bSuccess = true;
    }
    return bSuccess;
}

void _readLine(const std::string &filename, std::string &line)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
    {
        THROWLOG(SysException, "Cannot open({})\n", filename);
    }

    getline(ifs, line);
}

void _writeLine(const std::string &filename, const std::string &line)
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        THROWLOG(SysException, "Cannot open({})\n", filename);
    }

    if (line.empty())
    {
        return;
    }

    ofs << line << std::endl;
}

void _readFile(const std::string &filename, void *ptr, size_t size)
{
    try
    {
        std::ifstream(filename, std::ifstream::binary).read(reinterpret_cast<char *>(ptr), size);
    }
    catch (const std::ifstream::failure &e)
    {
        THROWLOG(SysException, "ifstream({}) read failed : [{}]", filename, e.what());
    }
}

void _writeFile(const std::string &filename, void *ptr, size_t size)
{
    try
    {
        std::ofstream(filename, std::ifstream::binary).write(reinterpret_cast<char *>(ptr), size);
    }
    catch (const std::ofstream::failure &e)
    {
        THROWLOG(SysException, "ofstream({}) write failed : [{}]", filename, e.what());
    }
}

void readLines(const std::string &filename, std::vector<std::string> &lines, size_t linesToRead)
{
    if (!isFileExist(filename))
    {
        THROWLOG(SysException, "file({}) not found({})[{}]", filename, errno, strerror(errno));
    }

    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs.is_open())
    {
        THROWLOG(SysException, "ifstream({}) not opened", filename);
    }

    std::string line;
    if (linesToRead == 0)
    {
        while (std::getline(ifs, line))
        {
            lines.push_back(line);
        }
    }
    else
    {
        for (size_t i = 0; i < linesToRead; i++)
        {
            std::getline(ifs, line);
            lines.push_back(line);
        }
    }
}

std::string loadFile(const std::string &fileName)
{
    std::ifstream ifs(fileName, std::ifstream::binary);
    if (!ifs.is_open())
    {
        THROWLOG(SysException, "ifstream({}) not opened", fileName);
    }

    std::string fStr((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    ifs.close();
    return fStr;
}

void saveFile(const std::string &fileName, const std::string &fStr)
{
    std::ofstream ofs(fileName, std::ifstream::binary);
    if (!ofs.is_open())
    {
        THROWLOG(SysException, "ofstream({}) not opened", fileName);
    }

    ofs << fStr;
    ofs.close();
    return;
}

void removeDir(const std::string &pathname)
{
    if (isDirExist(pathname))
    {
        std::vector<std::string> vDir{};
        std::vector<std::string> vFile{};

        readDirDirs(pathname, vDir, true);
        readDirFiles(pathname, vFile, true);

        for (auto &dir : vDir)
        {
            removeDir(joinDir(pathname, dir));
        }
        for (auto &file : vFile)
        {
            removeFile(joinDir(pathname, file));
        }

        if (::rmdir(pathname.c_str()) != 0)
        {
            THROWLOG(SysException, "remove({}) error({})[{}]", pathname, errno, strerror(errno));
        }
    }
}

void removeFile(const std::string &pathname)
{
    if (std::remove(pathname.c_str()) != 0)
    {
        THROWLOG(SysException, "remove({}) error({})[{}]", pathname, errno, strerror(errno));
    }
}

bool fileNameExt(const std::string &fileName, std::string &name, std::string &ext)
{
    size_t pos = fileName.rfind('.');
    if (pos == std::string::npos) // '.' not found
    {
        name = fileName;
        ext.clear();
        return false;
    }

    name = fileName.substr(0, pos);
    ext = fileName.substr(pos + 1);
    return true;
}

} // namespace vp
