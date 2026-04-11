#pragma once
#include <vector>
#include <cstdint>
#include "detection.hpp" // BoundingBox, ClassId 포함 가정

namespace vp::domain::model
{

/**
 * @brief 트래킹 상태 정의
 * NEW: 처음 발견됨 (신뢰도 낮음)
 * TRACKED: 연속적으로 매칭되어 신뢰할 수 있음
 * LOST: 검출은 안 됐으나 칼만 필터 예측으로 유지 중 (Occlusion 상황)
 */
enum class TrackStatus {
    NEW,
    TRACKED,
    LOST,
    REMOVED
};

struct Vector2D {
    float x;
    float y;
};

/**
 * @brief 추적된 개별 객체 정보
 */
struct TrackedObject {
    int32_t track_id;          // 객체 고유 식별 번호
    ClassId class_id;          // 객체 클래스 (Car, Pedestrian 등)
    BoundingBox bbox;          // 현재 프레임의 위치 및 크기
    float confidence;          // 검출/추적 종합 신뢰도
    
    // 칼만 필터의 핵심 산출물
    Vector2D velocity;         // 객체의 이동 속도 (px/s 또는 m/s)
    TrackStatus status;        // 현재 추적 상태
    
    uint32_t tracking_age;     // 추적 시작 후 경과 프레임 수
    uint32_t lost_count;       // 연속적으로 검출되지 않은 프레임 수
};

/**
 * @brief ObjectTrackingPort의 최종 반환형
 */
struct TrackingResult {
    uint64_t frame_id;         // 프레임 번호
    uint64_t timestamp;        // 타임스탬프 (ms)
    std::vector<TrackedObject> objects; // 추적 중인 객체 리스트
};

} // namespace vp::domain::model