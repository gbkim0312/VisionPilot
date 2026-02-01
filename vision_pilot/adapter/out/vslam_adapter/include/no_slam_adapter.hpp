#pragma once
#include "localization_port.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{

class NoSlamAdapter : public vp::port::out::LocalizationPort
{
public:
    NoSlamAdapter(const config::VslamAdapterConfig &vslam_config);
    ~NoSlamAdapter() override;

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;
    bool deinitialize();

private:
    const config::VslamAdapterConfig &config_;
};
} // namespace vp::adapter::out