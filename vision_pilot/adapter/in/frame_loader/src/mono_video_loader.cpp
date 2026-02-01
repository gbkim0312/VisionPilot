#include "mono_video_loader.hpp"
#include "gaia_dir.hpp"
#include "gaia_log.hpp"
#include "gaia_string_util.hpp"
#include "gaia_time.hpp"
#include "video_loader_config.hpp"
#include <exception>
#include <opencv2/imgcodecs.hpp>

namespace vp::adapter::in
{
MonoVideoLoader::~MonoVideoLoader()
{
    LOG_TRA("");
    this->stopPrefetch();
}

bool MonoVideoLoader::initialize()
{
    LOG_TRA("");

    auto *mono_param = std::get_if<config::MonoParam>(&config_.cameraParam);
    if (mono_param == nullptr)
    {
        LOG_ERR("MonoParam is not set in cameraParam.");
        return false;
    }

    LOG_INF("Opening video source: {}", mono_param->source);
    if (config_.dataType == config::DataType::FRAME_SET)
    {
        if (!this->scanDirectoryFiles())
        {
            LOG_ERR("Failed to start prefetch thread for frame set.");
            return false;
        }

        LOG_INF("Starting prefetch thread for frame set...");
        prefetch_running_ = true;
        prefetch_thread_ = std::thread(&MonoVideoLoader::prefetchLoop, this);
        return true;
    }

    video_capture_.open(mono_param->source);
    if (!video_capture_.isOpened())
    {
        LOG_ERR("Failed to open video source: {}", mono_param->source);
        return false;
    }

    return true;
}

void MonoVideoLoader::release()
{
    LOG_TRA("");

    this->stopPrefetch();

    if (video_capture_.isOpened())
    {
        video_capture_.release();
    }
}

bool MonoVideoLoader::fetchFrame()
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

bool MonoVideoLoader::fetchFrameFromVideo()
{
    LOG_TRA("");

    cv::Mat image;
    if (!video_capture_.read(image))
    {
        LOG_DBG("Failed to read frame from video source.");
        return false;
    }

    auto image_packet = this->createImagePacket(image);
    this->pushToQueue(image_packet);
    return true;
}

bool MonoVideoLoader::fetchFrameFromFrameSet()
{
    cv::Mat image;

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

        image = frame_buffer_.front().image;
        frame_buffer_.pop_front();

        // 빈 자리가 생겼으니 Producer 깨우기
        buffer_cv_.notify_one();
    }

    auto image_packet = this->createImagePacket(image);
    this->pushToQueue(image_packet);
    return true;
}

bool MonoVideoLoader::scanDirectoryFiles()
{
    LOG_TRA("");
    image_files_.clear();
    disk_read_index_ = 0;
    is_eof_ = false;

    auto *mono_param = std::get_if<config::MonoParam>(&config_.cameraParam);

    if (mono_param == nullptr)
    {
        LOG_ERR("MonoParam is not set in cameraParam.");
        return false;
    }
    try
    {
        vp::readDirFiles(mono_param->source, image_files_, true);
    }
    catch (const std::exception &e)
    {
        LOG_ERR("Error reading directory {}: {}", mono_param->source, e.what());
        return false;
    }

    if (image_files_.empty())
    {
        LOG_ERR("No files found in directory: {}", mono_param->source);
        return false;
    }

    LOG_INF("Found {} files to load from: {}", image_files_.size(), mono_param->source);
    return true;
}

void MonoVideoLoader::prefetchLoop()
{
    LOG_TRA("Prefetch thread started.");

    auto *mono_param = std::get_if<config::MonoParam>(&config_.cameraParam);
    if (!mono_param)
    {
        LOG_ERR("MonoParam is not set in cameraParam.");
        return;
    }

    while (prefetch_running_)
    {
        std::unique_lock<std::mutex> lock(buffer_mutex_);

        buffer_cv_.wait(lock, [this, mono_param]()
                        {
                            return !prefetch_running_ || frame_buffer_.size() < mono_param->maxImageBufferSize; // maxImageBufferSize가 없다면 상수로 대체 (예: 100)
                        });

        if (!prefetch_running_)
        {
            break;
        }
        // 3. 파일 끝 확인
        if (disk_read_index_ >= image_files_.size())
        {
            is_eof_ = true;
            lock.unlock();                                               // 대기하는 동안은 Lock 해제
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // EOF 상태면 천천히 돎
            continue;
        }

        auto file = image_files_[disk_read_index_];
        auto full_path = vp::joinDir(mono_param->source, file);

        std::string name{};
        std::string ext{};

        vp::fileNameExt(file, name, ext);

        lock.unlock();

        cv::Mat image;
        bool load_success = false;

        if (vp::stricmp(ext, "png") == 0) // 확장자 체크
        {
            switch (mono_param->colorFormat)
            {
            case config::ColorFormat::GRAYSCALE:
                image = cv::imread(full_path, cv::IMREAD_GRAYSCALE);
                break;
            default:
                image = cv::imread(full_path, cv::IMREAD_COLOR);
                break;
            }

            if (!image.empty())
            {
                load_success = true;
            }
            else
            {
                LOG_WRN("Failed to load image (empty): {}", full_path);
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
            FrameCache cache;
            cache.image = image;
            // cache.frame_id = disk_read_index_; // 필요하다면 ID 할당

            frame_buffer_.push_back(std::move(cache));

            buffer_cv_.notify_one();
        }

        ++disk_read_index_;
    }

    LOG_TRA("Prefetch thread stopped.");
}

void MonoVideoLoader::stopPrefetch()
{
    LOG_TRA("");

    prefetch_running_ = false;

    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        buffer_cv_.notify_all();
    }

    // 3. Join
    if (prefetch_thread_.joinable())
    {
        prefetch_thread_.join();
    }
}

std::shared_ptr<domain::model::ImagePacket> MonoVideoLoader::createImagePacket(const cv::Mat &image)
{
    auto mono_packet = std::make_shared<domain::model::MonoImagePacket>();
    auto &frame = mono_packet->frame;

    frame.channels = image.channels();
    frame.data.assign(image.data, image.data + (image.cols * image.rows * image.channels()));
    frame.height = image.rows;
    frame.width = image.cols;
    frame.step = static_cast<int>(image.step);

    auto image_packet = std::make_shared<domain::model::ImagePacket>(
        domain::model::ImagePacket{
            .format = domain::model::ImageFormat::MONO,
            .encoding = (image.channels() == 1) ? domain::model::ImageEncoding::MONO8 : domain::model::ImageEncoding::BGR8,
            .frame_id = frame_id_++,
            .timestamp = getTime64(),
            .payload = *mono_packet,
        });

    return image_packet;
}

} // namespace vp::adapter::in