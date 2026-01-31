#pragma once
#include <cstdint>
#include <variant>
#include <vector>

namespace vp::domain::model
{

// 공통 이미지 데이터 구조
struct RawImage
{
    int width = 0;
    int height = 0;
    int channels = 0;
    int step = 0;
    std::vector<uint8_t> data;
};

struct MonoImagePacket
{
    RawImage frame;
};

struct StereoImagePacket
{
    RawImage left;
    RawImage right;
};

enum class ImageEncoding
{
    UNKNOWN = 0,
    MONO8,
    RGB8,
    BGR8
};

enum class ImageFormat
{
    UNKNOWN = 0,
    MONO,
    STEREO
};

struct ImagePacket
{
    ImageFormat format = ImageFormat::MONO;
    ImageEncoding encoding = ImageEncoding::BGR8;
    uint64_t frame_id = 0;
    uint64_t timestamp = 0;

    // variant를 통해 타입 안전성 확보
    std::variant<MonoImagePacket, StereoImagePacket> payload;
};

} // namespace vp::domain::model