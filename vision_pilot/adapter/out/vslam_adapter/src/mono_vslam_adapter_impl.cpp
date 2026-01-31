#include "mono_vslam_adapter_impl.hpp"
#include "gaia_log.hpp"
#include "stella_vslam/config.h"
#include <exception>
#include <opencv2/imgcodecs.hpp>

namespace vp::adapter::out
{
MonoVSlamAdapterImpl::MonoVSlamAdapterImpl(const config::VslamAdapterConfig &vslam_config)
    : vslam_config_{vslam_config}
{
    LOG_TRA("");
}

MonoVSlamAdapterImpl::~MonoVSlamAdapterImpl()
{
    LOG_TRA("");
    if (slam_system_)
    {
        slam_system_->shutdown();
    }
}

bool MonoVSlamAdapterImpl::initialize()
{
    LOG_INF("Initializing VSLAM Adapter...");

    if (is_initialized_)
    {
        LOG_INF("VSLAM Adapter is already initialized.");
        return true;
    }

    try
    {
        LOG_INF("Loading VSLAM configuration from: {}", vslam_config_.vslamConfigFilePath);
        auto config = std::make_shared<stella_vslam::config>(vslam_config_.vslamConfigFilePath);
        LOG_INF("VSLAM configuration loaded successfully.");

        LOG_INF("Creating VSLAM system...");
        slam_system_ = std::make_shared<stella_vslam::system>(config, vslam_config_.vocabPath);
        LOG_INF("VSLAM system created successfully.");

        LOG_INF("Starting up VSLAM system...");
        slam_system_->startup();
        LOG_INF("VSLAM system started successfully.");

        is_initialized_ = true;
    }
    catch (const std::exception &e)
    {
        LOG_ERR("VSLAM initialization failed: {}", e.what());
        return false;
    }

    return true;
}

domain::model::Pose MonoVSlamAdapterImpl::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    LOG_TRA("");

    if (!is_initialized_ || slam_system_ == nullptr)
    {
        LOG_ERR("VSLAM system is not initialized.");
        return domain::model::Pose{};
    }

    while (slam_system_->loop_BA_is_running())
    {
        LOG_DBG("Waiting for loop bundle adjustment to complete...");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        // TODO(GBKIM): 실제 구현 시에는 빈 포즈나 이 전 pose를 내보내도록 고려 필요
    }

    const auto *mono_payload = std::get_if<domain::model::MonoImagePacket>(&image.payload);
    if (image.format != domain::model::ImageFormat::MONO || mono_payload == nullptr)
    {
        LOG_ERR("Invalid image payload for Mono VSLAM Adapter.");
        return domain::model::Pose{};
    }

    const auto rows = mono_payload->frame.height;
    const auto cols = mono_payload->frame.width;
    const auto channels = mono_payload->frame.channels;
    auto type = channels == 3 ? CV_8UC3 : CV_8UC1; // NOLINT: OPENCV

    cv::Mat img(rows, cols, type, const_cast<uint8_t *>(mono_payload->frame.data.data())); // NOLINT: OPENCV

    if (img.empty())
    {
        LOG_ERR("Failed to decode frame at ts: {}", timestamp);
        return domain::model::Pose{};
    }

    constexpr auto kMicroSecondsInSecond = 1000000;
    double time_in_seconds = static_cast<double>(timestamp) / kMicroSecondsInSecond;

    LOG_DBG("Frame size: {}x{}, Timestamp: {}, | color: {}, Time (s): {:.6f}", cols, rows, timestamp, channels == 3 ? "true" : "false", time_in_seconds);

    auto raw_pose = slam_system_->feed_monocular_frame(img, time_in_seconds);

    domain::model::Pose pose;
    if (raw_pose)
    {
        const Eigen::Matrix4d mat = raw_pose->cast<double>();
        const Eigen::Matrix3d rot = mat.block<3, 3>(0, 0);
        const Eigen::Vector3d trans = mat.block<3, 1>(0, 3);
        const Eigen::Quaterniond q(rot);

        pose.x = trans.x();
        pose.y = trans.y();
        pose.z = trans.z();
        pose.qw = q.w();
        pose.qx = q.x();
        pose.qy = q.y();
        pose.qz = q.z();
        pose.is_lost = false;

        LOG_DBG("Pose: Position({:.3f}, {:.3f}, {:.3f}), Orientation({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                pose.x, pose.y, pose.z,
                pose.qx, pose.qy, pose.qz, pose.qw);
    }
    else
    {
        pose.is_lost = true;
        LOG_DBG("VSLAM tracking lost at ts: {}", timestamp);
    }

    return pose;
}

bool MonoVSlamAdapterImpl::stop()
{
    LOG_TRA("Stopping VSLAM Adapter...");

    if (!is_initialized_)
    {
        LOG_WRN("VSLAM Adapter is not initialized.");
        return true;
    }

    if (slam_system_)
    {
        slam_system_->shutdown();
        LOG_INF("VSLAM system shut down successfully.");
    }
    else
    {
        LOG_WRN("VSLAM system was not initialized.");
    }

    is_initialized_ = false;
    return true;
}

} // namespace vp::adapter::out
