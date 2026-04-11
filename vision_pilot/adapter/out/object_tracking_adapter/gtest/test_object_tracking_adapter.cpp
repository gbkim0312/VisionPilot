#include "object_tracking_adapter.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <vector>

namespace vp::adapter::out
{
namespace
{
constexpr auto kTimestampStepMs = 33ULL;
constexpr auto kFloatTolerance = 1.0e-3F;

vp::domain::model::Detection makeDetection(
    vp::domain::model::ClassId class_id,
    float confidence,
    float x,
    float y,
    float width,
    float height)
{
    auto detection = vp::domain::model::Detection{};
    detection.class_id = class_id;
    detection.confidence = confidence;
    detection.bbox = vp::domain::model::BoundingBox{x, y, width, height};
    detection.label = "test";
    return detection;
}

vp::domain::model::DetectionResult makeDetectionResult(
    uint64_t frame_id,
    uint64_t timestamp,
    const std::vector<vp::domain::model::Detection> &detections)
{
    auto result = vp::domain::model::DetectionResult{};
    result.frame_id = frame_id;
    result.timestamp = timestamp;
    result.detections = detections;
    return result;
}

vp::domain::model::Pose makePose(double x, double y, double yaw_rad, uint64_t timestamp)
{
    auto pose = vp::domain::model::Pose{};
    pose.x = x;
    pose.y = y;
    pose.z = 0.0;
    pose.qw = std::cos(yaw_rad * 0.5);
    pose.qx = 0.0;
    pose.qy = 0.0;
    pose.qz = std::sin(yaw_rad * 0.5);
    pose.timestamp = timestamp;
    pose.is_lost = false;
    return pose;
}
} // namespace

class ObjectTrackingAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adapter = std::make_unique<vp::adapter::out::ObjectTrackingAdapter>();
    }

    void TearDown() override
    {
        adapter.reset();
    }

    std::unique_ptr<vp::adapter::out::ObjectTrackingAdapter> adapter;
};

TEST_F(ObjectTrackingAdapterTest, createTrackWhenSingleDetectionIsGiven)
{
    const auto timestamp = 1000ULL;
    const auto pose = makePose(0.0, 0.0, 0.0, timestamp);
    const auto detection_result = makeDetectionResult(
        1,
        timestamp,
        {makeDetection(vp::domain::model::ClassId::CAR, 0.95F, 100.0F, 50.0F, 40.0F, 20.0F)});

    const auto tracking_result = adapter->update(detection_result, pose, timestamp);

    ASSERT_EQ(tracking_result.frame_id, 1U);
    ASSERT_EQ(tracking_result.timestamp, timestamp);
    ASSERT_EQ(tracking_result.objects.size(), 1U);

    const auto &object = tracking_result.objects.front();
    EXPECT_EQ(object.track_id, 1);
    EXPECT_EQ(object.class_id, vp::domain::model::ClassId::CAR);
    EXPECT_EQ(object.status, vp::domain::model::TrackStatus::NEW);
    EXPECT_EQ(object.tracking_age, 1U);
    EXPECT_EQ(object.lost_count, 0U);
    EXPECT_FLOAT_EQ(object.bbox.x, 100.0F);
    EXPECT_FLOAT_EQ(object.bbox.y, 50.0F);
    EXPECT_FLOAT_EQ(object.bbox.width, 40.0F);
    EXPECT_FLOAT_EQ(object.bbox.height, 20.0F);
}

TEST_F(ObjectTrackingAdapterTest, keepSameTrackIdForConsecutiveDetection)
{
    const auto timestamp_1 = 1000ULL;
    const auto timestamp_2 = timestamp_1 + kTimestampStepMs;
    const auto pose_1 = makePose(0.0, 0.0, 0.0, timestamp_1);
    const auto pose_2 = makePose(0.0, 0.0, 0.0, timestamp_2);

    const auto detection_result_1 = makeDetectionResult(
        1,
        timestamp_1,
        {makeDetection(vp::domain::model::ClassId::PERSON, 0.90F, 50.0F, 60.0F, 30.0F, 40.0F)});

    const auto detection_result_2 = makeDetectionResult(
        2,
        timestamp_2,
        {makeDetection(vp::domain::model::ClassId::PERSON, 0.92F, 52.0F, 61.0F, 30.0F, 40.0F)});

    const auto tracking_result_1 = adapter->update(detection_result_1, pose_1, timestamp_1);
    const auto tracking_result_2 = adapter->update(detection_result_2, pose_2, timestamp_2);

    ASSERT_EQ(tracking_result_1.objects.size(), 1U);
    ASSERT_EQ(tracking_result_2.objects.size(), 1U);

    const auto track_id_1 = tracking_result_1.objects.front().track_id;
    const auto &object_2 = tracking_result_2.objects.front();

    EXPECT_EQ(object_2.track_id, track_id_1);
    EXPECT_EQ(object_2.status, vp::domain::model::TrackStatus::TRACKED);
    EXPECT_GE(object_2.tracking_age, 2U);
    EXPECT_EQ(object_2.lost_count, 0U);
}

