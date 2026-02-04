#pragma once
#include <nlohmann/json.hpp>

namespace vp::config
{

enum class ViewerType
{
    NONE = 0,
    PANGOLIN,
    OPENCV,
    SOCKET
};

NLOHMANN_JSON_SERIALIZE_ENUM(ViewerType,
                             {
                                 {ViewerType::NONE, "none"},
                                 {ViewerType::PANGOLIN, "pangolin"},
                                 {ViewerType::OPENCV, "opencv"},
                                 {ViewerType::SOCKET, "socket"},
                             })

struct ViewerConfig
{
    ViewerType viewerType = ViewerType::OPENCV;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewerConfig,
                                   viewerType)
// 설정 항목 추가 예정
} // namespace vp::config