#include "video_loader_impl.hpp"
#include "gaia_dir.hpp"
#include "gaia_log.hpp"
#include "gaia_string_util.hpp"
#include "gaia_time.hpp"
#include <exception>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

namespace
{

std::shared_ptr<vp::domain::model::ImagePacket> createImagePacketFromMat(const cv::Mat &frame, uint64_t frame_id)
{
    auto frame_packet = std::make_shared<vp::domain::model::ImagePacket>();

    auto &mono_packet = frame_packet->payload.emplace<vp::domain::model::MonoImagePacket>();

    mono_packet.frame.width = frame.cols;
    mono_packet.frame.height = frame.rows;
    mono_packet.frame.channels = frame.channels();
    mono_packet.frame.step = static_cast<int>(frame.step);
    mono_packet.frame.data.assign(frame.data, frame.data + (frame.total() * frame.elemSize())); // NOLINT: OPENCV

    frame_packet->timestamp = vp::getTime64();
    frame_packet->encoding = (frame.channels() == 1) ? vp::domain::model::ImageEncoding::MONO8 : vp::domain::model::ImageEncoding::BGR8; // TODO: 추후 RGB8 등도 지원
    frame_packet->format = vp::domain::model::ImageFormat::MONO;
    frame_packet->frame_id = frame_id;

    return frame_packet;
}
} // namespace

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
        LOG_INF("Setting FPS to {}", config_.fps);
        video_capture_->set(cv::CAP_PROP_FPS, config_.fps);
    }

    if (config_.frameSize.width > 0 && config_.frameSize.height > 0)
    {
        LOG_INF("Setting frame size to {}x{}", config_.frameSize.width, config_.frameSize.height);
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

    switch (config_.sourceType)
    {
    case config::SourceType::VIDEO_FILE:
        this->loadFramesFromVideoFile();
        break;
    case config::SourceType::CAMERA_DEVICE:
        this->loadFramesFromCameraDevice();
        break;
    case config::SourceType::RTSP_STREAM:
        this->loadFramesFromRtspStream();
        break;
    case config::SourceType::FRAME_SET:
        this->loadFramesFromFrameSet();
        break;
    default:
        LOG_ERR("Unsupported source type.");
        break;
    }

    LOG_INF("Frame loading stopped.");
}

void VideoLoaderImpl::loadFramesFromVideoFile()
{
    LOG_TRA("");

    cv::Mat frame;
    auto sleep_ms = config_.fps > 0 ? static_cast<int>(1000 / config_.fps) : 30;

    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        if (!video_capture_->read(frame))
        {
            continue;
        }

        // 1. 데이터 패킷 생성
        auto frame_packet = ::createImagePacketFromMat(frame, ++frame_id_);

        // 2. 이벤트를 생성하여 큐에 Push (std::variant 사용)
        domain::model::Event evt;
        evt.type = domain::model::EventType::IMAGE;
        evt.timestamp = vp::getTime64();
        evt.source = "VideoLoader";
        evt.data = frame_packet; // ImageEventPayload (shared_ptr)로 자동 변환됨

        event_queue_.push(std::move(evt));
    }
}

void VideoLoaderImpl::loadFramesFromCameraDevice()
{
    LOG_TRA("");

    cv::Mat frame;

    while (running_)
    {
        if (!video_capture_->read(frame))
        {
            continue;
        }

        auto frame_packet = ::createImagePacketFromMat(frame, ++frame_id_);

        domain::model::Event evt;
        evt.type = domain::model::EventType::IMAGE;
        evt.timestamp = vp::getTime64();
        evt.source = "VideoLoader";
        evt.data = frame_packet;

        event_queue_.push(std::move(evt));
    }
}

void VideoLoaderImpl::loadFramesFromRtspStream()
{
    LOG_TRA("");

    cv::Mat frame;

    while (running_)
    {
        if (!video_capture_->read(frame))
        {
            continue;
        }

        auto frame_packet = ::createImagePacketFromMat(frame, ++frame_id_);

        domain::model::Event evt;
        evt.type = domain::model::EventType::IMAGE;
        evt.timestamp = vp::getTime64();
        evt.source = "VideoLoader";
        evt.data = frame_packet;

        event_queue_.push(std::move(evt));
    }
}

void VideoLoaderImpl::loadFramesFromFrameSet()
{
    LOG_TRA("");

    cv::Mat frame;
    auto sleep_ms = config_.fps > 0 ? static_cast<int>(1000 / config_.fps) : 30;
    std::vector<cv::Mat> buffer_frames = this->loadFramesFromDirectory();

    for (const auto &frm : buffer_frames)
    {
        if (!running_)
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));

        auto frame_packet = ::createImagePacketFromMat(frm, ++frame_id_);

        domain::model::Event evt;
        evt.type = domain::model::EventType::IMAGE;
        evt.timestamp = vp::getTime64();
        evt.source = "VideoLoader";
        evt.data = frame_packet;

        event_queue_.push(std::move(evt));
    }
}

std::vector<cv::Mat> VideoLoaderImpl::loadFramesFromDirectory()
{
    LOG_TRA("Loading frames from directory: {}", config_.source);
    std::vector<cv::Mat> frames;

    std::vector<std::string> all_files;
    try
    {
        vp::readDirFiles(config_.source, all_files, true);
    }
    catch (const std::exception &e)
    {
        LOG_ERR("Failed to read directory: {}. Error: {}", config_.source, e.what());
        return frames;
    }

    for (const auto &filename : all_files)
    {
        std::string name;
        std::string ext;

        vp::fileNameExt(filename, name, ext);

        // TODO: 추후 .jpg, .bmp 등 다양한 데이터셋 형식 지원 필요 시 extension 체크 로직 확장
        if (vp::stricmp(ext, "png") == 0)
        {
            std::string full_path = vp::joinDir(config_.source, filename);

            cv::Mat img = cv::imread(full_path, cv::IMREAD_COLOR);
            if (img.empty())
            {
                LOG_WRN("Failed to load image: {}", full_path);
                continue;
            }
            frames.push_back(img);
        }
    }

    LOG_INF("Successfully loaded {} PNG frames from {}", frames.size(), config_.source);
    return frames;
}

} // namespace vp::adapter::in::frame_loader