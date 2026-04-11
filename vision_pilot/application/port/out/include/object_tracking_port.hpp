#pragma  once
#include "tracking.hpp"
#include <optional>
#include "pose.hpp"

namespace vp::port::out
{
class ObjectTrackingPort
{
public:
/**
     * @brief 현재 프레임의 객체 상태 업데이트 및 예측
     * @param detection 검출 결과. Skip-frame 전략이나 검출 실패 시 nullopt 가능.
     * @param ego_pose  현재 차량의 Pose. 카메라 이동으로 인한 객체 위치 변화를 상쇄(Motion Compensation)하기 위함.
     * @param timestamp 현재 프레임의 타임스탬프(ms). Kalman Filter의 dt 계산에 사용.
     * * @return domain::model::TrackingResult ID와 속도 벡터가 포함된 추적 결과
     */
    virtual domain::model::TrackingResult update(
        const std::optional<domain::model::DetectionResult>& detection,
        const domain::model::Pose& ego_pose,
        uint64_t timestamp
    ) = 0;
};
}