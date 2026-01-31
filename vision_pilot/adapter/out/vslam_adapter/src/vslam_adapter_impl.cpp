#include "vslam_adapter_impl.hpp"
#include "gaia_log.hpp"
#include "stella_vslam/config.h"
#include <exception>

namespace vp::adapter::out
{
VslamAdapterImpl::VslamAdapterImpl(const config::VslamAdapterConfig &vslam_config)
    : vslam_config_{vslam_config}
{
    LOG_TRA("");
}

VslamAdapterImpl::~VslamAdapterImpl()
{
    LOG_TRA("");
    if (slam_system_)
    {
        slam_system_->shutdown();
    }
}

bool VslamAdapterImpl::initialize()
{
    LOG_INF("Initializing VSLAM Adapter...");

    try
    {
        auto config = std::make_shared<stella_vslam::config>(vslam_config_.vslamConfigFilePath);
        slam_system_ = std::make_shared<stella_vslam::system>(config, vslam_config_.vocabPath);

        slam_system_->startup();

        is_initialized_ = true;
        LOG_INF("VSLAM initialized successfully.");
    }
    catch (const std::exception &e)
    {
        LOG_ERR("VSLAM initialization failed: {}", e.what());
        return false;
    }

    return true;
}

domain::model::Pose VslamAdapterImpl::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    LOG_TRA("");

    if (!is_initialized_ || slam_system_ == nullptr)
    {
        LOG_ERR("VSLAM system is not initialized.");
        return domain::model::Pose{};
    }

    while (slam_system_->loop_BA_is_running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    const auto rows = image.height;
    const auto cols = image.width;
    const auto channels = image.channels;
    auto type = channels == 3 ? CV_8UC3 : CV_8UC1; // NOLINT: OPENCV

    cv::Mat img(rows, cols, type, const_cast<uint8_t *>(image.data.data())); // NOLINT: OPENCV
    if (img.empty())
    {
        LOG_ERR("Failed to decode frame at ts: {}", timestamp);
        return domain::model::Pose{};
    }

    constexpr auto kMicroSecondsInSecond = 1000000;
    double time_in_seconds = static_cast<double>(timestamp) / kMicroSecondsInSecond;
    auto raw_pose = slam_system_->feed_monocular_frame(img, time_in_seconds);

    domain::model::Pose pose;
    if (raw_pose)
    {
        // 3. Stella의 Eigen Matrix -> 우리 도메인 모델(Pose)로 매핑
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

        LOG_INF("Pose Updated: x={:.2f}, y={:.2f}, z={:.2f}", pose.x, pose.y, pose.z);
    }
    else
    {
        pose.is_lost = true;
        LOG_DBG("VSLAM tracking lost at ts: {}", timestamp);
    }

    return pose;
}
} // namespace vp::adapter::out
