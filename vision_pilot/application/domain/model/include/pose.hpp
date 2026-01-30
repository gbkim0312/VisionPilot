#pragma once
#include <cstdint>

namespace vp::domain::model
{
struct Pose
{
    double x = 0.0; // 위치 X (미터)
    double y = 0.0; // 위치 Y (미터)
    double z = 0.0; // 위치 Z (미터)
    double qw = 1.0;
    double qx = 0.0;
    double qy = 0.0;
    double qz = 0.0;

    uint64_t timestamp = 0; // 타임스탬프 (ms)
    bool is_lost = true;

    Pose() = default;

    Pose(double x_, double y_, double z_, double qw_, double qx_, double qy_, double qz_)
        : x(x_), y(y_), z(z_), qw(qw_), qx(qx_), qy(qy_), qz(qz_) {}
};
} // namespace vp::domain::model