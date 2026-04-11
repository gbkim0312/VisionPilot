#include "object_tracking_adapter_impl.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "gaia_log.hpp"
#include "object_tracking_adapter.hpp"

namespace
{
constexpr auto kPi = 3.14159265358979323846F;
}

namespace vp::adapter::out
{
ObjectTrackingAdapterImpl::ObjectTrackingAdapterImpl()
{
    LOG_TRA("");

    measurement_matrix_.setZero();
    measurement_matrix_(0, 0) = 1.0F;
    measurement_matrix_(1, 1) = 1.0F;
    measurement_matrix_(2, 4) = 1.0F;
    measurement_matrix_(3, 5) = 1.0F;

    measurement_noise_matrix_.setZero();
    measurement_noise_matrix_(0, 0) = kMeasurementNoisePosition;
    measurement_noise_matrix_(1, 1) = kMeasurementNoisePosition;
    measurement_noise_matrix_(2, 2) = kMeasurementNoiseSize;
    measurement_noise_matrix_(3, 3) = kMeasurementNoiseSize;

    this->initializeKalmanMatrices(kDefaultDtSec);
}

ObjectTrackingAdapterImpl::~ObjectTrackingAdapterImpl()
{
    LOG_TRA("");
}

domain::model::TrackingResult ObjectTrackingAdapterImpl::update(
    const std::optional<domain::model::DetectionResult> &detection,
    const domain::model::Pose &ego_pose,
    uint64_t timestamp)
{
    LOG_TRA("");

    auto dt_sec = kDefaultDtSec;
    if (prev_timestamp_.has_value() && timestamp > *prev_timestamp_)
    {
        const auto dt_ms = timestamp - *prev_timestamp_;
        dt_sec = static_cast<float>(dt_ms) / 1000.0F;
        if (dt_sec > kMaxDtSec)
        {
            dt_sec = kMaxDtSec;
        }
    }

    this->initializeKalmanMatrices(dt_sec);

    for (auto &track : tracks_)
    {
        track.matched_in_frame = false;
    }

    this->predictTracks();

    if (prev_ego_pose_.has_value())
    {
        this->compensateTracksByEgoMotion(*prev_ego_pose_, ego_pose);
    }

    const auto measurements = this->convertDetections(detection);
    this->matchDetectionsToTracks(measurements);
    this->markUnmatchedTracks();
    this->removeDeadTracks();

    prev_ego_pose_ = ego_pose;
    prev_timestamp_ = timestamp;

    uint64_t frame_id = 0;
    if (detection.has_value())
    {
        frame_id = detection->frame_id;
        last_frame_id_ = frame_id;
    }
    else
    {
        ++last_frame_id_;
        frame_id = last_frame_id_;
    }

    return this->buildTrackingResult(frame_id, timestamp);
}

void ObjectTrackingAdapterImpl::initializeKalmanMatrices(float dt_sec)
{
    transition_matrix_ = StateMatrix::Identity();
    transition_matrix_(0, 2) = dt_sec;
    transition_matrix_(1, 3) = dt_sec;

    process_noise_matrix_ = StateMatrix::Zero();
    process_noise_matrix_(0, 0) = kProcessNoisePosition * dt_sec;
    process_noise_matrix_(1, 1) = kProcessNoisePosition * dt_sec;
    process_noise_matrix_(2, 2) = kProcessNoiseVelocity * dt_sec;
    process_noise_matrix_(3, 3) = kProcessNoiseVelocity * dt_sec;
    process_noise_matrix_(4, 4) = kProcessNoiseSize * dt_sec;
    process_noise_matrix_(5, 5) = kProcessNoiseSize * dt_sec;
}

void ObjectTrackingAdapterImpl::predictTracks()
{
    for (auto &track : tracks_)
    {
        track.state = transition_matrix_ * track.state;
        track.covariance = transition_matrix_ * track.covariance * transition_matrix_.transpose() + process_noise_matrix_;
        track.tracking_age += 1;
    }
}

void ObjectTrackingAdapterImpl::compensateTracksByEgoMotion(
    const domain::model::Pose &prev_pose,
    const domain::model::Pose &current_pose)
{
    const auto prev_yaw = this->quaternionToYaw(prev_pose);
    const auto current_yaw = this->quaternionToYaw(current_pose);
    const auto delta_yaw = this->normalizeAngle(current_yaw - prev_yaw);

    const auto delta_x_world = static_cast<float>(current_pose.x - prev_pose.x);
    const auto delta_y_world = static_cast<float>(current_pose.y - prev_pose.y);

    Vector2 translation_world{};
    translation_world << delta_x_world, delta_y_world;

    const auto translation_in_prev_ego = this->rotate2D(translation_world, -prev_yaw);

    for (auto &track : tracks_)
    {
        Vector2 position{};
        position << track.state(0), track.state(1);
        position -= translation_in_prev_ego;
        position = this->rotate2D(position, -delta_yaw);
        track.state(0) = position(0);
        track.state(1) = position(1);

        Vector2 velocity{};
        velocity << track.state(2), track.state(3);
        velocity = this->rotate2D(velocity, -delta_yaw);
        track.state(2) = velocity(0);
        track.state(3) = velocity(1);
    }
}

void ObjectTrackingAdapterImpl::updateMatchedTrack(KalmanTrack &track, const DetectionMeasurement &measurement)
{
    const auto innovation = measurement.z - measurement_matrix_ * track.state;
    const auto innovation_covariance =
        measurement_matrix_ * track.covariance * measurement_matrix_.transpose() + measurement_noise_matrix_;
    const auto kalman_gain =
        track.covariance * measurement_matrix_.transpose() * innovation_covariance.inverse();

    track.state = track.state + kalman_gain * innovation;

    const auto identity = StateMatrix::Identity();
    track.covariance = (identity - kalman_gain * measurement_matrix_) * track.covariance;

    track.class_id = measurement.class_id;
    track.confidence = measurement.confidence;
    track.state(4) = this->clampPositive(track.state(4));
    track.state(5) = this->clampPositive(track.state(5));
    track.lost_count = 0;
    track.matched_in_frame = true;

    if (track.tracking_age >= kMinTrackedAge)
    {
        track.status = domain::model::TrackStatus::TRACKED;
    }
    else
    {
        track.status = domain::model::TrackStatus::NEW;
    }
}

void ObjectTrackingAdapterImpl::markUnmatchedTracks()
{
    for (auto &track : tracks_)
    {
        if (track.matched_in_frame)
        {
            continue;
        }

        track.lost_count += 1;
        if (track.lost_count > kMaxLostCount)
        {
            track.status = domain::model::TrackStatus::REMOVED;
        }
        else
        {
            track.status = domain::model::TrackStatus::LOST;
        }
    }
}

void ObjectTrackingAdapterImpl::createNewTrack(const DetectionMeasurement &measurement)
{
    auto track = KalmanTrack{};
    track.track_id = next_track_id_;
    ++next_track_id_;

    track.class_id = measurement.class_id;
    track.confidence = measurement.confidence;
    track.state.setZero();
    track.state(0) = measurement.z(0);
    track.state(1) = measurement.z(1);
    track.state(2) = 0.0F;
    track.state(3) = 0.0F;
    track.state(4) = this->clampPositive(measurement.z(2));
    track.state(5) = this->clampPositive(measurement.z(3));
    track.covariance = StateMatrix::Zero();
    track.covariance(0, 0) = 50.0F;
    track.covariance(1, 1) = 50.0F;
    track.covariance(2, 2) = 100.0F;
    track.covariance(3, 3) = 100.0F;
    track.covariance(4, 4) = 25.0F;
    track.covariance(5, 5) = 25.0F;
    track.status = domain::model::TrackStatus::NEW;
    track.tracking_age = 1;
    track.lost_count = 0;
    track.matched_in_frame = true;

    tracks_.push_back(track);
}

void ObjectTrackingAdapterImpl::removeDeadTracks()
{
    tracks_.erase(
        std::remove_if(
            tracks_.begin(),
            tracks_.end(),
            [](const KalmanTrack &track)
            {
                return track.status == domain::model::TrackStatus::REMOVED;
            }),
        tracks_.end());
}

domain::model::TrackingResult ObjectTrackingAdapterImpl::buildTrackingResult(uint64_t frame_id, uint64_t timestamp) const
{
    auto result = domain::model::TrackingResult{};
    result.frame_id = frame_id;
    result.timestamp = timestamp;

    for (const auto &track : tracks_)
    {
        if (track.status == domain::model::TrackStatus::REMOVED)
        {
            continue;
        }

        auto tracked_object = domain::model::TrackedObject{};
        tracked_object.track_id = track.track_id;
        tracked_object.class_id = track.class_id;
        tracked_object.bbox = this->makeBoundingBoxFromTrack(track);
        tracked_object.confidence = track.confidence;
        tracked_object.velocity = domain::model::Vector2D{track.state(2), track.state(3)};
        tracked_object.status = track.status;
        tracked_object.tracking_age = track.tracking_age;
        tracked_object.lost_count = track.lost_count;

        result.objects.push_back(tracked_object);
    }

    return result;
}

std::vector<ObjectTrackingAdapterImpl::DetectionMeasurement> ObjectTrackingAdapterImpl::convertDetections(
    const std::optional<domain::model::DetectionResult> &detection) const
{
    auto measurements = std::vector<DetectionMeasurement>{};

    if (!detection.has_value())
    {
        return measurements;
    }

    measurements.reserve(detection->detections.size());

    for (const auto &det : detection->detections)
    {
        auto measurement = DetectionMeasurement{};
        measurement.class_id = det.class_id;
        measurement.confidence = det.confidence;

        const auto width = this->clampPositive(det.bbox.width);
        const auto height = this->clampPositive(det.bbox.height);
        const auto center_x = det.bbox.x + width * 0.5F;
        const auto center_y = det.bbox.y + height * 0.5F;

        measurement.z(0) = center_x;
        measurement.z(1) = center_y;
        measurement.z(2) = width;
        measurement.z(3) = height;

        measurements.push_back(measurement);
    }

    return measurements;
}

void ObjectTrackingAdapterImpl::matchDetectionsToTracks(const std::vector<DetectionMeasurement> &measurements)
{
    struct MatchCandidate
    {
        int track_index = -1;
        int measurement_index = -1;
        float iou = 0.0F;
    };

    auto candidates = std::vector<MatchCandidate>{};

    for (int track_index = 0; track_index < static_cast<int>(tracks_.size()); ++track_index)
    {
        const auto &track = tracks_[track_index];
        if (track.status == domain::model::TrackStatus::REMOVED)
        {
            continue;
        }

        for (int measurement_index = 0; measurement_index < static_cast<int>(measurements.size()); ++measurement_index)
        {
            const auto &measurement = measurements[measurement_index];

            if (track.class_id != measurement.class_id)
            {
                continue;
            }

            const auto iou = this->calculateIoU(track, measurement);
            if (iou >= kMinIoUForMatch)
            {
                candidates.push_back(MatchCandidate{track_index, measurement_index, iou});
            }
        }
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const MatchCandidate &lhs, const MatchCandidate &rhs)
        {
            return lhs.iou > rhs.iou;
        });

