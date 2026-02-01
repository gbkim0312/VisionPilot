#include "vision_pilot_service_impl.hpp"
#include "gaia_log.hpp"
#include "vision_pilot_service.hpp"

namespace vp::service
{

VisionPilotServiceImpl::VisionPilotServiceImpl(vp::port::out::LocalizationPort &localization_port,
                                               vp::port::out::VisualizationPort &visualization_port,
                                               vp::port::out::ObjectDetectionPort &object_detection_port)
    : localization_port_{localization_port},
      visualization_port_{visualization_port},
      object_detection_port_{object_detection_port}
{
    LOG_TRA("Starting VisionPilot Service...");

    is_running_ = true;
    detection_thread_ = std::thread(&VisionPilotServiceImpl::detectionLoop, this);
}

VisionPilotServiceImpl::~VisionPilotServiceImpl()
{
    LOG_TRA("Stopping VisionPilot Service...");

    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        is_running_ = false;
        new_frame_available_ = true; // 자고 있는 스레드를 깨우기 위해 true 설정
    }
    detection_cv_.notify_all(); // 스레드 깨움

    if (detection_thread_.joinable())
    {
        detection_thread_.join();
    }
}

void VisionPilotServiceImpl::onFrameReceived(const domain::model::ImagePacket &frame)
{
    auto pose = localization_port_.update(frame, frame.timestamp);

    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        latest_frame_ = frame; // 최신 프레임 덮어쓰기 (큐가 아님! 이전거 버림)
        new_frame_available_ = true;
    }
    detection_cv_.notify_one(); // 자고 있는 탐지기 깨우기

    std::vector<domain::model::Detection> current_detections;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        current_detections = latest_results_; // 가장 최근 결과 복사
    }

    visualization_port_.render(pose, current_detections, frame);
}

void VisionPilotServiceImpl::detectionLoop()
{
    while (is_running_)
    {
        domain::model::ImagePacket frame_to_process;

        {
            std::unique_lock<std::mutex> lock(data_mutex_);

            detection_cv_.wait(lock, [this]
                               { return !is_running_ || new_frame_available_; });

            if (!is_running_)
            {
                break;
            }

            // 데이터를 가져오고 플래그 초기화
            if (latest_frame_.has_value())
            {
                frame_to_process = latest_frame_.value();
                new_frame_available_ = false; // 처리 시작하니까 플래그 내림
            }
            else
            {
                continue; // 데이터 없으면 다시 대기
            }
        }

        auto detections = object_detection_port_.detectObject(frame_to_process);

        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            latest_results_ = std::move(detections);
        }
    }
}

} // namespace vp::service