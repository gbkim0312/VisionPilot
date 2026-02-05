#pragma once
#include "nlohmann/json.hpp"
#include <optional>

namespace vp::config
{

enum class VslamMethod
{
    MONOCULAR = 0,
    STEREO = 1,
    RGB_D = 2,
    IMU_MONOCULAR = 3,
    IMU_STEREO = 4,
    IMU_RGB_D = 5
};

NLOHMANN_JSON_SERIALIZE_ENUM(VslamMethod,
                             {
                                 {VslamMethod::MONOCULAR, "monocular"},
                                 {VslamMethod::STEREO, "stereo"},
                                 {VslamMethod::RGB_D, "rgbd"},
                                 {VslamMethod::IMU_MONOCULAR, "imuMonocular"},
                                 {VslamMethod::IMU_STEREO, "imuStereo"},
                                 {VslamMethod::IMU_RGB_D, "imuRgbd"},
                             })

enum class SaveType
{
    NONE = 0,
    FULL_TRAJECTORY = 1,
    KEYFRAME_TRAJECTORY = 2,
    MAP_DATABASE = 3
};

NLOHMANN_JSON_SERIALIZE_ENUM(SaveType,
                             {
                                 {SaveType::NONE, "none"},
                                 {SaveType::FULL_TRAJECTORY, "fullTrajectory"},
                                 {SaveType::KEYFRAME_TRAJECTORY, "keyframeTrajectory"},
                                 {SaveType::MAP_DATABASE, "mapDatabase"},
                             })

enum class SaveFormat
{
    TUM = 1,
    KITTI = 2,
};

NLOHMANN_JSON_SERIALIZE_ENUM(SaveFormat,
                             {
                                 {SaveFormat::TUM, "tum"},
                                 {SaveFormat::KITTI, "kitti"},
                             })
struct SaveConfig
{
    SaveType saveTypes;
    std::optional<SaveFormat> saveFormat;
    std::string path;
};

inline void to_json(nlohmann::json &j, const SaveConfig &p)
{
    j = nlohmann::json{
        {"saveTypes", p.saveTypes},
        {"path", p.path}};

    if (p.saveTypes == SaveType::FULL_TRAJECTORY ||
        p.saveTypes == SaveType::KEYFRAME_TRAJECTORY)
    {

        if (p.saveFormat.has_value())
        {
            j["saveFormat"] = p.saveFormat.value();
        }
        else
        {
            j["saveFormat"] = SaveFormat::TUM;
        }
    }
}

inline void from_json(const nlohmann::json &j, SaveConfig &p)
{
    j.at("saveTypes").get_to(p.saveTypes);
    j.at("path").get_to(p.path);

    if (p.saveTypes == SaveType::FULL_TRAJECTORY ||
        p.saveTypes == SaveType::KEYFRAME_TRAJECTORY)
    {
        if (j.contains("saveFormat"))
        {
            p.saveFormat = j.at("saveFormat").get<SaveFormat>();
        }
        else
        {
            p.saveFormat = SaveFormat::TUM; // 기본값
        }
    }
    else
    {
        p.saveFormat = std::nullopt;
    }
}

struct LoadConfig
{
    bool loadMapDatabase = false;
    std::string path;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LoadConfig,
                                   loadMapDatabase,
                                   path)

enum class VslamType
{
    ORB_SLAM3 = 0,
    STELLA_VSLAM = 1,
    NONE
};

NLOHMANN_JSON_SERIALIZE_ENUM(VslamType,
                             {
                                 {VslamType::ORB_SLAM3, "orbSlam3"},
                                 {VslamType::STELLA_VSLAM, "stellaVslam"},
                                 {VslamType::NONE, "none"},
                             })

struct VslamAdapterConfig
{
    VslamType type = VslamType::STELLA_VSLAM;
    VslamMethod method = VslamMethod::MONOCULAR;
    std::string vslamConfigFilePath; // VSLAM용 설정 파일 경로
    std::string vocabPath;           // VSLAM용 보캐뷸러리 파일 경로
    bool useInternalViewer = false;
    uint32_t frameQueueSize = 2;
    LoadConfig loadConfig;
    std::vector<SaveConfig> saveConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VslamAdapterConfig,
                                   type,
                                   vslamConfigFilePath,
                                   method,
                                   vocabPath,
                                   frameQueueSize,
                                   loadConfig,
                                   saveConfig,
                                   useInternalViewer)
} // namespace vp::config