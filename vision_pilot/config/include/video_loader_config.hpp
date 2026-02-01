#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

namespace vp::config
{

// 공통 Enum (소스 유형, 카메라 포맷)
enum class DataType
{
    VIDEO_FILE,
    FRAME_SET,
    CAMERA_DEVICE,
    RTSP_STREAM
};
NLOHMANN_JSON_SERIALIZE_ENUM(DataType, {
                                           {DataType::VIDEO_FILE, "videoFile"},
                                           {DataType::FRAME_SET, "frameSet"},
                                           {DataType::CAMERA_DEVICE, "cameraDevice"},
                                           {DataType::RTSP_STREAM, "rtspStream"},
                                       })

inline std::string toString(DataType type)
{
    switch (type)
    {
    case DataType::VIDEO_FILE:
        return "videoFile";
    case DataType::FRAME_SET:
        return "frameSet";
    case DataType::CAMERA_DEVICE:
        return "cameraDevice";
    case DataType::RTSP_STREAM:
        return "rtspStream";
    default:
        return "unknown";
    }
}

enum class CameraFormat
{
    MONO,
    STEREO,
    RGB_D
};
NLOHMANN_JSON_SERIALIZE_ENUM(CameraFormat, {
                                               {CameraFormat::MONO, "mono"},
                                               {CameraFormat::STEREO, "stereo"},
                                               {CameraFormat::RGB_D, "rgbd"},
                                           })

inline std::string toString(CameraFormat format)
{
    switch (format)
    {
    case CameraFormat::MONO:
        return "mono";
    case CameraFormat::STEREO:
        return "stereo";
    case CameraFormat::RGB_D:
        return "rgbd";
    default:
        return "unknown";
    }
}

enum class ColorFormat
{
    COLOR,
    GRAYSCALE
};
NLOHMANN_JSON_SERIALIZE_ENUM(ColorFormat, {
                                              {ColorFormat::COLOR, "color"},
                                              {ColorFormat::GRAYSCALE, "grayscale"},
                                          })

// 포맷별 파라미터 구조체 (소스 경로 포함)
struct MonoParam
{
    uint64_t maxImageBufferSize = 100; // 최대 이미지 버퍼 크기 (프레임 세트용)
    ColorFormat colorFormat = ColorFormat::COLOR;
    std::string source; // 단일 소스
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MonoParam, maxImageBufferSize, colorFormat, source)
struct StereoParam
{
    uint64_t maxImageBufferSize = 50; // 최대 이미지 버퍼 크기 (프레임 세트용)
    ColorFormat colorFormat = ColorFormat::COLOR;
    std::string leftSource;  // 왼쪽 소스
    std::string rightSource; // 오른쪽 소스
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StereoParam, maxImageBufferSize, colorFormat, leftSource, rightSource)
struct RgbdParam
{
    std::string rgbSource;   // 컬러 소스
    std::string depthSource; // Depth 소스
    float depthScale = 1000.0f;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RgbdParam, rgbSource, depthSource, depthScale)

// Variant 정의
using CameraParam = std::variant<MonoParam, StereoParam, RgbdParam>;

// 공통 이미지 사이즈
struct ImageSize
{
    uint32_t width = 0;
    uint32_t height = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImageSize, width, height)

// VideoLoaderConfig
struct VideoLoaderConfig
{
    // 공통 설정
    ImageSize frameSize;
    DataType dataType = DataType::VIDEO_FILE;
    uint32_t fps = 30;

    // 카메라 포맷 및 가변 파라미터
    CameraFormat cameraFormat = CameraFormat::MONO;
    CameraParam cameraParam = MonoParam{};
};

// 6. 커스텀 JSON 직렬화 (핵심 로직)
inline void to_json(nlohmann::json &j, const VideoLoaderConfig &p)
{
    j = nlohmann::json{
        {"frameSize", p.frameSize},
        {"dataType", p.dataType},
        {"fps", p.fps},
        {"cameraFormat", p.cameraFormat}};

    if (std::holds_alternative<MonoParam>(p.cameraParam))
    {
        j["mono"] = std::get<MonoParam>(p.cameraParam);
    }
    else if (std::holds_alternative<StereoParam>(p.cameraParam))
    {
        j["stereo"] = std::get<StereoParam>(p.cameraParam);
    }
    else if (std::holds_alternative<RgbdParam>(p.cameraParam))
    {
        j["rgbd"] = std::get<RgbdParam>(p.cameraParam);
    }
}

inline void from_json(const nlohmann::json &j, VideoLoaderConfig &p)
{
    j.at("frameSize").get_to(p.frameSize);
    j.at("dataType").get_to(p.dataType);
    p.fps = j.value("fps", 30);

    p.cameraFormat = j.value("cameraFormat", CameraFormat::MONO);

    if (p.cameraFormat == CameraFormat::STEREO)
    {
        p.cameraParam = j.at("stereo").get<StereoParam>();
    }
    else if (p.cameraFormat == CameraFormat::RGB_D)
    {
        p.cameraParam = j.at("rgbd").get<RgbdParam>();
    }
    else
    {
        if (j.contains("mono"))
        {
            p.cameraParam = j.at("mono").get<MonoParam>();
        }
        else
        {
            p.cameraParam = MonoParam{};
        }
    }
}

} // namespace vp::config