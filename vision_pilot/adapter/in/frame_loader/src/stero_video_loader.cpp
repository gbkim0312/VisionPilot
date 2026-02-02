#include "gaia_dir.hpp"
#include "gaia_log.hpp"
#include "gaia_string_util.hpp"
#include "gaia_time.hpp"
#include "stereo_video_loader.hpp"
#include "video_loader_config.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

namespace vp::adapter::in
{
StereoVideoLoader::~StereoVideoLoader()
{
    LOG_TRA("");

    this->releaseImpl();
}

bool StereoVideoLoader::initialize()
{
    LOG_TRA("");

    const auto *stereo_param = std::get_if<config::StereoParam>(&config_.cameraParam);
    if (stereo_param == nullptr)
    {
        LOG_ERR("StereoParam is not set in cameraParam.");
        return false;
    }

    LOG_INF("Opening stereo video sources: left={}, right={}", stereo_param->leftSource, stereo_param->rightSource);
    if (config_.dataType == config::DataType::FRAME_SET)
    {
        if (!this->scanDirectoryFiles())
        {
            LOG_ERR("Failed to start prefetch thread for frame set.");
            return false;
        }

        LOG_INF("Starting prefetch thread for frame set...");
        prefetch_running_ = true;
        prefetch_thread_ = std::thread(&StereoVideoLoader::prefetchLoop, this);
        return true;
    }

    left_video_capture_.open(stereo_param->leftSource);
    if (!left_video_capture_.isOpened())
    {
        LOG_ERR("Failed to open left video source: {}", stereo_param->leftSource);
        return false;
    }

    right_video_capture_.open(stereo_param->rightSource);
    if (!right_video_capture_.isOpened())
    {
        LOG_ERR("Failed to open right video source: {}", stereo_param->rightSource);
        return false;
    }

    return true;
}

void StereoVideoLoader::release()
{
    LOG_TRA("");
    this->releaseImpl();
}

bool StereoVideoLoader::fetchFrame()
{
    switch (config_.dataType)
    {
    case config::DataType::VIDEO_FILE:
    case config::DataType::CAMERA_DEVICE:
    case config::DataType::RTSP_STREAM:
        return this->fetchFrameFromVideo();
    case config::DataType::FRAME_SET:
        return this->fetchFrameFromFrameSet();
    default:
        LOG_ERR("Unsupported DataType: {}", config::toString(config_.dataType));
        return false;
    }
}

bool StereoVideoLoader::releaseImpl()
{
    LOG_TRA("");

    this->stopPrefetch();

    if (left_video_capture_.isOpened())
    {
        left_video_capture_.release();
    }

    if (right_video_capture_.isOpened())
    {
        right_video_capture_.release();
    }
    return true;
}

bool StereoVideoLoader::fetchFrameFromVideo()
{
    LOG_TRA("");

    cv::Mat left_image;
    cv::Mat right_image;

    if (!left_video_capture_.read(left_image))
    {
        LOG_DBG("Failed to read frame from left video source.");
        return false;
    }

    if (!right_video_capture_.read(right_image))
    {
        LOG_DBG("Failed to read frame from right video source.");
        return false;
    }

    auto image_packet = this->createImagePacket(left_image, right_image);
    this->pushToQueue(image_packet);
    return true;
}
bool StereoVideoLoader::fetchFrameFromFrameSet()
{
    cv::Mat left_image;
    cv::Mat right_image;

    {
        std::unique_lock<std::mutex> lock(buffer_mutex_);

        if (frame_buffer_.empty())
        {
            if (is_eof_)
            {
                LOG_INF("End of frame set.");
                return false; // 재생 종료
            }

            return true;
        }

        const auto &front_frame = frame_buffer_.front();
        left_image = front_frame.left_image;
        right_image = front_frame.right_image;
        frame_buffer_.pop_front();

        buffer_cv_.notify_one();
    }

    auto image_packet = this->createImagePacket(left_image, right_image);
    this->pushToQueue(image_packet);
    return true;
}

bool StereoVideoLoader::scanDirectoryFiles()
{
    LOG_TRA("");

    left_image_files_.clear();
    right_image_files_.clear();
    disk_read_index_ = 0;
    is_eof_ = false;

    const auto *stereo_param = std::get_if<config::StereoParam>(&config_.cameraParam);
    if (stereo_param == nullptr)
    {
        return false;
    }

    try
    {
        vp::readDirFiles(stereo_param->leftSource, left_image_files_, true);
        vp::readDirFiles(stereo_param->rightSource, right_image_files_, true);
    }
    catch (const std::exception &e)
    {
        LOG_ERR("Error reading directory: {}", e.what());
        return false;
    }

    if (left_image_files_.empty() || right_image_files_.empty())
    {
        LOG_ERR("No files found in left or right directory: {}, {}", stereo_param->leftSource, stereo_param->rightSource);
        return false;
    }

    if (left_image_files_.size() != right_image_files_.size())
    {
        LOG_WRN("Number of left and right images do not match: {} left vs {} right", left_image_files_.size(), right_image_files_.size());
    }

    LOG_INF("Found {} left files and {} right files to load from: {}, {}", left_image_files_.size(), right_image_files_.size(), stereo_param->leftSource, stereo_param->rightSource);
    return true;
}

void StereoVideoLoader::prefetchLoop()
{
    LOG_TRA("Prefetch thread started.");

    const auto *stereo_param = std::get_if<config::StereoParam>(&config_.cameraParam);
    if (stereo_param == nullptr)
    {
        LOG_ERR("StereoParam is not set in cameraParam.");
        return;
    }

    while (prefetch_running_)
    {
        std::unique_lock<std::mutex> lock(buffer_mutex_);

        buffer_cv_.wait(lock, [this, stereo_param]()
                        {
                            return !prefetch_running_ || frame_buffer_.size() < stereo_param->maxImageBufferSize; // maxImageBufferSize가 없다면 상수로 대체 (예: 100)
                        });

        if (!prefetch_running_)
        {
            break;
        }

        if (disk_read_index_ >= left_image_files_.size() || disk_read_index_ >= right_image_files_.size())
        {
            is_eof_ = true;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        auto left_file = left_image_files_[disk_read_index_];
        auto left_file_full_path = vp::joinDir(stereo_param->leftSource, left_file);
        auto right_file = right_image_files_[disk_read_index_];
        auto right_file_full_path = vp::joinDir(stereo_param->rightSource, right_file);

        std::string left_name{};
        std::string left_ext{};

        std::string right_name{};
        std::string right_ext{};

        vp::fileNameExt(left_file, left_name, left_ext);
        vp::fileNameExt(right_file, right_name, right_ext);

        lock.unlock();

        cv::Mat left_image;
        cv::Mat right_image;
        bool load_success = false;

        // 왼쪽 이미지 로드
        if (vp::stricmp(left_ext, "png") == 0) // 확장자 체크
        {
            switch (stereo_param->colorFormat)
            {
            case config::ColorFormat::GRAYSCALE:
                left_image = cv::imread(left_file_full_path, cv::IMREAD_GRAYSCALE);
                break;
            default:
                left_image = cv::imread(left_file_full_path, cv::IMREAD_COLOR);
                break;
            }

            if (!left_image.empty())
            {
                load_success = true;
            }
            else
            {
                LOG_WRN("Failed to load left image: {}", left_file_full_path);
            }
        }

        if (vp::stricmp(right_ext, "png") == 0) // 확장자 체크
        {
            switch (stereo_param->colorFormat)
            {
            case config::ColorFormat::GRAYSCALE:
                right_image = cv::imread(right_file_full_path, cv::IMREAD_GRAYSCALE);
                break;
            default:
                right_image = cv::imread(right_file_full_path, cv::IMREAD_COLOR);
                break;
            }

            if (right_image.empty())
            {
                LOG_WRN("Failed to load right image: {}", right_file_full_path);
                load_success = false;
            }
        }

        lock.lock();

        if (!prefetch_running_)
        {
            break; // 읽는 동안 종료 신호 체크
        }

        if (load_success)
        {
            // FrameCache 구조체에 담기
            StereoFrame cache;
            cache.left_image = left_image;
            cache.right_image = right_image;

            frame_buffer_.push_back(std::move(cache));

            // Consumer(fetchFrameFromFrameSet)가 빈 버퍼에서 기다리고 있을 수 있으니 깨움
            buffer_cv_.notify_one();
        }
        ++disk_read_index_;
    }
    LOG_TRA("Prefetch thread stopped.");
}

void StereoVideoLoader::stopPrefetch()
{
    LOG_TRA("");

    prefetch_running_ = false;

    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        buffer_cv_.notify_all();
    }

    if (prefetch_thread_.joinable())
    {
        prefetch_thread_.join();
    }
}

std::shared_ptr<domain::model::ImagePacket> StereoVideoLoader::createImagePacket(const cv::Mat &left_image, const cv::Mat &right_image)
{
    auto stereo_packet = std::make_shared<domain::model::StereoImagePacket>();

    auto &left_frame = stereo_packet->left;
    left_frame.channels = left_image.channels();
    left_frame.data.assign(left_image.data, left_image.data + (left_image.cols * left_image.rows * left_image.channels())); // NOLINT: OPNECV
    left_frame.height = left_image.rows;
    left_frame.width = left_image.cols;
    left_frame.step = static_cast<int>(left_image.step);

    auto &right_frame = stereo_packet->right;
    right_frame.channels = right_image.channels();
    right_frame.data.assign(right_image.data, right_image.data + (right_image.cols * right_image.rows * right_image.channels())); // NOLINT: OPNECV
    right_frame.height = right_image.rows;
    right_frame.width = right_image.cols;
    right_frame.step = static_cast<int>(right_image.step);

    auto image_packet = std::make_shared<domain::model::ImagePacket>(
        domain::model::ImagePacket{
            .format = domain::model::ImageFormat::STEREO,
            .encoding = (left_image.channels() == 3) ? domain::model::ImageEncoding::BGR8 : domain::model::ImageEncoding::MONO8,
            .frame_id = frame_id_++,
            .timestamp = getTime64(),
            .payload = *stereo_packet,
        });

    return image_packet;
}

} // namespace vp::adapter::in