    auto track_used = std::vector<bool>(tracks_.size(), false);
    auto measurement_used = std::vector<bool>(measurements.size(), false);

    for (const auto &candidate : candidates)
    {
        if (track_used[candidate.track_index] || measurement_used[candidate.measurement_index])
        {
            continue;
        }

        this->updateMatchedTrack(tracks_[candidate.track_index], measurements[candidate.measurement_index]);
        track_used[candidate.track_index] = true;
        measurement_used[candidate.measurement_index] = true;
    }

    for (int measurement_index = 0; measurement_index < static_cast<int>(measurements.size()); ++measurement_index)
    {
        if (!measurement_used[measurement_index])
        {
            this->createNewTrack(measurements[measurement_index]);
        }
    }
}

float ObjectTrackingAdapterImpl::calculateIoU(const KalmanTrack &track, const DetectionMeasurement &measurement)
{
    const float track_center_x = track.state(0);
    const float track_center_y = track.state(1);
    const float track_width = clampPositive(track.state(4));
    const float track_height = clampPositive(track.state(5));

    const float measurement_center_x = measurement.z(0);
    const float measurement_center_y = measurement.z(1);
    const float measurement_width = clampPositive(measurement.z(2));
    const float measurement_height = clampPositive(measurement.z(3));

    const float track_left = track_center_x - track_width * 0.5F;
    const float track_top = track_center_y - track_height * 0.5F;
    const float track_right = track_center_x + track_width * 0.5F;
    const float track_bottom = track_center_y + track_height * 0.5F;

    const float measurement_left = measurement_center_x - measurement_width * 0.5F;
    const float measurement_top = measurement_center_y - measurement_height * 0.5F;
    const float measurement_right = measurement_center_x + measurement_width * 0.5F;
    const float measurement_bottom = measurement_center_y + measurement_height * 0.5F;

    const float inter_left = std::max(track_left, measurement_left);
    const float inter_top = std::max(track_top, measurement_top);
    const float inter_right = std::min(track_right, measurement_right);
    const float inter_bottom = std::min(track_bottom, measurement_bottom);

    const float inter_width = std::max(0.0F, inter_right - inter_left);
    const float inter_height = std::max(0.0F, inter_bottom - inter_top);
    const float inter_area = inter_width * inter_height;

    const float track_area = track_width * track_height;
    const float measurement_area = measurement_width * measurement_height;
    const float union_area = track_area + measurement_area - inter_area;

    if (union_area <= 0.0F)
    {
        return 0.0F;
    }

    return inter_area / union_area;
}

