#include "stereo_vslam_adapter_impl.hpp"
#include "gaia_log.hpp"
#include <stella_vslam/config.h>

namespace vp::adapter::out
{
StereoVSlamAdapterImpl::StereoVSlamAdapterImpl(const config::VslamAdapterConfig &vslam_config)
    : vslam_config_(vslam_config)
{
}

StereoVSlamAdapterImpl::~StereoVSlamAdapterImpl()
{
    LOG_TRA("");
    if (slam_system_)
    {
        slam_system_->shutdown();
    }
}

bool StereoVSlamAdapterImpl::initialize()
{
    LOG_INF("Initializing Stereo VSLAM Adapter...");

    if (is_initialized_)
    {
        LOG_INF("Stereo VSLAM Adapter is already initialized.");
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

domain::model::Pose StereoVSlamAdapterImpl::update(const domain::model::ImagePacket &image, uint64_t /* timestamp */)
{
    LOG_TRA("");

    if (!is_initialized_ || slam_system_ == nullptr)
    {
        LOG_ERR("Stereo VSLAM Adapter is not initialized.");
        return domain::model::Pose{};
    }

    while (slam_system_->loop_BA_is_running())
    {
        LOG_DBG("Waiting for loop bundle adjustment to complete...");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    const auto *stereo_payload = std::get_if<domain::model::StereoImagePacket>(&image.payload);
    if (image.format != domain::model::ImageFormat::STEREO || stereo_payload == nullptr)
    {
        LOG_ERR("Invalid image payload for Stereo VSLAM Adapter.");
        return domain::model::Pose{};
    }

    const auto &left_frame = stereo_payload->left;
    const auto &right_frame = stereo_payload->right;

    const auto rows = left_frame.height;
    const auto cols = left_frame.width;
    const auto channels = left_frame.channels;
    auto type = channels == 3 ? CV_8UC3 : CV_8UC1; // NOLINT: OPENCV

    cv::Mat left_img(rows, cols, type, const_cast<uint8_t *>(left_frame.data.data()));   // NOLINT: OPENCV
    cv::Mat right_img(rows, cols, type, const_cast<uint8_t *>(right_frame.data.data())); // NOLINT: OPENCV

    if (left_img.empty() || right_img.empty())
    {
        LOG_ERR("Failed to decode stereo frames.");
        return domain::model::Pose{};
    }

    constexpr auto kMicroSecondsInSecond = 1000000;
    double time_in_seconds = static_cast<double>(image.timestamp) / kMicroSecondsInSecond;

    LOG_DBG("Frame size: {}x{}, Timestamp: {}, | color: {}, Time (s): {:.6f}", cols, rows, image.timestamp, channels == 3 ? "true" : "false", time_in_seconds);

    auto raw_pose = slam_system_->feed_stereo_frame(left_img, right_img, time_in_seconds);

    domain::model::Pose pose;
    if (raw_pose)
    {
        // 3. Stella의 Eigen Matrix -> 우리 도메인 모델(Pose)로 매핑 --- IGNORE ---
        const Eigen::Matrix4d mat = raw_pose->cast<double>();
        const Eigen::Matrix3d rot = mat.block<3, 3>(0, 0);
        const Eigen::Vector3d trans = mat.block<3, 1>(0, 3);
        const Eigen::Quaterniond q(rot);

        pose.x = trans.x();
        pose.y = trans.y();
        pose.z = trans.z();
        pose.qx = q.x();
        pose.qy = q.y();
        pose.qz = q.z();
        pose.qw = q.w();
        pose.is_lost = false;

        LOG_DBG("Pose: Position({:.3f}, {:.3f}, {:.3f}), Orientation({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                pose.x, pose.y, pose.z,
                pose.qx, pose.qy, pose.qz, pose.qw);
    }
    else
    {
        LOG_WRN("Pose could not be estimated for the current frame.");
        pose.is_lost = true;
    }

    return pose;
}

bool StereoVSlamAdapterImpl::stop()
{
    is_initialized_ = false;
    return true;
}

} // namespace vp::adapter::out