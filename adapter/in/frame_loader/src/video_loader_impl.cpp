#include "video_loader_impl.hpp"
#include "gaia_log.hpp"
#include "gaia_time.hpp"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

namespace vp::adapter::in::frame_loader
{
VideoLoaderImpl::VideoLoaderImpl(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue)
    : config_{config}, event_queue_{event_queue}
{
    LOG_INF("VideoLoaderImpl created with source: {}", config_.source);
}

VideoLoaderImpl::~VideoLoaderImpl()
{
    LOG_TRA("");

    this->stop();
}

bool VideoLoaderImpl::start()
{
    LOG_TRA("");

    video_capture_ = std::make_unique<cv::VideoCapture>(config_.source);
    if (!video_capture_->isOpened())
    {
        LOG_ERR("Failed to open video source: {}", config_.source);
        return false;
    }

    if (config_.fps > 0)
    {
        video_capture_->set(cv::CAP_PROP_FPS, config_.fps);
    }

    if (config_.frameSize.width > 0 && config_.frameSize.height > 0)
    {
        video_capture_->set(cv::CAP_PROP_FRAME_WIDTH, config_.frameSize.width);
        video_capture_->set(cv::CAP_PROP_FRAME_HEIGHT, config_.frameSize.height);
    }

    running_ = true;
    worker_thread_ = std::thread(&VideoLoaderImpl::loadFrames, this);
    return true;
}

bool VideoLoaderImpl::stop()
{
    LOG_TRA("");

    running_ = false;
    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }
    return true;
}

void VideoLoaderImpl::loadFrames()
{
    LOG_INF("Frame loading started.");

    cv::Mat frame;
    while (running_)
    {
        if (!video_capture_->read(frame))
        {
            continue;
        }

        // 1. 데이터 패킷 생성
        auto frame_packet = std::make_shared<domain::model::ImagePacket>();
        frame_packet->width = frame.cols;
        frame_packet->height = frame.rows;
        frame_packet->data.assign(frame.data, frame.data + (frame.total() * frame.elemSize()));

        // 2. 이벤트를 생성하여 큐에 Push (std::variant 사용)
        domain::model::Event evt;
        evt.type = domain::model::EventType::IMAGE;
        evt.timestamp = vp::getTime64();
        evt.source = "VideoLoader";
        evt.data = frame_packet; // ImageEventPayload (shared_ptr)로 자동 변환됨

        event_queue_.push(std::move(evt));
    }

    LOG_INF("Frame loading stopped.");
}

} // namespace vp::adapter::in::frame_loader