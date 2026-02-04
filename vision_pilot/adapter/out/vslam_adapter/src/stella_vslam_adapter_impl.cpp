#include "stella_vslam_adapter_impl.hpp"

#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "pose.hpp"
#include "stella_vslam/config.h"
#include <exception>
#include <opencv2/imgcodecs.hpp>

#include "stella_vslam/publish/frame_publisher.h"
#include "stella_vslam/publish/map_publisher.h"

namespace
{
std::string convertSaveFormatToString(std::optional<vp::config::SaveFormat> format)
{
    if (!format.has_value())
    {
        LOG_INF("Save format not specified. Using default KITTI format.");
        return "KITTI"; // default format
    }

    switch (format.value())
    {
    case vp::config::SaveFormat::TUM:
        return "TUM";
    case vp::config::SaveFormat::KITTI:
        return "KITTI";
    default:
        return "unknown";
    }
}
} // namespace

namespace vp::adapter::out
{
StellaVslamAdapterImpl::StellaVslamAdapterImpl(const config::VslamAdapterConfig &vslam_config)
    : vslam_config_{vslam_config}
{
    LOG_TRA("");
}

StellaVslamAdapterImpl::~StellaVslamAdapterImpl()
{
    LOG_TRA("");
    if (slam_system_)
    {
        slam_system_->shutdown();
    }
}

bool StellaVslamAdapterImpl::initialize()
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

        LOG_INF("Setting up Pangolin Viewer...");
        auto frame_publisher = slam_system_->get_frame_publisher();
        auto map_publisher = slam_system_->get_map_publisher();
        viewer_ = std::make_shared<pangolin_viewer::viewer>(config, slam_system_.get(), frame_publisher, map_publisher);

        std::thread([this]()
                    { viewer_->run(); })
            .detach();

        LOG_INF("Pangolin Viewer started.");

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

domain::model::Pose StellaVslamAdapterImpl::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    if (!is_initialized_)
    {
        LOG_INF("Stella VSLAM Adapter is not yet initialized.");
        return domain::model::Pose{};
    }

    switch (vslam_config_.method)
    {
    case config::VslamMethod::MONOCULAR:
        return this->feedMonoFrame(image, timestamp);
    case config::VslamMethod::STEREO:
        return this->feedStereoFrame(image, timestamp);
    case config::VslamMethod::RGB_D:
        return this->feedRgbdFrame(image, timestamp);
    default:
        LOG_ERR("Unsupported VSLAM method in update(): {}", static_cast<int>(vslam_config_.method));
        return domain::model::Pose{};
    }
}

domain::model::Pose StellaVslamAdapterImpl::feedMonoFrame(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    LOG_TRA("");

    const auto *mono_payload = std::get_if<domain::model::MonoImagePacket>(&image.payload);
    if (mono_payload == nullptr)
    {
        THROWLOG(SysException, "Invalid image payload for Mono VSLAM Adapter. Check VideoLoader configuration.");
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
    return this->convertStellaPoseToDomainPose(raw_pose);
}

bool StellaVslamAdapterImpl::deinitialize()
{
    LOG_TRA("Stopping VSLAM Adapter...");

    if (!is_initialized_)
    {
        LOG_WRN("VSLAM Adapter is not initialized.");
        return true;
    }

    for (const auto &save_config : vslam_config_.saveConfig)
    {
        switch (save_config.saveTypes)
        {
        case config::SaveType::MAP_DATABASE:
            LOG_INF("Saving VSLAM map to: {}", save_config.path);
            slam_system_->save_map_database(save_config.path);
            LOG_INF("VSLAM map saved successfully.");
            break;
        case config::SaveType::FULL_TRAJECTORY:
            LOG_INF("Saving VSLAM trajectory to: {}", save_config.path);
            slam_system_->save_frame_trajectory(save_config.path, ::convertSaveFormatToString(save_config.saveFormat));
            LOG_INF("VSLAM trajectory saved successfully.");
            break;
        case config::SaveType::KEYFRAME_TRAJECTORY:
            LOG_INF("Saving VSLAM keyframe trajectory to: {}", save_config.path);
            slam_system_->save_keyframe_trajectory(save_config.path, ::convertSaveFormatToString(save_config.saveFormat));
            LOG_INF("VSLAM keyframe trajectory saved successfully.");
            break;
        default:
            LOG_WRN("Unknown SaveType encountered during VSLAM shutdown.");
            break;
        }
    }

    if (viewer_ != nullptr)
    {
        viewer_->request_terminate();
        viewer_.reset();
        LOG_INF("Pangolin Viewer terminated.");
    }

    if (slam_system_ != nullptr)
    {
        LOG_INF("Shutting down VSLAM system...");
        slam_system_->shutdown();
        slam_system_.reset();

        LOG_INF("VSLAM system shut down and reset successfully.");
    }
    else
    {
        LOG_WRN("VSLAM system was not initialized or already shut down.");
    }

    is_initialized_ = false;
    return true;
}

domain::model::Pose StellaVslamAdapterImpl::feedStereoFrame(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    LOG_TRA("");

    const auto *stereo_payload = std::get_if<domain::model::StereoImagePacket>(&image.payload);
    if (stereo_payload == nullptr)
    {
        THROWLOG(SysException, "Invalid image payload for Stereo VSLAM Adapter. Check VideoLoader configuration.");
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
    double time_in_seconds = static_cast<double>(timestamp) / kMicroSecondsInSecond;

    LOG_DBG("Frame size: {}x{}, Timestamp: {}, | color: {}, Time (s): {:.6f}", cols, rows, timestamp, channels == 3 ? "true" : "false", time_in_seconds);

    auto raw_pose = slam_system_->feed_stereo_frame(left_img, right_img, time_in_seconds);
    return this->convertStellaPoseToDomainPose(raw_pose);
}

domain::model::Pose StellaVslamAdapterImpl::feedRgbdFrame(const domain::model::ImagePacket & /* image */, uint64_t /* timestamp */)
{
    LOG_TRA("");
    LOG_WRN("RGB-D method is not yet implemented in StellaVSLAM Adapter.");
    return domain::model::Pose{};
}

domain::model::Pose StellaVslamAdapterImpl::convertStellaPoseToDomainPose(const std::shared_ptr<stella_vslam::Mat44_t> &raw_pose)
{
    domain::model::Pose pose;

    if (raw_pose != nullptr)
    {
        const Eigen::Matrix4d mat = raw_pose->cast<double>();
        const Eigen::Matrix3d rot = mat.block<3, 3>(0, 0);
        const Eigen::Vector3d trans = mat.block<3, 1>(0, 3);
        const Eigen::Quaterniond q(rot);

        pose.x = trans.x();
        pose.y = trans.y();
        pose.z = trans.z();
        pose.qw = q.w(); // NOLINT: eigen-alignment
        pose.qx = q.x(); // NOLINT: eigen-alignment
        pose.qy = q.y(); // NOLINT: eigen-alignment
        pose.qz = q.z(); // NOLINT: eigen-alignment
        pose.is_lost = false;

        LOG_DBG("Pose: Position({:.3f}, {:.3f}, {:.3f}), Orientation({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                pose.x, pose.y, pose.z,
                pose.qx, pose.qy, pose.qz, pose.qw);
    }
    else
    {
        pose.is_lost = true;
        LOG_DBG("VSLAM tracking lost.");
    }

    return pose;
}
} // namespace vp::adapter::out