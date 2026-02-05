#pragma once
#include "localization_port.hpp"
#include "vslam_config.hpp"
#include <memory>

namespace vp::adapter::out
{
class StellaVslamAdapterImpl;

class StellaVslamAdapter : public vp::port::out::LocalizationPort
{
public:
    StellaVslamAdapter(const config::VslamAdapterConfig &vslam_config);
    ~StellaVslamAdapter() override;

    bool start();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;
    bool stop();

private:
    std::unique_ptr<StellaVslamAdapterImpl> impl_;
};
} // namespace vp::adapter::out