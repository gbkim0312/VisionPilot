// adapter/out/visualizer/src/opencv_viewer_adapter_impl.cpp
#include "opencv_viewer_adapter_impl.hpp"
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
    // OpenCV 창 미리 생성 (옵션)
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
    // 1. Raw Data를 cv::Mat으로 변환 (Zero-copy view)
    // ImagePacket 구조에 따라 CV_8UC1(Gray) 또는 CV_8UC3(RGB) 선택
    int type = (frame.channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat canvas(frame.height, frame.width, type, const_cast<uint8_t *>(frame.data.data()));

    // 2. 포즈 정보 오버레이 (간단한 텍스트 출력)
    std::string pos_text = "Pose: X=" + std::to_string(pose.x).substr(0, 5) +
                           ", Y=" + std::to_string(pose.y).substr(0, 5);

    cv::putText(canvas, pos_text, cv::Point(20, 40),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

    // 3. 화면 표시
    cv::imshow(window_name_, canvas);

    cv::waitKey(1);
}

} // namespace vp::adapter::out