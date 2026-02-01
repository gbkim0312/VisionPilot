#pragma once

#include "localization_port.hpp"
#include "object_detection_port.hpp"
#include "vision_pilot_service.hpp"
#include "visualization_port.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>

namespace vp::service
{

class VisionPilotServiceImpl
{
public:
    VisionPilotServiceImpl(vp::port::out::LocalizationPort &localization_port,
                           vp::port::out::VisualizationPort &visualization_port,
                           vp::port::out::ObjectDetectionPort &object_detection_port);
    ~VisionPilotServiceImpl();

    void onFrameReceived(const domain::model::ImagePacket &frame);

private:
    void detectionLoop();

private:
    vp::port::out::LocalizationPort &localization_port_;
    vp::port::out::VisualizationPort &visualization_port_;
    vp::port::out::ObjectDetectionPort &object_detection_port_;

    // --- 스레드 관리 ---
    std::thread detection_thread_{};
    std::atomic<bool> is_running_{false};

    // --- 데이터 공유 (Main Thread <-> Detection Thread) ---
    std::mutex data_mutex_{};                // 데이터 보호용 뮤텍스
    std::condition_variable detection_cv_{}; // 스레드 깨우기용

    // 공유 데이터
    std::optional<domain::model::ImagePacket> latest_frame_{}; // 탐지기가 처리할 최신 이미지
    std::vector<domain::model::Detection> latest_results_{};   // 탐지 완료된 결과
    bool new_frame_available_ = false;                         // 새 프레임 도착 플래그
};

} // namespace vp::service