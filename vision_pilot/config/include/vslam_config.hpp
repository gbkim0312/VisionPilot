#pragma once
#include "nlohmann/json.hpp"

namespace vp::config
{

enum class VslamMethod
{
    MONOCULAR = 0,
    STEREO = 1,
    RGB_D = 2,
    DISABLED = -1
};

NLOHMANN_JSON_SERIALIZE_ENUM(VslamMethod,
                             {
                                 {VslamMethod::MONOCULAR, "monocular"},
                                 {VslamMethod::STEREO, "stereo"},
                                 {VslamMethod::RGB_D, "rgbd"},
                                 {VslamMethod::DISABLED, "disabled"},
                             })

struct VslamAdapterConfig
{
    VslamMethod method = VslamMethod::MONOCULAR;
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
    OPENCV,
    SOCKET
};

NLOHMANN_JSON_SERIALIZE_ENUM(VslamViewerType,
                             {
                                 {VslamViewerType::NONE, "none"},
                                 {VslamViewerType::PANGOLIN, "pangolin"},
                                 {VslamViewerType::OPENCV, "opencv"},
                                 {VslamViewerType::SOCKET, "socket"},
                             })

struct VslamViewerConfig
{
    VslamViewerType viewerType = VslamViewerType::OPENCV;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VslamViewerConfig,
                                   viewerType)
// 설정 항목 추가 예정

} // namespace vp::config