TEST_F(ObjectTrackingAdapterTest, markTrackLostWhenDetectionIsMissing)
{
    const auto timestamp_1 = 1000ULL;
    const auto timestamp_2 = timestamp_1 + kTimestampStepMs;
    const auto pose_1 = makePose(0.0, 0.0, 0.0, timestamp_1);
    const auto pose_2 = makePose(0.0, 0.0, 0.0, timestamp_2);

    const auto detection_result = makeDetectionResult(
        1,
        timestamp_1,
        {makeDetection(vp::domain::model::ClassId::CAR, 0.95F, 120.0F, 80.0F, 50.0F, 25.0F)});

    const auto tracking_result_1 = adapter->update(detection_result, pose_1, timestamp_1);
    const auto tracking_result_2 = adapter->update(std::nullopt, pose_2, timestamp_2);

    ASSERT_EQ(tracking_result_1.objects.size(), 1U);
    ASSERT_EQ(tracking_result_2.objects.size(), 1U);

    const auto track_id = tracking_result_1.objects.front().track_id;
    const auto &lost_object = tracking_result_2.objects.front();

    EXPECT_EQ(lost_object.track_id, track_id);
    EXPECT_EQ(lost_object.status, vp::domain::model::TrackStatus::LOST);
    EXPECT_EQ(lost_object.lost_count, 1U);
}

TEST_F(ObjectTrackingAdapterTest, removeTrackWhenDetectionIsMissingForLongTime)
{
    const auto initial_timestamp = 1000ULL;
    const auto initial_pose = makePose(0.0, 0.0, 0.0, initial_timestamp);

    const auto detection_result = makeDetectionResult(
        1,
        initial_timestamp,
        {makeDetection(vp::domain::model::ClassId::BUS, 0.88F, 200.0F, 100.0F, 60.0F, 30.0F)});

    auto tracking_result = adapter->update(detection_result, initial_pose, initial_timestamp);
    ASSERT_EQ(tracking_result.objects.size(), 1U);

    for (uint64_t i = 1; i <= 11; ++i)
    {
        const auto timestamp = initial_timestamp + i * kTimestampStepMs;
        const auto pose = makePose(0.0, 0.0, 0.0, timestamp);
        tracking_result = adapter->update(std::nullopt, pose, timestamp);
    }

    EXPECT_TRUE(tracking_result.objects.empty());
}

TEST_F(ObjectTrackingAdapterTest, compensateTrackByEgoMotionWhenVehicleMovesForward)
{
    const auto timestamp_1 = 1000ULL;
    const auto timestamp_2 = timestamp_1 + kTimestampStepMs;

    const auto pose_1 = makePose(0.0, 0.0, 0.0, timestamp_1);
    const auto pose_2 = makePose(1.0, 0.0, 0.0, timestamp_2);

    const auto detection_result = makeDetectionResult(
        1,
        timestamp_1,
        {makeDetection(vp::domain::model::ClassId::TRUCK, 0.93F, 100.0F, 50.0F, 40.0F, 20.0F)});

    const auto tracking_result_1 = adapter->update(detection_result, pose_1, timestamp_1);
    const auto tracking_result_2 = adapter->update(std::nullopt, pose_2, timestamp_2);

    ASSERT_EQ(tracking_result_1.objects.size(), 1U);
    ASSERT_EQ(tracking_result_2.objects.size(), 1U);

    const auto &object_1 = tracking_result_1.objects.front();
    const auto &object_2 = tracking_result_2.objects.front();

    EXPECT_LT(object_2.bbox.x, object_1.bbox.x);
    EXPECT_NEAR(object_2.bbox.y, object_1.bbox.y, kFloatTolerance);
    EXPECT_EQ(object_2.status, vp::domain::model::TrackStatus::LOST);
}

TEST_F(ObjectTrackingAdapterTest, preserveTopLeftBoundingBoxConvention)
{
    const auto timestamp = 1000ULL;
    const auto pose = makePose(0.0, 0.0, 0.0, timestamp);

    const auto detection_result = makeDetectionResult(
        1,
        timestamp,
        {makeDetection(vp::domain::model::ClassId::BICYCLE, 0.85F, 10.0F, 20.0F, 30.0F, 40.0F)});

    const auto tracking_result = adapter->update(detection_result, pose, timestamp);

    ASSERT_EQ(tracking_result.objects.size(), 1U);

    const auto &object = tracking_result.objects.front();

    EXPECT_FLOAT_EQ(object.bbox.x, 10.0F);
    EXPECT_FLOAT_EQ(object.bbox.y, 20.0F);
    EXPECT_FLOAT_EQ(object.bbox.width, 30.0F);
    EXPECT_FLOAT_EQ(object.bbox.height, 40.0F);
}

TEST_F(ObjectTrackingAdapterTest, createDifferentTracksForDifferentClasses)
{
    const auto timestamp = 1000ULL;
    const auto pose = makePose(0.0, 0.0, 0.0, timestamp);

    const auto detection_result = makeDetectionResult(
        1,
        timestamp,
        {
            makeDetection(vp::domain::model::ClassId::CAR, 0.91F, 100.0F, 80.0F, 50.0F, 30.0F),
            makeDetection(vp::domain::model::ClassId::PERSON, 0.89F, 200.0F, 120.0F, 20.0F, 45.0F),
        });

    const auto tracking_result = adapter->update(detection_result, pose, timestamp);

    ASSERT_EQ(tracking_result.objects.size(), 2U);

    EXPECT_NE(tracking_result.objects[0].track_id, tracking_result.objects[1].track_id);
    EXPECT_NE(tracking_result.objects[0].class_id, tracking_result.objects[1].class_id);
}
} // namespace vp::adapter::out