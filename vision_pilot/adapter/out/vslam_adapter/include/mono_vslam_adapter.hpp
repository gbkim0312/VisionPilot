#pragma once
#include "localization_port.hpp"
#include "vslam_config.hpp"
#include <memory>

namespace vp::adapter::out
{
class MonoVSlamAdapterImpl;

class MonoVSlamAdapter : public vp::port::out::LocalizationPort
{
public:
    MonoVSlamAdapter(const config::VslamAdapterConfig &vslam_config);
    ~MonoVSlamAdapter() override;

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;
    bool deinitialize();

private:
    std::unique_ptr<MonoVSlamAdapterImpl> impl_;
};
} // namespace vp::adapter::out