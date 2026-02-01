#include "video_loader.hpp"
#include "gaia_log.hpp"
#include "gaia_time.hpp"
#include "video_loader_config.hpp"
#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread>

namespace vp::adapter::in
{
VideoLoader::VideoLoader(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue)
    : config_{config}, event_queue_{event_queue}
{
    LOG_TRA("");
}

VideoLoader::~VideoLoader()
{
    LOG_TRA("");
    this->stop();
}

bool VideoLoader::start()
{
    LOG_TRA("");

    if (running_)
    {
        LOG_DBG("VideoLoader is already running.");
        return true;
    }

    if (!this->initialize())
    {
        LOG_ERR("Failed to initialize VideoLoader.");
        return false;
    }

    running_ = true;
    worker_thread_ = std::thread(&VideoLoader::runLoop, this);
    return true;
}

bool VideoLoader::stop()
{
    LOG_TRA("");

    if (!running_)
    {
        LOG_DBG("VideoLoader is not running.");
        return true;
    }

    running_ = false;
    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }

    this->release();
    return true;
}

void VideoLoader::pushToQueue(std::shared_ptr<domain::model::ImagePacket> frame_packet)
{
    domain::model::Event evt;
    evt.type = domain::model::EventType::IMAGE;
    evt.timestamp = vp::getTime64();
    evt.source = "VideoLoader";
    evt.data = frame_packet;

    event_queue_.push(std::move(evt));
}

void VideoLoader::runLoop()
{
    LOG_TRA("");

    switch (config_.dataType)
    {
    case config::DataType::VIDEO_FILE:
    case config::DataType::FRAME_SET:
        this->loadFramesFromFile();
        break;
    case config::DataType::RTSP_STREAM:
    case config::DataType::CAMERA_DEVICE:
        this->loadFramesFromCameraDevice();
        break;
    default:
        LOG_ERR("Unsupported DataType: {}", config::toString(config_.dataType));
        break;
    }
}

void VideoLoader::loadFramesFromFile()
{
    LOG_TRA("");

    constexpr auto kDefaultFps = 30;
    uint32_t target_fps = config_.fps > 0 ? config_.fps : kDefaultFps;

    auto interval = std::chrono::nanoseconds(1000000000 / target_fps);
    auto next_frame_time = std::chrono::steady_clock::now();

    while (running_)
    {
        next_frame_time += interval;

        if (!this->fetchFrame())
        {
            LOG_DBG("No more frames to read (EOF) or Fetch Error.");
            break;
        }

        std::this_thread::sleep_until(next_frame_time);
    }
}

void VideoLoader::loadFramesFromCameraDevice()
{
    LOG_TRA("");

    constexpr auto kDefaultSleepMs = 100;
    while (running_)
    {
        if (!this->fetchFrame())
        {
            LOG_DBG("Failed to fetch frame from camera device.");
            std::this_thread::sleep_for(std::chrono::milliseconds(kDefaultSleepMs));
            continue;
        }
    }
}
} // namespace vp::adapter::in