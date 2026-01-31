#include "gaia_dir.hpp"
#include "gaia_log.hpp"
#include "image.hpp"
#include "mono_vslam_adapter.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

namespace vp::adapter::out
{
class VslamAdapterTest : public ::testing::Test
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

        // 빌드 환경에 따른 보캐뷸러리 경로
        vslam_config_.vocabPath = vp::joinDir(project_root, "vision_pilot/res/orb_vocab.fbow");

        ASSERT_TRUE(vp::isFileExist(vslam_config_.vslamConfigFilePath)) << "VSLAM config file not found: " << vslam_config_.vslamConfigFilePath;
        ASSERT_TRUE(vp::isFileExist(vslam_config_.vocabPath)) << "Vocab file not found: " << vslam_config_.vocabPath;

        // 3. 어댑터 초기화 및 검증
        vslam_adapter_ = std::make_unique<MonoVSlamAdapter>(vslam_config_);
        ASSERT_TRUE(vslam_adapter_->initialize()) << "VSLAM Adapter initialization failed!";

        // 4. vp 유틸리티를 사용한 이미지 파일 목록 읽기
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

    // OpenCV Mat -> ImagePacket 변환 헬퍼
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
    std::unique_ptr<MonoVSlamAdapter> vslam_adapter_;
    std::vector<std::string> image_filenames_;
    std::vector<std::string> image_full_paths_;
    std::vector<uint64_t> timestamps_;
};

// 초기화 성공 여부 확인
TEST_F(VslamAdapterTest, ShouldInitializeSuccessfully)
{
    EXPECT_NE(vslam_adapter_, nullptr);
}

// 데이터 로딩 정합성 확인
TEST_F(VslamAdapterTest, DatasetLoadingCheck)
{
    ASSERT_FALSE(image_full_paths_.empty());
    ASSERT_EQ(image_full_paths_.size(), timestamps_.size())
        << "Mismatch between image count and timestamp count!";
}

// 실제 프레임 입력 및 포즈 추론 테스트
TEST_F(VslamAdapterTest, ProcessKittiSequence)
{
    int test_frame_count = std::min(static_cast<int>(image_full_paths_.size()), 100);
    bool has_initialized = false;

    for (int i = 0; i < test_frame_count; ++i)
    {
        cv::Mat img = cv::imread(image_full_paths_[i], cv::IMREAD_UNCHANGED);
        auto packet = createPacket(img, i, timestamps_[i]);
        auto pose = vslam_adapter_->update(packet, packet.timestamp);

        if (!pose.is_lost)
        {
            has_initialized = true;
            LOG_INF("[Frame {}] Tracking Success! x={:.2f}, y={:.2f}", i, pose.x, pose.y);
            break;
        }
    }

    EXPECT_TRUE(has_initialized) << "VSLAM failed to initialize within 100 frames.";
}

} // namespace vp::adapter::out