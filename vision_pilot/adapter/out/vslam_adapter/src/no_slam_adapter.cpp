#include "no_slam_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"

namespace vp::adapter::out
{
NoSlamAdapter::NoSlamAdapter(const config::VslamAdapterConfig &vslam_config)
    : config_(vslam_config)
{

    if (config_.method != config::VslamMethod::DISABLED)
    {
        THROWLOG(SysException, "NoSlamAdapter can be used only when VSLAM is disabled.");
    }
    LOG_TRA("");
}

NoSlamAdapter::~NoSlamAdapter()
{
    LOG_TRA("");
}

bool NoSlamAdapter::initialize()
{
    LOG_TRA("");
    return true;
}

domain::model::Pose NoSlamAdapter::update(const domain::model::ImagePacket & /* image */, uint64_t timestamp)
{
    LOG_TRA("");

    domain::model::Pose pose;
    pose.timestamp = timestamp;

    return pose;
}

bool NoSlamAdapter::deinitialize()
{
    LOG_TRA("");
    return true;
}
} // namespace vp::adapter::out