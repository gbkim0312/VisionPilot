#include "gaia_dir.hpp"
#include "yolov8_adapter.hpp"
#include <fmt/core.h>
#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>

namespace vp::adapter::out
{
class YOLOv8AdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const std::string project_root = "/home/gbkim/project/VisionPilot";
        const std::string model_path = vp::joinDir(project_root, "vision_pilot/res/etc/yolov8n.onnx");
        config_.modelPath = model_path;
        config_.confThreshold = 0.25f;
        config_.nmsThreshold = 0.45f;
        config_.inputWidth = 640;
        config_.inputHeight = 640;
        config_.useCuda = false;

        adapter_ = std::make_unique<YOLOv8Adapter>(config_);
    }
    void TearDown() override
    {
    }

    config::YoloConfig config_;
    std::unique_ptr<YOLOv8Adapter> adapter_;
};

TEST_F(YOLOv8AdapterTest, ShouldInitializeSuccessfully)
{
    EXPECT_TRUE(adapter_->initialize());
}

TEST_F(YOLOv8AdapterTest, ShouldDetectObjectsInSampleImage)
{
    EXPECT_TRUE(adapter_->initialize());

    std::string project_root = "/home/gbkim/project/VisionPilot";
    std::string image_path = joinDir(project_root, "vision_pilot/res/etc/sample.png");
    ASSERT_TRUE(isFileExist(image_path)) << "Test image not found: " << image_path;

    cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);

    domain::model::ImagePacket image_packet;
    image_packet.timestamp = 0;
    image_packet.encoding = domain::model::ImageEncoding::MONO8;
    image_packet.format = domain::model::ImageFormat::MONO;
    image_packet.frame_id = 1;

    domain::model::MonoImagePacket payload;
    payload.frame.channels = 1;
    payload.frame.width = img.cols;
    payload.frame.height = img.rows;
    payload.frame.step = static_cast<int>(img.step);
    payload.frame.data.assign(img.data, img.data + (img.total() * img.elemSize()));
    image_packet.payload = payload;

    auto detections = adapter_->detectObject(image_packet);

    EXPECT_FALSE(detections.empty()) << "No objects detected in the sample image.";
    for (const auto &det : detections)
    {
        fmt::print("Detected class_id: {} ({}), confidence: {:.2f}, bbox: x={}, y={}, w={}, h={}", static_cast<uint64_t>(det.class_id), domain::model::ClassIdHelper::toString(det.class_id), det.confidence, det.bbox.x, det.bbox.y, det.bbox.width, det.bbox.height);
    }
}
} // namespace vp::adapter::out