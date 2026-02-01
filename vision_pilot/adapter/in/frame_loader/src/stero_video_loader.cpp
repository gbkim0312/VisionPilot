#include "gaia_log.hpp"
#include "gaia_time.hpp"
#include "stereo_video_loader.hpp"
#include "video_loader_config.hpp"
#include <opencv2/core/mat.hpp>

namespace vp::adapter::in
{
bool StereoVideoLoader::initialize()
{
    LOG_TRA("");

    auto *stereo_param = std::get_if<config::StereoParam>(&config_.cameraParam);
    if (stereo_param == nullptr)
    {
        LOG_ERR("StereoParam is not set in cameraParam.");
        return false;
    }

    LOG_INF("Opening stereo video sources: left={}, right={}", stereo_param->leftSource, stereo_param->rightSource);
    left_video_capture_.open(stereo_param->leftSource);
    if (!left_video_capture_.isOpened())
    {
        LOG_ERR("Failed to open left video source: {}", stereo_param->leftSource);
        return false;
    }

    right_video_capture_.open(stereo_param->rightSource);
    if (!right_video_capture_.isOpened())
    {
        LOG_ERR("Failed to open right video source: {}", stereo_param->rightSource);
        return false;
    }

    return true;
}

void StereoVideoLoader::release()
{
    LOG_TRA("");

    if (left_video_capture_.isOpened())
    {
        left_video_capture_.release();
    }

    if (right_video_capture_.isOpened())
    {
        right_video_capture_.release();
    }
}

bool StereoVideoLoader::fetchFrame()
{
    LOG_TRA("");

    cv::Mat left_iamge;
    cv::Mat right_image;

    if (!left_video_capture_.read(left_iamge))
    {
        LOG_DBG("Failed to read frame from left video source.");
        return false;
    }

    if (!right_video_capture_.read(right_image))
    {
        LOG_DBG("Failed to read frame from right video source.");
        return false;
    }

    auto stereo_packet = std::make_shared<domain::model::StereoImagePacket>();

    auto &left_frame = stereo_packet->left;
    left_frame.channels = left_iamge.channels();
    left_frame.data.assign(left_iamge.data, left_iamge.data + (left_iamge.cols * left_iamge.rows * left_iamge.channels()));
    left_frame.height = left_iamge.rows;
    left_frame.width = left_iamge.cols;
    left_frame.step = static_cast<int>(left_iamge.step);

    auto &right_frame = stereo_packet->right;
    right_frame.channels = right_image.channels();
    right_frame.data.assign(right_image.data, right_image.data + (right_image.cols * right_image.rows * right_image.channels()));
    right_frame.height = right_image.rows;
    right_frame.width = right_image.cols;
    right_frame.step = static_cast<int>(right_image.step);

    auto image_packet = std::make_shared<domain::model::ImagePacket>(
        domain::model::ImagePacket{
            .format = domain::model::ImageFormat::STEREO,
            .encoding = (left_iamge.channels() == 3) ? domain::model::ImageEncoding::BGR8 : domain::model::ImageEncoding::MONO8,
            .frame_id = frame_id_++,
            .timestamp = getTime32(),
            .payload = *stereo_packet,
        });

    return true;
}
} // namespace vp::adapter::in