domain::model::BoundingBox ObjectTrackingAdapterImpl::makeBoundingBoxFromTrack(const KalmanTrack &track)
{
    const auto width = clampPositive(track.state(4));
    const auto height = clampPositive(track.state(5));

    return domain::model::BoundingBox{
        track.state(0) - width * 0.5F,
        track.state(1) - height * 0.5F,
        width,
        height};
}

float ObjectTrackingAdapterImpl::quaternionToYaw(const domain::model::Pose &pose)
{
    const auto siny_cosp = static_cast<float>(2.0 * (pose.qw * pose.qz + pose.qx * pose.qy));
    const auto cosy_cosp = static_cast<float>(1.0 - 2.0 * (pose.qy * pose.qy + pose.qz * pose.qz));
    return std::atan2(siny_cosp, cosy_cosp);
}

float ObjectTrackingAdapterImpl::normalizeAngle(float angle_rad)
{
    auto normalized = angle_rad;

    while (normalized > kPi)
    {
        normalized -= 2.0F * kPi;
    }

    while (normalized < -kPi)
    {
        normalized += 2.0F * kPi;
    }

    return normalized;
}

float ObjectTrackingAdapterImpl::clampPositive(float value)
{
    return std::max(value, 1.0F);
}

ObjectTrackingAdapterImpl::Vector2 ObjectTrackingAdapterImpl::rotate2D(const Vector2 &point, float yaw_rad)
{
    Matrix2 rotation_matrix{};
    const auto cos_yaw = std::cos(yaw_rad);
    const auto sin_yaw = std::sin(yaw_rad);

    rotation_matrix << cos_yaw, -sin_yaw,
        sin_yaw, cos_yaw;

    return rotation_matrix * point;
}
} // namespace vp::adapter::out