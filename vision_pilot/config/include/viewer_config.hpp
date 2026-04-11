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

struct RenderOption
{
    bool renderDetection = true;
    bool renderTracking = true;
};

inline void to_json(nlohmann::json &j, const RenderOption &render_option)
{
    j = nlohmann::json{
        {"renderDetection", render_option.renderDetection},
        {"renderTracking", render_option.renderTracking}};
}

inline void from_json(const nlohmann::json &j, RenderOption &render_option)
{
    render_option.renderDetection = j.value("renderDetection", j.value("rendaerDetection", true));
    render_option.renderTracking = j.value("renderTracking", j.value("renderTrackling", true));
}

struct ViewerConfig
{
    ViewerType viewerType = ViewerType::OPENCV;
    RenderOption renderOption;
};

inline void to_json(nlohmann::json &j, const ViewerConfig &viewer_config)
{
    j = nlohmann::json{
        {"viewerType", viewer_config.viewerType},
        {"renderOption", viewer_config.renderOption}};
}

inline void from_json(const nlohmann::json &j, ViewerConfig &viewer_config)
{
    viewer_config.viewerType = j.value("viewerType", ViewerType::OPENCV);
    viewer_config.renderOption = j.value("renderOption", RenderOption{});
}

} // namespace vp::config