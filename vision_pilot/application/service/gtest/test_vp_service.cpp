#include "gaia_dir.hpp"
#include "mono_vslam_adapter.hpp"
#include "opencv_viewer_adapter.hpp"
#include "vision_pilot_service.hpp"
#include "vslam_config.hpp"
#include "yolov8_adapter.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>

namespace vp::service
{
class VisionPilotServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 1. 기본 경로 설정
        const std::string project_root = "/home/gbkim/project/VisionPilot";
        const std::string kitti_base = vp::joinDir(project_root, "vision_pilot/res/dataset/kitti/dataset/sequences/00");
        const std::string stella_config_dir = vp::joinDir(project_root, "thirdparty/stella_vslam/example/kitti");

        // 2. VslamAdapter 설정 채우기
        vslam_config_.method = config::VslamMethod::STELLA_VSLAM;
        vslam_config_.vslamConfigFilePath = vp::joinDir(stella_config_dir, "KITTI_mono_00-02.yaml");

        // 3. YOLOv8 Adapter 설정 채우기
        std::string model_path = vp::joinDir(project_root, "vision_pilot/res/etc/yolov8n.onnx");
        yolo_config_.modelPath = model_path;
        yolo_config_.confThreshold = 0.25f;
        yolo_config_.nmsThreshold = 0.45f;
        yolo_config_.inputWidth = 640;
        yolo_config_.inputHeight = 640;
        yolo_config_.useCuda = false;

        // 빌드 환경에 따른 보캐뷸러리 경로
        vslam_config_.vocabPath = vp::joinDir(project_root, "vision_pilot/res/orb_vocab.fbow");

        ASSERT_TRUE(vp::isFileExist(vslam_config_.vslamConfigFilePath)) << "VSLAM config file not found: " << vslam_config_.vslamConfigFilePath;
        ASSERT_TRUE(vp::isFileExist(vslam_config_.vocabPath)) << "Vocab file not found: " << vslam_config_.vocabPath;

        // 포트 초기화
        localization_adapter_ = std::make_unique<adapter::out::MonoVSlamAdapter>(vslam_config_);
        visualization_adapter_ = std::make_unique<adapter::out::OpenCVViewerAdapter>(vslam_viewer_config_);
        object_detection_adapter_ = std::make_unique<adapter::out::YOLOv8Adapter>(yolo_config_);

        vision_pilot_service_ = std::make_unique<VisionPilotService>(*localization_adapter_, *visualization_adapter_, *object_detection_adapter_);

        //  유틸리티를 사용한 이미지 파일 목록 읽기
        std::string image_dir = vp::joinDir(kitti_base, "image_0");
        ASSERT_TRUE(vp::isDirExist(image_dir)) << "KITTI image directory not found: " << image_dir;

        // readDirFiles는 결과를 대소문자 무시하고 정렬해주므로 별도 sort 불필요
        vp::readDirFiles(image_dir, image_filenames_, true);
        for (auto &name : image_filenames_)
        {
            image_full_paths_.push_back(vp::joinDir(image_dir, name));
        }

        // 5. vp::readLines를 사용한 타임스탬프 로드
        std::vector<std::string> ts_lines;
        std::string ts_file = vp::joinDir(kitti_base, "times.txt");
        ASSERT_TRUE(vp::isFileExist(ts_file)) << "times.txt not found!";

        vp::readLines(ts_file, ts_lines);
        for (const auto &line : ts_lines)
        {
            if (!line.empty())
            {
                // KITTI times.txt는 초(sec) 단위 double 값임
                timestamps_.push_back(static_cast<uint64_t>(std::stod(line) * 1e6));
            }
        }
    }

    void TearDown() override
    {
        localization_adapter_.reset();
        visualization_adapter_.reset();
        object_detection_adapter_.reset();
        vision_pilot_service_.reset();
        // 정리 코드
    }

    domain::model::ImagePacket createPacket(const cv::Mat &frame, uint64_t frame_id, uint64_t ts)
    {
        domain::model::ImagePacket packet;
        packet.format = domain::model::ImageFormat::MONO;
        packet.encoding = frame.channels() == 3 ? domain::model::ImageEncoding::BGR8 : domain::model::ImageEncoding::MONO8;
        packet.frame_id = frame_id;
        packet.timestamp = ts;

        domain::model::MonoImagePacket mono_payload;
        mono_payload.frame.width = frame.cols;
        mono_payload.frame.height = frame.rows;
        mono_payload.frame.channels = frame.channels();
        mono_payload.frame.step = static_cast<int>(frame.step);
        mono_payload.frame.data.assign(frame.data, frame.data + (frame.total() * frame.elemSize())); // NOLINT: OPENCV
        packet.payload = mono_payload;

        return packet;
    }

    config::VslamAdapterConfig vslam_config_;
    config::VslamViewerConfig vslam_viewer_config_;
    config::YoloConfig yolo_config_;

    std::vector<std::string> image_filenames_;
    std::vector<std::string> image_full_paths_;
    std::vector<uint64_t> timestamps_;

    std::unique_ptr<adapter::out::MonoVSlamAdapter> localization_adapter_ = nullptr;
    std::unique_ptr<adapter::out::OpenCVViewerAdapter> visualization_adapter_ = nullptr;
    std::unique_ptr<adapter::out::YOLOv8Adapter> object_detection_adapter_ = nullptr;

    std::unique_ptr<VisionPilotService> vision_pilot_service_ = nullptr;
};

TEST_F(VisionPilotServiceTest, InitializationTest)
{
    ASSERT_NE(vision_pilot_service_, nullptr);
}

TEST_F(VisionPilotServiceTest, RunServiceTest)
{
    localization_adapter_->initialize();
    object_detection_adapter_->initialize();
    visualization_adapter_->start();

    constexpr size_t expected_count = 100;
    int test_frame_count = std::min(image_full_paths_.size(), expected_count);

    // 목표 주기 설정 (예: 100ms = 10Hz)
    auto interval = std::chrono::milliseconds(100);

    // 시작 시간 기준점 잡기
    auto next_frame_time = std::chrono::steady_clock::now();

    for (int i = 0; i < test_frame_count; ++i)
    {
        // 다음 실행 예정 시간 계산 (현재 기준점 + 주기)
        next_frame_time += interval;

        cv::Mat img = cv::imread(image_full_paths_[i], cv::IMREAD_GRAYSCALE); // NOLINT
        auto packet = createPacket(img, i, timestamps_[i]);

        vision_pilot_service_->onFrameReceived(packet);

        // 예정된 시간까지 대기 (코드 실행 시간이 interval보다 짧으면 남은 만큼만 대기)
        std::this_thread::sleep_until(next_frame_time);
    }
}
} // namespace vp::service
