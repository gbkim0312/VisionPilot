#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

namespace vp::config
{

enum class SourceType
{
    VIDEO_FILE,
    CAMERA_DEVICE,
    RTSP_STREAM
};

struct ImageSize
{
    uint32_t width = 0;  // 0: 자동 조정
    uint32_t height = 0; // 0: 자동 조정
};

struct VideoLoaderConfig
{
    ImageSize frameSize;   // 프레임 크기
    std::string source;    // 비디오 소스 경로 또는 장치 ID
    SourceType sourceType; // 비디오 소스 유형
    uint32_t fps = 30;     // 프레임 속도 (지원하는 경우)
};
} // namespace vp::config