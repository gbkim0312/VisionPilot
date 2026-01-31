#pragma once
#include "nlohmann/json.hpp"

namespace vp::config
{

enum class VslamMethod
{
    ORB_SLAM3 = 0,
    STELLA_VSLAM
};

NLOHMANN_JSON_SERIALIZE_ENUM(VslamMethod,
                             {
                                 {VslamMethod::ORB_SLAM3, "orbSlam3"},
                                 {VslamMethod::STELLA_VSLAM, "stellaVslam"},
                             })

struct VslamAdapterConfig
{
    VslamMethod method = VslamMethod::STELLA_VSLAM;
    std::string vslamConfigFilePath; // VSLAM용 설정 파일 경로
    std::string vocabPath;           // VSLAM용 보캐뷸러리 파일 경로
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VslamAdapterConfig,
                                   vslamConfigFilePath,
                                   method,
                                   vocabPath)

enum class VslamViewerType
{
    NONE = 0,
    PANGOLIN,
    OPENCV
};

NLOHMANN_JSON_SERIALIZE_ENUM(VslamViewerType,
                             {
                                 {VslamViewerType::NONE, "none"},
                                 {VslamViewerType::PANGOLIN, "pangolin"},
                                 {VslamViewerType::OPENCV, "opencv"},
                             })

struct VslamViewerConfig
{
    VslamViewerType viewerType = VslamViewerType::NONE;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VslamViewerConfig,
                                   viewerType)
// 설정 항목 추가 예정

} // namespace vp::config