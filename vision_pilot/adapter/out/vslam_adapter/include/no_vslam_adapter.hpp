#pragma once

#include "localization_port.hpp"

namespace vp::adapter::out
{
class NoVslamAdapter : public port::out::LocalizationPort
{
public:
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) override;
};
} // namespace vp::adapter::out