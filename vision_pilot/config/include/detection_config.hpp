#pragma once
#include "gaia_log.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <variant>

namespace vp::config
{
struct YoloConfig
{
    std::string modelPath;
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;
    int inputWidth = 640;
    int inputHeight = 640;
    bool useCuda = false; // GPU 사용 여부
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(YoloConfig,
                                   modelPath,
                                   confThreshold,
                                   nmsThreshold,
                                   inputWidth,
                                   inputHeight,
                                   useCuda)

enum class DetectionType
{
    YOLOV8,
    NONE
};

NLOHMANN_JSON_SERIALIZE_ENUM(DetectionType,
                             {
                                 {DetectionType::YOLOV8, "yolov8"},
                                 {DetectionType::NONE, "none"},
                             })

struct DetectionConfig
{
    DetectionType type = DetectionType::YOLOV8;
    std::variant<std::monostate, YoloConfig> modelConfig;
};

inline void to_json(nlohmann::json &j, const DetectionConfig &p)
{
    j = nlohmann::json{{"type", p.type}};

    if (std::holds_alternative<YoloConfig>(p.modelConfig))
    {
        j["yolov8"] = std::get<YoloConfig>(p.modelConfig);
    }
}

inline void from_json(const nlohmann::json &j, DetectionConfig &p)
{
    j.at("type").get_to(p.type);

    if (p.type == DetectionType::YOLOV8)
    {
        if (j.contains("yolov8"))
        {
            p.modelConfig = j.at("yolov8").get<YoloConfig>();
        }
        else
        {
            LOG_WRN("YOLOv8 configuration not found in JSON. Using default values.");
            p.modelConfig = YoloConfig{};
        }
    }
    else
    {
        p.modelConfig = std::monostate{};
    }
}

} // namespace vp::config