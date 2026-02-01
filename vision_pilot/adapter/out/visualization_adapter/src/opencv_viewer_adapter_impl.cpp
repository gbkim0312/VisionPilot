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

void OpenCVViewerAdapterImpl::render(const domain::model::Pose &pose,
                                     std::vector<domain::model::Detection> detections,
                                     const domain::model::ImagePacket &frame)
{
    cv::Mat canvas;

    // 1. 이미지 획득 (기존 코드 유지)
    std::visit(overloaded{[&](const domain::model::MonoImagePacket &mono)
                          {
                              int type = (mono.frame.channels == 1) ? CV_8UC1 : CV_8UC3;
                              canvas = cv::Mat(mono.frame.height, mono.frame.width, type,
                                               const_cast<uint8_t *>(mono.frame.data.data()))
                                           .clone();
                              if (type == CV_8UC1)
                                  cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR);
                          },
                          [&](const domain::model::StereoImagePacket &stereo)
                          {
                              int type = (stereo.left.channels == 1) ? CV_8UC1 : CV_8UC3;
                              cv::Mat left_mat(stereo.left.height, stereo.left.width, type,
                                               const_cast<uint8_t *>(stereo.left.data.data()));
                              cv::Mat right_mat(stereo.right.height, stereo.right.width, type,
                                                const_cast<uint8_t *>(stereo.right.data.data()));
                              cv::vconcat(left_mat, right_mat, canvas);
                              if (type == CV_8UC1)
                                  cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR);
                          }},
               frame.payload);

    if (canvas.empty())
        return;

    // Object Detections 그리기
    for (const auto &det : detections)
    {
        // 사각형 좌표 계산
        cv::Rect rect(static_cast<int>(det.bbox.x),
                      static_cast<int>(det.bbox.y),
                      static_cast<int>(det.bbox.width),
                      static_cast<int>(det.bbox.height));

        // 박스 그리기
        cv::Scalar box_color(0, 255, 255); // 노란색
        cv::rectangle(canvas, rect, box_color, 2);

        // 라벨 텍스트 생성
        // "Person 0.95"
        std::string class_name = domain::model::ClassIdHelper::toString(det.class_id);

        std::stringstream ss;
        ss << class_name << " " << std::fixed << std::setprecision(2) << det.confidence;
        std::string label_text = ss.str();

        // 텍스트 배경 그리기
        // 폰트 크기(0.5)와 두께(1)를 작게 설정
        double font_scale = 0.5;
        int font_thickness = 1;
        int baseline = 0;

        cv::Size text_size = cv::getTextSize(label_text, cv::FONT_HERSHEY_SIMPLEX, font_scale, font_thickness, &baseline);

        // 박스 바로 위에 텍스트 배경 (꽉 찬 사각형)
        cv::rectangle(canvas,
                      cv::Point(rect.x, rect.y - text_size.height - 5),
                      cv::Point(rect.x + text_size.width, rect.y),
                      box_color, -1);

        // 텍스트 그리기 (검은색)
        cv::putText(canvas, label_text,
                    cv::Point(rect.x, rect.y - 5), // 박스 위쪽 라인에 딱 붙게
                    cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0), font_thickness);
    }

    // 포즈 정보 표시
    std::string status = pose.is_lost ? "LOST" : "TRACKING";
    std::string pos_text = "Status: " + status +
                           " | Pose: X=" + std::to_string(pose.x).substr(0, 5) +
                           ", Y=" + std::to_string(pose.y).substr(0, 5);

    cv::Scalar text_color = pose.is_lost ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
    cv::putText(canvas, pos_text, cv::Point(20, 40),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, text_color, 2);

    // 화면 표시
    cv::imshow(window_name_, canvas);
    cv::waitKey(1);
}
} // namespace vp::adapter::out