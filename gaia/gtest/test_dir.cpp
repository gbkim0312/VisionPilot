#include "gaia_dir.hpp"
#include <fstream>
#include <gtest/gtest.h>

#define DIR_SLASH "/"

namespace autocrypt
{

TEST(dirUtil, isDirExist)
{
    std::string dirName = "/tmp" DIR_SLASH "gtest";
    makeDir(dirName);

    std::cout << "check " << dirName << std::endl;
    ASSERT_TRUE(isDirExist(dirName));
    dirName += DIR_SLASH;
    std::cout << "check " << dirName << std::endl;
    ASSERT_TRUE(isDirExist(dirName));

    dirName = "/tmp" DIR_SLASH "gtest" DIR_SLASH "popsof";

    std::cout << "makeDir " << dirName << std::endl;
    makeDir(dirName);
    ASSERT_TRUE(isDirExist(dirName));

    dirName = "/tmp" DIR_SLASH "gtest" DIR_SLASH "popsof2" DIR_SLASH;

    std::cout << "makeDir " << dirName << std::endl;
    makeDir(dirName);
    ASSERT_TRUE(isDirExist(dirName));
}

TEST(dirUtil, removeDir) // 위에서 만든 디렉토리 구조에 파일을 넣은 뒤에 그 최상단 디렉토리를 삭제하는 테스트
{
    std::string filePath = "/tmp" DIR_SLASH "gtest" DIR_SLASH "file1"; // 최상단 디렉토리 아래 파일 하나 생성
    std::ofstream file1(filePath);
    ASSERT_TRUE(isFileExist(filePath));

    filePath = "/tmp" DIR_SLASH "gtest" DIR_SLASH "popsof" DIR_SLASH "file11"; // 하위 디렉토리 아래 파일 하나 생성 (1)
    std::ofstream file11(filePath);
    ASSERT_TRUE(isFileExist(filePath));

    filePath = "/tmp" DIR_SLASH "gtest" DIR_SLASH "popsof2" DIR_SLASH "file21"; // 하위 디렉토리 아래 파일 하나 생성 (2)
    std::ofstream file21(filePath);
    ASSERT_TRUE(isFileExist(filePath));

    std::string dirName = "/tmp" DIR_SLASH "gtest"; // 최상단 디렉토리 삭제
    std::cout << "rmDir " << dirName << std::endl;
    removeDir(dirName);
    ASSERT_FALSE(isDirExist(dirName));             // 최상단 디렉토리가 없어야 한다
    ASSERT_FALSE(isDirExist(dirName + "/popsof")); // 하위 디렉토리들도 없어야 한다
    ASSERT_FALSE(isDirExist(dirName + "/popsof2"));
    ASSERT_FALSE(isFileExist("/tmp" DIR_SLASH "gtest" DIR_SLASH "file1")); // 디렉토리 아래 파일들도 없어야 한다
    ASSERT_FALSE(isFileExist("/tmp" DIR_SLASH "gtest" DIR_SLASH "popsof" DIR_SLASH "file11"));
    ASSERT_FALSE(isFileExist("/tmp" DIR_SLASH "gtest" DIR_SLASH "popsof2" DIR_SLASH "file21"));
}

TEST(dirUtil, joinDir)
{
    std::string res;
    res = joinDir("a", "b");
    EXPECT_EQ(res, "a/b");

    res = joinDir("a", "b", "c");
    EXPECT_EQ(res, "a/b/c");

    res = joinDir("a", "b/c");
    EXPECT_EQ(res, "a/b/c");

    res = joinDir("a/b", "c");
    EXPECT_EQ(res, "a/b/c");

    res = joinDir("", "");
    EXPECT_EQ(res, "/");

    res = joinDir("a", "");
    EXPECT_EQ(res, "a/");

    res = joinDir("", "a");
    EXPECT_EQ(res, "/a");

    res = joinDir("a", "b", "c", "d");
    EXPECT_EQ(res, "a/b/c/d");

    res = joinDir("a", "b/", "c/", "d");
    EXPECT_EQ(res, "a/b//c//d");

    res = joinDir("a", "", "", "d");
    EXPECT_EQ(res, "a///d");
}

} // namespace autocrypt