#include "no_vslam_adapter.hpp"

namespace vp::adapter::out
{
domain::model::Pose NoVslamAdapter::update(const domain::model::ImagePacket & /* image */, uint64_t /* timestamp */)
{
    return domain::model::Pose{};
}
} // namespace vp::adapter::out