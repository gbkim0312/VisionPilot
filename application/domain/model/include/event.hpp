// application/domain/model/include/event.hpp
#pragma once

#include "image.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace vp::domain::model
{

/**
 * @brief 시스템에서 발생하는 이벤트의 종류
 */
enum class EventType
{
    NONE = 0,
    IMAGE,   // 카메라 영상 데이터
    IMU,     // 가속도/자이로 센서 데이터
    CAN_BUS, // 차량 제어/상태 정보
    LIDAR    // 라이다 점군 데이터
};

using ImageEventPayload = std::shared_ptr<ImagePacket>;

using EventData = std::variant<
    std::monostate,
    ImageEventPayload>;

/**
 * @brief 모든 센서 데이터를 통합 관리하기 위한 이벤트 구조체
 */
struct Event
{
    EventType type = EventType::NONE; // 이벤트 타입
    uint64_t timestamp = 0;           // 발생 시간 (ms)
    EventData data;                   // 실제 데이터 (shared_ptr<ImagePacket> 등)
    std::string source;               // 발생원 (예: "FL_Camera", "Main_IMU")

    Event() = default;
    Event(EventType t, EventData d, uint64_t ts)
        : type(t), data(std::move(d)), timestamp(ts) {}
};

} // namespace vp::domain::model