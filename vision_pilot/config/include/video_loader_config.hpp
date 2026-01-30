#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

namespace vp::config
{

enum class SourceType
{
    VIDEO_FILE,
    FRAME_SET,
    CAMERA_DEVICE,
    RTSP_STREAM
};

NLOHMANN_JSON_SERIALIZE_ENUM(SourceType,
                             {
                                 {SourceType::VIDEO_FILE, "videoFile"},
                                 {SourceType::FRAME_SET, "frameSet"},
                                 {SourceType::CAMERA_DEVICE, "cameraDevice"},
                                 {SourceType::RTSP_STREAM, "rtspStream"},
                             })

struct ImageSize
{
    uint32_t width = 0;  // 0: 자동 조정
    uint32_t height = 0; // 0: 자동 조정
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImageSize,
                                   width,
                                   height)

struct VideoLoaderConfig
{
    ImageSize frameSize;   // 프레임 크기
    std::string source;    // 비디오 소스 경로 또는 장치 ID
    SourceType sourceType; // 비디오 소스 유형
    uint32_t fps = 30;     // 프레임 속도 (지원하는 경우)
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoLoaderConfig,
                                   frameSize,
                                   source,
                                   sourceType,
                                   fps)
} // namespace vp::config