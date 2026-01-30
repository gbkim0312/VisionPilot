#include "gaia_zip_manager.hpp"
#include "gaia_exception.hpp" // for SysException, THROWLOG
#include "gaia_log.hpp"       // for LOG_TRA, LOG_DBG, LOG_ERR
#include "miniz.h"            // for mz_zip_archive
#include "miniz.h"            // for mz_bool, MZ_TRUE
#include <utility>            // for pair

namespace
{
std::pair<uint32_t, std::string> parseLatestZipError(mz_zip_archive &zip)
{
    auto error = mz_zip_get_last_error(&zip);
    return {static_cast<uint32_t>(error), mz_zip_get_error_string(error)};
}
} // namespace

namespace vp
{

bool ZipManager::loadFromMem(const OctStr &content)
{
    mz_zip_archive zip{};
    mz_zip_zero_struct(&zip);
    latest_error_code_ = 0;
    latest_error_str_.clear();

    mz_bool rc = mz_zip_reader_init_mem(&zip, content.data(), content.size(), 0);
    if (rc == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_reader_end(&zip);
        return false;
    }

    if (!this->readAllFiles(&zip))
    {
        this->parseLatestZipError(&zip);
        mz_zip_reader_end(&zip);
        return false;
    }

    mz_zip_reader_end(&zip);
    return true;
}

bool ZipManager::loadFromFile(const std::string &zip_filepath)
{
    mz_zip_archive zip{};
    mz_zip_zero_struct(&zip);
    latest_error_code_ = 0;
    latest_error_str_.clear();

    mz_bool rc = mz_zip_reader_init_file(&zip, zip_filepath.c_str(), 0);
    if (rc == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_reader_end(&zip);
        return false;
    }

    if (!this->readAllFiles(&zip))
    {
        this->parseLatestZipError(&zip);
        mz_zip_reader_end(&zip);
        return false;
    }

    mz_zip_reader_end(&zip);
    return true;
}

bool ZipManager::saveToFile(const std::string &zip_filepath)
{
    mz_zip_archive zip{};
    mz_zip_zero_struct(&zip);
    latest_error_code_ = 0;
    latest_error_str_.clear();

    if (mz_zip_writer_init_file(&zip, zip_filepath.c_str(), 0) == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return false;
    }
    if (!this->writeAllFiles(&zip))
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return false;
    }
    if (mz_zip_writer_finalize_archive(&zip) == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return false;
    }
    if (mz_zip_writer_end(&zip) == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return false;
    }
    return true;
}

OctStr ZipManager::saveToMem()
{
    mz_zip_archive zip{};
    mz_zip_zero_struct(&zip);
    latest_error_code_ = 0;
    latest_error_str_.clear();

    mz_uint64 total_filesize = 0;
    for (auto &file_data_pair : archive_)
    {
        total_filesize += file_data_pair.second.size();
    }

    mz_bool rc = mz_zip_writer_init_heap(&zip, total_filesize, total_filesize);
    if (rc == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return {};
    }

    if (!this->writeAllFiles(&zip))
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return {};
    }

    size_t ret_size = 0;
    uint8_t *ptr = nullptr;
    rc = mz_zip_writer_finalize_heap_archive(&zip, reinterpret_cast<void **>(&ptr), &ret_size);
    if (rc == MZ_FALSE)
    {
        this->parseLatestZipError(&zip);
        mz_zip_writer_end(&zip);
        return {};
    }
    OctStr result{ptr, ret_size};
    mz_free(ptr);
    mz_zip_writer_end(&zip);

    return std::move(result);
}

uint32_t ZipManager::getLatestErrorCode()
{
    return latest_error_code_;
}

const std::string &ZipManager::getLatestErrorMessage()
{
    return latest_error_str_;
}

std::set<std::string> ZipManager::getDirectories()
{
    std::set<std::string> filenames{};
    for (auto &file_data_pair : archive_)
    {
        auto &filename = file_data_pair.first;
        auto forward_slash_pos = filename.find_last_of('/');
        bool has_directory = forward_slash_pos != std::string::npos;
        if (has_directory)
        {
            filenames.insert(filename.substr(0, forward_slash_pos + 1));
        }
    }
    return std::move(filenames);
}

std::vector<std::string> ZipManager::getAllFileNames()
{
    std::vector<std::string> filenames{};
    filenames.reserve(archive_.size());
    for (auto &file_data_pair : archive_)
    {
        filenames.push_back(file_data_pair.first);
    }
    return std::move(filenames);
}

OctStr ZipManager::getFile(const std::string &filename)
{
    auto it = archive_.find(filename);
    if (it == archive_.end())
    {
        return {};
    }
    return it->second;
}

void ZipManager::addFile(const std::string &filename, const OctStr &filedata)
{
    archive_.insert({filename, filedata});
}

void ZipManager::removeFile(const std::string &filename)
{
    archive_.erase(filename);
}

void ZipManager::clear()
{
    archive_.clear();
}

size_t ZipManager::size() const
{
    return archive_.size();
}

void ZipManager::parseLatestZipError(zip_t *zip_ptr)
{
    auto error = mz_zip_get_last_error(static_cast<mz_zip_archive *>(zip_ptr));
    latest_error_code_ = static_cast<uint32_t>(error);
    latest_error_str_ = mz_zip_get_error_string(error);
}

bool ZipManager::readAllFiles(zip_t *zip_ptr)
{
    auto &zip = *static_cast<mz_zip_archive *>(zip_ptr);
    archive_.clear();

    auto file_count = static_cast<uint32_t>(mz_zip_reader_get_num_files(&zip));
    if (file_count == 0)
    {
        return true;
    }
    mz_zip_archive_file_stat file_stat{};
    for (uint32_t i = 0; i < file_count; i++)
    {
        if (mz_zip_reader_file_stat(&zip, i, &file_stat) == MZ_FALSE)
        {
            continue;
        }
        if (file_stat.m_is_directory == MZ_TRUE)
        {
            // Skip directory
            continue;
        }
        OctStr file_data{};
        file_data.resize(file_stat.m_uncomp_size);
        auto rc = mz_zip_reader_extract_file_to_mem(&zip, static_cast<char *>(file_stat.m_filename), file_data.data(), file_stat.m_uncomp_size, 0);
        if (rc == MZ_FALSE)
        {
            return false;
        }
        archive_.insert({file_stat.m_filename, std::move(file_data)});
    }
    return true;
}

bool ZipManager::writeAllFiles(zip_t *zip_ptr)
{
    auto &zip = *static_cast<mz_zip_archive *>(zip_ptr);
    mz_bool rc = MZ_TRUE;
    for (auto &file_data_pair : archive_)
    {
        auto filename = file_data_pair.first;
        auto forward_slash_pos = filename.find_last_of('/');
        bool has_directory = forward_slash_pos != std::string::npos;
        if (has_directory)
        {
            rc = mz_zip_writer_add_mem(&zip, filename.substr(0, forward_slash_pos + 1).c_str(), nullptr, 0, MZ_BEST_COMPRESSION);
            if (rc == MZ_FALSE)
            {
                return false;
            }
        }
        if (file_data_pair.second.empty())
        {
            continue;
        }

        rc = mz_zip_writer_add_mem(&zip, filename.c_str(), file_data_pair.second.data(), file_data_pair.second.size(), MZ_BEST_COMPRESSION);
        if (rc == MZ_FALSE)
        {
            return false;
        }
    }
    return true;
}

} // namespace vp