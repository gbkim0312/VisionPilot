// adapter/out/visualizer/src/opencv_viewer_adapter_impl.cpp
#include "opencv_viewer_adapter_impl.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace vp::adapter::out
{
OpenCVViewerAdapterImpl::OpenCVViewerAdapterImpl(const config::VslamViewerConfig &config)
    : config_{config}
{
    LOG_TRA("OpenCVViewerAdapterImpl Instance Created");
}

OpenCVViewerAdapterImpl::~OpenCVViewerAdapterImpl()
{
    this->stop();
    LOG_TRA("OpenCVViewerAdapterImpl Instance Destroyed");
}

bool OpenCVViewerAdapterImpl::start()
{
    LOG_INF("Starting OpenCV Viewer...");
    if (config_.viewerType != config::VslamViewerType::OPENCV)
    {
        THROWLOG(SysException, "Type mismatch: OpenCVViewerAdapterImpl can be used only with OPENCV viewer type.");
    }
    cv::namedWindow(window_name_, cv::WINDOW_AUTOSIZE);
    return true;
}

bool OpenCVViewerAdapterImpl::stop()
{
    LOG_INF("Stopping OpenCV Viewer...");
    if (cv::getWindowProperty(window_name_, cv::WND_PROP_VISIBLE) >= 1)
    {
        cv::destroyWindow(window_name_);
    }
    return true;
}
void OpenCVViewerAdapterImpl::render(const domain::model::Pose &pose,
                                     const std::vector<domain::model::Detection> &detections,
                                     const domain::model::ImagePacket &frame)
{
    cv::Mat canvas;

    // 이미지 획득 (if constexpr 사용으로 변경)
    std::visit([&](auto &&arg)
               {
        using T = std::decay_t<decltype(arg)>;

        // MonoImagePacket
        if constexpr (std::is_same_v<T, domain::model::MonoImagePacket>) //NOLINT
        {
            auto type = (arg.frame.channels == 1) ? CV_8UC1 : CV_8UC3; //NOLINT: opencv
            canvas = cv::Mat(arg.frame.height, arg.frame.width, type,
                             const_cast<uint8_t *>(arg.frame.data.data()))
                         .clone();

            if (type == CV_8UC1) //NOLINT: opencv
            {
                cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR);
            }
        }
        // StereoImagePacket
        else if constexpr (std::is_same_v<T, domain::model::StereoImagePacket>)
        {
            auto type = (arg.left.channels == 1) ? CV_8UC1 : CV_8UC3; //NOLINT: opencv
            cv::Mat left_mat(arg.left.height, arg.left.width, type,
                             const_cast<uint8_t *>(arg.left.data.data()));
            cv::Mat right_mat(arg.right.height, arg.right.width, type,
                              const_cast<uint8_t *>(arg.right.data.data()));

            cv::vconcat(left_mat, right_mat, canvas);

            if (type == CV_8UC1) //NOLINT: opencv
            {
                cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR);
            }
        } }, frame.payload);

    if (canvas.empty())
    {
        return;
    }

    // Object Detections 그리기
    for (const auto &det : detections)
    {
        cv::Rect rect(static_cast<int>(det.bbox.x),
                      static_cast<int>(det.bbox.y),
                      static_cast<int>(det.bbox.width),
                      static_cast<int>(det.bbox.height));

        cv::Scalar box_color(0, 255, 255); // 노란색
        cv::rectangle(canvas, rect, box_color, 2);

        std::string class_name = domain::model::ClassIdHelper::toString(det.class_id);

        std::stringstream ss;
        ss << class_name << " " << std::fixed << std::setprecision(2) << det.confidence;
        std::string label_text = ss.str();

        double font_scale = 0.5;
        int font_thickness = 1;
        int baseline = 0;

        cv::Size text_size = cv::getTextSize(label_text, cv::FONT_HERSHEY_SIMPLEX, font_scale, font_thickness, &baseline);

        cv::rectangle(canvas,
                      cv::Point(rect.x, rect.y - text_size.height - 5),
                      cv::Point(rect.x + text_size.width, rect.y),
                      box_color, -1);

        cv::putText(canvas, label_text,
                    cv::Point(rect.x, rect.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), font_thickness);
    }

    std::string status = pose.is_lost ? "LOST" : "TRACKING";
    std::string pos_text = "Status: " + status +
                           " | Pose: X=" + std::to_string(pose.x).substr(0, 5) +
                           ", Y=" + std::to_string(pose.y).substr(0, 5);

    cv::Scalar text_color = pose.is_lost ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
    cv::putText(canvas, pos_text, cv::Point(20, 40),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, text_color, 2);

    cv::imshow(window_name_, canvas);
    cv::waitKey(1);
}

} // namespace vp::adapter::out