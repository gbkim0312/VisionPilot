#pragma once

#include "socket_viewer_adapter.hpp"

namespace vp::adapter::out
{
class SocketViewerAdapterImpl
{
public:
    SocketViewerAdapterImpl(const config::VslamViewerConfig &config);
    ~SocketViewerAdapterImpl();

    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, std::vector<domain::model::Detection> detections, const domain::model::ImagePacket &frame);

private:
    const config::VslamViewerConfig &config_;
    int socket_fd_ = -1;
    bool is_running_ = false;
};
} // namespace vp::adapter::out