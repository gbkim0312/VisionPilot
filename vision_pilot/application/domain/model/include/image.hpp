#pragma once
#include <cstdint>
#include <vector>
namespace vp::domain::model
{
struct ImagePacket
{
    int width = 0;             // 프레임 폭
    int height = 0;            // 프레임 높이
    int channels = 0;          // 채널 수: 1=Gray, 3=RGB 등
    std::vector<uint8_t> data; // row-major order, size = width*height*channels

    uint64_t timestamp; // 프레임 수신 시점

    ImagePacket() = default;

    ImagePacket(int w, int h, int c)
        : width(w), height(h), channels(c), data(w * h * c) {}
};
} // namespace vp::domain::model