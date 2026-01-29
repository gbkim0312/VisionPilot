#pragma once

#include <cstddef> // for size_t
#include <cstdint> // for uint64_t
#include <string>  // for string, allocator, operator+, char_traits
#include <vector>  // for vector

namespace vp
{

template <typename Arg1, typename Arg2>
std::string joinDir(const Arg1 &cd1, const Arg2 &cd2)
{
    return std::string(cd1) + std::string("/") + std::string(cd2);
}

template <typename Arg, typename... Args>
std::string joinDir(const Arg &cd1, const Args &...cds)
{
    return std::string(cd1) + std::string("/") + joinDir(cds...);
}

bool isDirExist(const std::string &dirName);
bool isFileExist(const std::string &fileName);
bool isFileExist(const std::string &fileName, size_t &filesize);
void makeDir(const std::string &dirName);
void makeDirRecursive(const std::string &dirname);
bool makePath(const std::string &path);

// readDir 함수들은 결과를 대소문자를 ignore하고 sort한다.
// excludeDot : 이름이 '.'으로 시작하는 file(or directory)의 제외여부.
size_t readDirFiles(const std::string &dirPath,
                    std::vector<std::string> &list, bool excludeDot = false);
size_t readDirDirs(const std::string &dirPath,
                   std::vector<std::string> &list, bool excludeDot = false);

void readLines(const std::string &filename, std::vector<std::string> &lines, size_t linesToRead = 0);

std::string loadFile(const std::string &fileName);
void saveFile(const std::string &fileName, const std::string &fStr);

// remove directory and its subdirectories
void removeDir(const std::string &pathname);
void removeFile(const std::string &pathname);

bool fileNameExt(const std::string &fileName, std::string &name, std::string &ext);

} // namespace vp