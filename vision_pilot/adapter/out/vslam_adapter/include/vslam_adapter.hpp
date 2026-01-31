#pragma once
#include "localization_port.hpp"
#include "vslam_config.hpp"
#include <memory>

namespace vp::adapter::out
{
class VslamAdapterImpl;

class VslamAdapter : public vp::port::out::LocalizationPort
{
public:
    VslamAdapter(const config::VslamAdapterConfig &vslam_config);
    ~VslamAdapter() override;

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;

private:
    std::unique_ptr<VslamAdapterImpl> impl_;
};
} // namespace vp::adapter::out