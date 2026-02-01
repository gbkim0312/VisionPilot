#pragma once
#include "video_loader.hpp"
#include <opencv2/videoio.hpp>

namespace vp::adapter::in
{
class StereoVideoLoader : public VideoLoader
{
public:
    using VideoLoader::VideoLoader;

protected:
    bool initialize() override;
    bool fetchFrame() override;
    void release() override;

private:
    cv::VideoCapture left_video_capture_;
    cv::VideoCapture right_video_capture_;
};
} // namespace vp::adapter::in
