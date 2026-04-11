#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <Eigen/Dense>

#include "object_tracking_adapter.hpp"

namespace vp::adapter::out
{
class ObjectTrackingAdapterImpl
{
public:
    ObjectTrackingAdapterImpl();
    ~ObjectTrackingAdapterImpl();

    domain::model::TrackingResult update(
        const std::optional<domain::model::DetectionResult> &detection,
        const domain::model::Pose &ego_pose,
        uint64_t timestamp);

private:
    using StateVector = Eigen::Matrix<float, 6, 1>;
    using StateMatrix = Eigen::Matrix<float, 6, 6>;
    using MeasureVector = Eigen::Matrix<float, 4, 1>;
    using MeasureMatrix = Eigen::Matrix<float, 4, 4>;
    using MeasureProjectMatrix = Eigen::Matrix<float, 4, 6>;
    using Vector2 = Eigen::Matrix<float, 2, 1>;
    using Matrix2 = Eigen::Matrix<float, 2, 2>;

    struct KalmanTrack
    {
        int32_t track_id = 0;
        domain::model::ClassId class_id = domain::model::ClassId::UNKNOWN;
        float confidence = 0.0F;
        StateVector state = StateVector::Zero();
        StateMatrix covariance = StateMatrix::Zero();
        domain::model::TrackStatus status = domain::model::TrackStatus::NEW;
        uint32_t tracking_age = 0;
        uint32_t lost_count = 0;
        bool matched_in_frame = false;
    };

    struct DetectionMeasurement
    {
        domain::model::ClassId class_id = domain::model::ClassId::UNKNOWN;
        float confidence = 0.0F;
        MeasureVector z = MeasureVector::Zero();
    };

private:
    void initializeKalmanMatrices(float dt_sec);
    void predictTracks();
    void compensateTracksByEgoMotion(const domain::model::Pose &prev_pose, const domain::model::Pose &current_pose);
    void updateMatchedTrack(KalmanTrack &track, const DetectionMeasurement &measurement);
    void markUnmatchedTracks();
    void createNewTrack(const DetectionMeasurement &measurement);
    void removeDeadTracks();

    domain::model::TrackingResult buildTrackingResult(uint64_t frame_id, uint64_t timestamp) const;
    std::vector<DetectionMeasurement> convertDetections(const std::optional<domain::model::DetectionResult> &detection) const;
    void matchDetectionsToTracks(const std::vector<DetectionMeasurement> &measurements);

    static float calculateIoU(const KalmanTrack &track, const DetectionMeasurement &measurement);
    static domain::model::BoundingBox makeBoundingBoxFromTrack(const KalmanTrack &track);
    static float quaternionToYaw(const domain::model::Pose &pose);
    static float normalizeAngle(float angle_rad);
    static float clampPositive(float value);
    static Vector2 rotate2D(const Vector2 &point, float yaw_rad);

private:
    std::vector<KalmanTrack> tracks_;
    std::optional<domain::model::Pose> prev_ego_pose_;
    std::optional<uint64_t> prev_timestamp_;
    int32_t next_track_id_ = 1;
    uint64_t last_frame_id_ = 0;

    StateMatrix transition_matrix_ = StateMatrix::Identity();
    StateMatrix process_noise_matrix_ = StateMatrix::Zero();
    MeasureProjectMatrix measurement_matrix_ = MeasureProjectMatrix::Zero();
    MeasureMatrix measurement_noise_matrix_ = MeasureMatrix::Zero();

private:
    static constexpr auto kMaxLostCount = 10U;
    static constexpr auto kMinTrackedAge = 2U;
    static constexpr auto kMinIoUForMatch = 0.2F;
    static constexpr auto kMaxDtSec = 0.2F;
    static constexpr auto kDefaultDtSec = 0.033F;
    static constexpr auto kProcessNoisePosition = 15.0F;
    static constexpr auto kProcessNoiseVelocity = 30.0F;
    static constexpr auto kProcessNoiseSize = 10.0F;
    static constexpr auto kMeasurementNoisePosition = 20.0F;
    static constexpr auto kMeasurementNoiseSize = 25.0F;
};
} // namespace vp::adapter::out