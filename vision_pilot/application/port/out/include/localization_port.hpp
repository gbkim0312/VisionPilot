#pragma once
#include "image.hpp"
#include "pose.hpp"
#include <vector>

namespace vp::port::out
{
class LocalizationPort
{
public:
    virtual ~LocalizationPort() = default;

    virtual domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp) = 0;
};
} // namespace vp::port::out