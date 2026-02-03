#include "orb_slam3_adapter_impl.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"

namespace vp::adapter::out
{
OrbSlamAdapterImpl::OrbSlamAdapterImpl(const config::VslamAdapterConfig &config)
    : config_(config)
{
    LOG_TRA("");
}

OrbSlamAdapterImpl::~OrbSlamAdapterImpl()
{
    LOG_TRA("");
}

bool OrbSlamAdapterImpl::initialize()
{
    LOG_TRA("");

    if (is_initialized_)
    {
        LOG_INF("ORB SLAM3 Adapter is already initialized.");
        return true;
    }

    try
    {
        LOG_INF("Creating ORB SLAM3 system...");

        // Determine sensor type
        ORB_SLAM3::System::eSensor sensor_type = ORB_SLAM3::System::MONOCULAR;
        switch (config_.method)
        {
        case config::VslamMethod::MONOCULAR:
            LOG_INF("VSLAM method: MONOCULAR");
            sensor_type = ORB_SLAM3::System::MONOCULAR;
            break;
        case config::VslamMethod::STEREO:
            LOG_INF("VSLAM method: STEREO");
            sensor_type = ORB_SLAM3::System::STEREO;
            break;
        case config::VslamMethod::RGB_D:
            LOG_INF("VSLAM method: RGB-D");
            sensor_type = ORB_SLAM3::System::RGBD;
            break;
        case config::VslamMethod::IMU_MONOCULAR:
            LOG_INF("VSLAM method: IMU_MONOCULAR");
            sensor_type = ORB_SLAM3::System::IMU_MONOCULAR;
            break;
        case config::VslamMethod::IMU_STEREO:
            LOG_INF("VSLAM method: IMU_STEREO");
            sensor_type = ORB_SLAM3::System::IMU_STEREO;
            break;
        case config::VslamMethod::IMU_RGB_D:
            LOG_INF("VSLAM method: IMU_RGB_D");
            sensor_type = ORB_SLAM3::System::IMU_RGBD;
            break;
        default:
            LOG_ERR("Unsupported VSLAM method for ORB SLAM3: {}", static_cast<int>(config_.method));
            return false;
        }

        orb_slam_system_ = std::make_unique<ORB_SLAM3::System>(
            config_.vocabPath,
            config_.vslamConfigFilePath,
            sensor_type,
            false // enable viewer
        );

        LOG_INF("ORB SLAM3 system created successfully.");
    }
    catch (const std::exception &e)
    {
        LOG_ERR("Exception caught during ORB SLAM3 initialization: {}", e.what());
        return false;
    }

    is_initialized_ = true;
    return true;
}

domain::model::Pose OrbSlamAdapterImpl::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    if (!is_initialized_)
    {
        LOG_WRN("ORB SLAM3 Adapter is not initialized. Call initialize() before update().");
        return domain::model::Pose{};
    }

    switch (config_.method)
    {
    case config::VslamMethod::MONOCULAR:
        return this->feedMonoFrame(image, timestamp);
    case config::VslamMethod::STEREO:
        return this->feedStereoFrame(image, timestamp);
    case config::VslamMethod::RGB_D:
        return this->feedRgbdFrame(image, timestamp);
    default:
        LOG_ERR("Unsupported VSLAM method in update(): {}", static_cast<int>(config_.method));
        return domain::model::Pose{};
    }
}

bool OrbSlamAdapterImpl::deinitialize()
{
    LOG_INF("Deinitializing VSLAM Adapter...");

    if (!is_initialized_)
    {
        LOG_INF("VSLAM Adapter is not initialized. Nothing to deinitialize.");
        return true;
    }

    try
    {
        orb_slam_system_->Shutdown();
        orb_slam_system_->Reset();
        is_initialized_ = false;
        LOG_INF("ORB SLAM Adapter deinitialized successfully.");
    }
    catch (const std::exception &e)
    {
        LOG_ERR("ORB SLAM deinitialization failed: {}", e.what());
        return false;
    }

    return true;
}

domain::model::Pose OrbSlamAdapterImpl::feedMonoFrame(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    LOG_TRA("");

    const auto *mono_payload = std::get_if<domain::model::MonoImagePacket>(&image.payload);
    if (mono_payload == nullptr)
    {
        THROWLOG(SysException, "Invalid image payload for Monocular ORB SLAM3 Adapter. Check VideoLoader configuration.");
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

    auto raw_pose = orb_slam_system_->TrackMonocular(img, time_in_seconds);

    return this->convertOrbSlamPoseToDomainPose(raw_pose);
}

domain::model::Pose OrbSlamAdapterImpl::feedStereoFrame(const domain::model::ImagePacket & /* image */, uint64_t /* timestamp */)
{
    LOG_TRA("");
    return domain::model::Pose{};
}

domain::model::Pose OrbSlamAdapterImpl::feedRgbdFrame(const domain::model::ImagePacket & /* image */, uint64_t /* timestamp */)
{
    LOG_TRA("");
    return domain::model::Pose{};
}

domain::model::Pose OrbSlamAdapterImpl::convertOrbSlamPoseToDomainPose(const Sophus::SE3f &orb_pose)
{
    LOG_TRA("");

    domain::model::Pose pose;
    if (orb_pose.matrix().allFinite())
    {
        const Eigen::Matrix4f mat = orb_pose.matrix();
        const Eigen::Matrix3f rot = mat.block<3, 3>(0, 0);
        const Eigen::Vector3f trans = mat.block<3, 1>(0, 3);
        const Eigen::Quaternionf q(rot);

        pose.x = static_cast<double>(trans(0));
        pose.y = static_cast<double>(trans(1));
        pose.z = static_cast<double>(trans(2));
        pose.qx = static_cast<double>(q.x());
        pose.qy = static_cast<double>(q.y());
        pose.qz = static_cast<double>(q.z());
        pose.qw = static_cast<double>(q.w());

        LOG_DBG("Converted ORB SLAM3 pose: Position ({:.4f}, {:.4f}, {:.4f}), Orientation ({:.4f}, {:.4f}, {:.4f}, {:.4f})",
                pose.x, pose.y, pose.z,
                pose.qx, pose.qy, pose.qz, pose.qw);
    }
    else
    {
        pose.is_lost = true;
        LOG_DBG("ORB SLAM3 pose is not finite. Marking as lost.");
    }
    return pose;
}

} // namespace vp::adapter::out