#include "stereo_vslam_adapter_impl.hpp"
#include "gaia_log.hpp"

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

    // TODO(GBKIM): Stereo 이미지 처리 로직 구현 필요

    domain::model::Pose pose;

    return pose;
}

bool StereoVSlamAdapterImpl::stop()
{
    is_initialized_ = false;
    return true;
}

} // namespace vp::adapter::out