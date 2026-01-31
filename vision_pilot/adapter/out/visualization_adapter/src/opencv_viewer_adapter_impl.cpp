// adapter/out/visualizer/src/opencv_viewer_adapter_impl.cpp
#include "opencv_viewer_adapter_impl.hpp"
#include "gaia_log.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace vp::adapter::out
{

template <class... Ts>
struct overloaded : Ts... // NOLINT(fuchsia-multiple-inheritance)
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

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
    cv::namedWindow(window_name_, cv::WINDOW_AUTOSIZE);
    return true;
}

bool OpenCVViewerAdapterImpl::stop()
{
    LOG_INF("Stopping OpenCV Viewer...");
    cv::destroyWindow(window_name_);
    return true;
}

void OpenCVViewerAdapterImpl::render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame)
{
    cv::Mat canvas;

    // 1. Variant 페이로드 분기 처리
    std::visit(overloaded{[&](const domain::model::MonoImagePacket &mono)
                          {
                              // [Mono] 단일 영상을 캔버스로 사용
                              int type = (mono.frame.channels == 1) ? CV_8UC1 : CV_8UC3;
                              canvas = cv::Mat(mono.frame.height, mono.frame.width, type,
                                               const_cast<uint8_t *>(mono.frame.data.data()))
                                           .clone();
                          },
                          [&](const domain::model::StereoImagePacket &stereo)
                          {
                              // [Stereo] 좌/우 영상을 세로로 이어붙임 (Side-by-Side)
                              int type = (stereo.left.channels == 1) ? CV_8UC1 : CV_8UC3;
                              cv::Mat left_mat(stereo.left.height, stereo.left.width, type,
                                               const_cast<uint8_t *>(stereo.left.data.data()));
                              cv::Mat right_mat(stereo.right.height, stereo.right.width, type,
                                                const_cast<uint8_t *>(stereo.right.data.data()));

                              // 두 영상을 세로로 결합
                              cv::vconcat(left_mat, right_mat, canvas);
                          }},
               frame.payload);

    if (canvas.empty())
    {
        LOG_ERR("Canvas is empty. Failed to process ImagePacket payload.");
        return;
    }

    // 2. 포즈 정보 오버레이 (텍스트 정보)
    std::string status = pose.is_lost ? "LOST" : "TRACKING";
    std::string pos_text = "Status: " + status +
                           " | Pose: X=" + std::to_string(pose.x).substr(0, 5) +
                           ", Y=" + std::to_string(pose.y).substr(0, 5);

    // 가독성을 위해 배경색이 있는 텍스트 오버레이
    cv::Scalar text_color = pose.is_lost ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
    cv::putText(canvas, pos_text, cv::Point(20, 40),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, text_color, 2);

    // 3. 화면 표시
    cv::imshow(window_name_, canvas);
    cv::waitKey(1); // GUI 이벤트를 위해 필수
}

} // namespace vp::adapter::out