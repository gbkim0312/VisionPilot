// adapter/out/visualizer/src/opencv_viewer_adapter_impl.cpp
#include "opencv_viewer_adapter_impl.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "viewer_config.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace vp::adapter::out
{
OpenCVViewerAdapterImpl::OpenCVViewerAdapterImpl(const config::ViewerConfig &config)
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
    if (config_.viewerType != config::ViewerType::OPENCV)
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
                                     const domain::model::DetectionResult &detections,
                                     const domain::model::TrackingResult &tracking,
                                     const domain::model::ImagePacket &frame)
{
    cv::Mat canvas;
    std::visit(
        [&](auto &&arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, domain::model::MonoImagePacket>)
            {
                auto type = (arg.frame.channels == 1) ? CV_8UC1 : CV_8UC3;
                canvas = cv::Mat(
                             arg.frame.height,
                             arg.frame.width,
                             type,
                             const_cast<uint8_t *>(arg.frame.data.data()))
                             .clone();

                if (type == CV_8UC1)
                {
                    cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR);
                }
            }
            else if constexpr (std::is_same_v<T, domain::model::StereoImagePacket>)
            {
                auto type = (arg.left.channels == 1) ? CV_8UC1 : CV_8UC3;

                cv::Mat left_mat(
                    arg.left.height,
                    arg.left.width,
                    type,
                    const_cast<uint8_t *>(arg.left.data.data()));

                cv::Mat right_mat(
                    arg.right.height,
                    arg.right.width,
                    type,
                    const_cast<uint8_t *>(arg.right.data.data()));

                cv::vconcat(left_mat, right_mat, canvas);

                if (type == CV_8UC1)
                {
                    cv::cvtColor(canvas, canvas, cv::COLOR_GRAY2BGR);
                }
            }
        },
        frame.payload);

    if (canvas.empty())
    {
        return;
    }

    for (const auto &det : detections.detections)
    {
        cv::Rect rect(
            static_cast<int>(det.bbox.x),
            static_cast<int>(det.bbox.y),
            static_cast<int>(det.bbox.width),
            static_cast<int>(det.bbox.height));

        const cv::Scalar box_color{0, 255, 255};
        cv::rectangle(canvas, rect, box_color, 2);

        const auto class_name = domain::model::ClassIdHelper::toString(det.class_id);

        std::stringstream ss;
        ss << "DET " << class_name << " " << std::fixed << std::setprecision(2) << det.confidence;

        const auto label_text = ss.str();
        constexpr double kFontScale = 0.5;
        constexpr int kFontThickness = 1;

        auto baseline = 0;
        const auto text_size =
            cv::getTextSize(label_text, cv::FONT_HERSHEY_SIMPLEX, kFontScale, kFontThickness, &baseline);

        const auto label_top = std::max(0, rect.y - text_size.height - 5);

        cv::rectangle(
            canvas,
            cv::Point{rect.x, label_top},
            cv::Point{rect.x + text_size.width, rect.y},
            box_color,
            -1);

        cv::putText(
            canvas,
            label_text,
            cv::Point{rect.x, rect.y - 5},
            cv::FONT_HERSHEY_SIMPLEX,
            kFontScale,
            cv::Scalar{0, 0, 0},
            kFontThickness);
    }

    auto tracked_count = 0;
    auto new_count = 0;
    auto lost_count = 0;

    for (const auto &obj : tracking.objects)
    {
        cv::Scalar box_color{};
        std::string status_text;

        switch (obj.status)
        {
        case domain::model::TrackStatus::NEW:
            box_color = cv::Scalar{255, 0, 0};
            status_text = "NEW";
            ++new_count;
            break;
        case domain::model::TrackStatus::TRACKED:
            box_color = cv::Scalar{0, 255, 0};
            status_text = "TRACKED";
            ++tracked_count;
            break;
        case domain::model::TrackStatus::LOST:
            box_color = cv::Scalar{0, 0, 255};
            status_text = "LOST";
            ++lost_count;
            break;
        case domain::model::TrackStatus::REMOVED:
            continue;
        }

        cv::Rect rect(
            static_cast<int>(obj.bbox.x),
            static_cast<int>(obj.bbox.y),
            static_cast<int>(obj.bbox.width),
            static_cast<int>(obj.bbox.height));

        cv::rectangle(canvas, rect, box_color, 2);

        const auto class_name = domain::model::ClassIdHelper::toString(obj.class_id);

        std::stringstream ss;
        ss << "ID:" << obj.track_id
           << " " << class_name
           << " " << status_text
           << " C:" << std::fixed << std::setprecision(2) << obj.confidence;

        const auto line1 = ss.str();

        std::stringstream vs;
        vs << "V:(" << std::fixed << std::setprecision(1)
           << obj.velocity.x << "," << obj.velocity.y << ") "
           << "Age:" << obj.tracking_age
           << " Lost:" << obj.lost_count;

        const auto line2 = vs.str();

        constexpr double kFontScale = 0.5;
        constexpr int kFontThickness = 1;

        auto baseline_1 = 0;
        auto baseline_2 = 0;

        const auto text_size_1 = cv::getTextSize(line1, cv::FONT_HERSHEY_SIMPLEX, kFontScale, kFontThickness, &baseline_1);
        const auto text_size_2 = cv::getTextSize(line2, cv::FONT_HERSHEY_SIMPLEX, kFontScale, kFontThickness, &baseline_2);

        const auto label_width = std::max(text_size_1.width, text_size_2.width);
        const auto label_height = text_size_1.height + text_size_2.height + 10;
        const auto label_top = std::max(0, rect.y - label_height);

        cv::rectangle(
            canvas,
            cv::Point{rect.x, label_top},
            cv::Point{rect.x + label_width + 4, rect.y},
            box_color,
            -1);

        cv::putText(
            canvas,
            line1,
            cv::Point{rect.x + 2, label_top + text_size_1.height + 1},
            cv::FONT_HERSHEY_SIMPLEX,
            kFontScale,
            cv::Scalar{0, 0, 0},
            kFontThickness);

        cv::putText(
            canvas,
            line2,
            cv::Point{rect.x + 2, label_top + text_size_1.height + text_size_2.height + 5},
            cv::FONT_HERSHEY_SIMPLEX,
            kFontScale,
            cv::Scalar{0, 0, 0},
            kFontThickness);

        const auto center_x = obj.bbox.x + obj.bbox.width * 0.5F;
        const auto center_y = obj.bbox.y + obj.bbox.height * 0.5F;

        constexpr auto kVelocityArrowScale = 5.0F;

        const cv::Point start_point{
            static_cast<int>(center_x),
            static_cast<int>(center_y)};

        const cv::Point end_point{
            static_cast<int>(center_x + obj.velocity.x * kVelocityArrowScale),
            static_cast<int>(center_y + obj.velocity.y * kVelocityArrowScale)};

        cv::arrowedLine(canvas, start_point, end_point, box_color, 2);
    }

    const auto status = pose.is_lost ? std::string{"LOST"} : std::string{"TRACKING"};

    std::stringstream pose_ss;
    pose_ss << "Status: " << status
            << " | Pose: X=" << std::fixed << std::setprecision(2) << pose.x
            << ", Y=" << pose.y;

    const auto pose_text = pose_ss.str();
    const cv::Scalar pose_text_color = pose.is_lost ? cv::Scalar{0, 0, 255} : cv::Scalar{0, 255, 0};

    cv::putText(
        canvas,
        pose_text,
        cv::Point{20, 30},
        cv::FONT_HERSHEY_SIMPLEX,
        0.7,
        pose_text_color,
        2);

    std::stringstream track_summary_ss;
    track_summary_ss << "Tracks: total=" << tracking.objects.size()
                     << " tracked=" << tracked_count
                     << " new=" << new_count
                     << " lost=" << lost_count;

    cv::putText(
        canvas,
        track_summary_ss.str(),
        cv::Point{20, 60},
        cv::FONT_HERSHEY_SIMPLEX,
        0.7,
        cv::Scalar{255, 255, 255},
        2);

    cv::imshow(window_name_, canvas);
    cv::waitKey(1);
}
} // namespace vp::adapter::out