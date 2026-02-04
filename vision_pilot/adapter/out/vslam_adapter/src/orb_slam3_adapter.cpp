#include "orb_slam3_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"

namespace vp::adapter::out
{
OrbSlamAdapter::OrbSlamAdapter(const config::VslamAdapterConfig &config)
{
    LOG_TRA("");

    if (config.type != config::VslamType::ORB_SLAM3)
    {
        THROWLOG(SysException, "OrbSlamAdapter initialized with unsupported VSLAM type.");
    }

    LOG_WRN("OrbSlamAdapter is under development. Functionality may be limited.");
}

OrbSlamAdapter::~OrbSlamAdapter() = default;

bool OrbSlamAdapter::initialize()
{
    LOG_TRA("");

    return true;
}

domain::model::Pose OrbSlamAdapter::update(const domain::model::ImagePacket & /*image*/, uint64_t /*timestamp*/)
{
    return {};
}

bool OrbSlamAdapter::deinitialize()
{
    LOG_TRA("");

    return true;
}
} // namespace vp::adapter::out