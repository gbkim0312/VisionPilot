#include "gaia_dir.hpp"
#include "gaia_octstr.hpp"
#include "gaia_zip_manager.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <vector>

namespace
{
constexpr auto kZipRootDir = "zip_dir";
constexpr auto kZipRootPath = "zip_dir/test.zip";
constexpr auto kFilename1 = "file_one";
constexpr auto kFilename2 = "file_another";
constexpr auto kDirpath1 = "temp/";
constexpr auto kDirpath2 = "temp/with/subdir/";
autocrypt::OctStr kFileData1 = std::string{"hi"};
autocrypt::OctStr kFileData2 = std::string{"ok.."};

void resetDir(const std::string &dir_path)
{
    try
    {
        if (autocrypt::isDirExist(dir_path))
        {
            autocrypt::removeDir(dir_path);
        }
    }
    catch (...)
    {
        // Do nothing
    }
}
} // namespace

namespace autocrypt
{
class TestZip : public testing::Test
{
protected:
    void SetUp() override
    {
        makeDir(kZipRootDir);
    }

    void TearDown() override
    {
        resetDir(kZipRootDir);
    }
};

TEST_F(TestZip, AddFileAndGetZipValidity)
{
    ZipManager zip{};
    ZipManager another_zip{};

    zip.addFile(kFilename1, kFileData1);
    zip.addFile(kFilename2, kFileData2);
    auto new_zip_oer = zip.saveToMem();

    ASSERT_FALSE(new_zip_oer.empty());
    ASSERT_TRUE(another_zip.loadFromMem(new_zip_oer));
    ASSERT_EQ(another_zip.getFile(kFilename1), kFileData1);
    ASSERT_EQ(another_zip.getFile(kFilename2), kFileData2);
}

TEST_F(TestZip, LoadAndGetFileValidity)
{
    ZipManager zip{};
    ZipManager zip_another{};

    zip.addFile(kFilename1, kFileData1);
    zip.addFile(kFilename2, kFileData2);

    ASSERT_TRUE(zip.saveToFile(kZipRootPath));
    ASSERT_TRUE(zip_another.loadFromFile(kZipRootPath));
    ASSERT_EQ(zip_another.getFile(kFilename1), kFileData1);
}

TEST_F(TestZip, SingleSaveAndLoad)
{
    ZipManager zip{};
    zip.addFile(kFilename1, kFileData1);
    zip.addFile(kFilename2, kFileData2);
    zip.saveToFile(kZipRootPath);
    zip.loadFromFile(kZipRootPath);

    ASSERT_EQ(zip.getFile(kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(kFilename2), kFileData2);
}

TEST_F(TestZip, DirectoryHierarchyWithFile)
{
    std::string directory_path = kDirpath1;
    std::string directory_path_deeper = kDirpath2;
    ZipManager zip{};

    zip.addFile(kFilename1, kFileData1);
    zip.addFile(directory_path_deeper + kFilename1, kFileData1);
    zip.addFile(directory_path_deeper + kFilename2, kFileData2);
    zip.addFile(kFilename2, kFileData2);
    zip.addFile(directory_path + kFilename1, kFileData1);
    zip.saveToFile(kZipRootPath);
    zip.loadFromFile(kZipRootPath);

    ASSERT_EQ(zip.getFile(kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(kFilename2), kFileData2);
    ASSERT_EQ(zip.getFile(directory_path + kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(directory_path_deeper + kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(directory_path_deeper + kFilename2), kFileData2);
    ASSERT_EQ(zip.size(), 5);
    ASSERT_EQ(zip.getAllFileNames().size(), 5);
    ASSERT_EQ(zip.getDirectories().size(), 2);
}

TEST_F(TestZip, DirectoryHierarchyWithMemory)
{
    std::string directory_path = kDirpath1;
    std::string directory_path_deeper = kDirpath2;
    ZipManager zip{};

    zip.addFile(kFilename1, kFileData1);
    zip.addFile(directory_path_deeper + kFilename1, kFileData1);
    zip.addFile(directory_path_deeper + kFilename2, kFileData2);
    zip.addFile(kFilename2, kFileData2);
    zip.addFile(directory_path + kFilename1, kFileData1);
    auto zip_data = zip.saveToMem();
    zip.loadFromMem(zip_data);

    ASSERT_EQ(zip.getFile(kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(kFilename2), kFileData2);
    ASSERT_EQ(zip.getFile(directory_path + kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(directory_path_deeper + kFilename1), kFileData1);
    ASSERT_EQ(zip.getFile(directory_path_deeper + kFilename2), kFileData2);
    ASSERT_EQ(zip.size(), 5);
    ASSERT_EQ(zip.getAllFileNames().size(), 5);
    ASSERT_EQ(zip.getDirectories().size(), 2);
}
} // namespace autocrypt