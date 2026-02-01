#pragma once
#include "localization_port.hpp"
#include "vslam_config.hpp"
#include <memory>

namespace vp::adapter::out
{
class StereoVSlamAdapterImpl;

class StereoVSlamAdapter : public vp::port::out::LocalizationPort
{
public:
    StereoVSlamAdapter(const config::VslamAdapterConfig &vslam_config);
    ~StereoVSlamAdapter() override;

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;
    bool deinitialize();

private:
    std::unique_ptr<StereoVSlamAdapterImpl> impl_;
};
} // namespace vp::adapter::out