#pragma once

#include "localization_port.hpp"
#include "pose.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
class OrbSlamAdapterImpl;

class OrbSlamAdapter : public vp::port::out::LocalizationPort
{
public:
    OrbSlamAdapter(const config::VslamAdapterConfig &config);
    ~OrbSlamAdapter() override;

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;
    bool deinitialize();
};
} // namespace vp::adapter::out
