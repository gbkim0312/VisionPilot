#include "gaia_dir.hpp"
#include "opencv_viewer_adapter.hpp"
#include "stereo_vslam_adapter.hpp"
#include "vision_pilot_service.hpp"
#include "vslam_config.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>

namespace vp::service
{
class VisionPilotServiceStereoTest : public ::testing::Test
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
        vslam_config_.vslamConfigFilePath = vp::joinDir(stella_config_dir, "KITTI_stereo_00-02.yaml");

        // 빌드 환경에 따른 보캐뷸러리 경로
        vslam_config_.vocabPath = vp::joinDir(project_root, "vision_pilot/res/orb_vocab.fbow");

        ASSERT_TRUE(vp::isFileExist(vslam_config_.vslamConfigFilePath)) << "VSLAM config file not found: " << vslam_config_.vslamConfigFilePath;
        ASSERT_TRUE(vp::isFileExist(vslam_config_.vocabPath)) << "Vocab file not found: " << vslam_config_.vocabPath;

        // 포트 초기화
        localization_adapter_ = std::make_unique<adapter::out::StereoVSlamAdapter>(vslam_config_);
        visualization_adapter_ = std::make_unique<adapter::out::OpenCVViewerAdapter>(vslam_viewer_config_);
        // localization_adapter_->initialize();

        // VisionPilotService 초기화
        vision_pilot_service_ = std::make_unique<VisionPilotService>(*localization_adapter_, *visualization_adapter_);

        //  유틸리티를 사용한 이미지 파일 목록 읽기
        std::string image0_dir = vp::joinDir(kitti_base, "image_0");
        ASSERT_TRUE(vp::isDirExist(image0_dir)) << "KITTI image directory not found: " << image0_dir;
        std::string image1_dir = vp::joinDir(kitti_base, "image_1");
        ASSERT_TRUE(vp::isDirExist(image1_dir)) << "KITTI image directory not found: " << image1_dir;

        vp::readDirFiles(image0_dir, image0_filenames_, true);
        for (auto &name : image0_filenames_)
        {
            image0_full_paths_.push_back(vp::joinDir(image0_dir, name));
        }

        vp::readDirFiles(image1_dir, image1_filenames_, true);
        for (auto &name : image1_filenames_)
        {
            image1_full_paths_.push_back(vp::joinDir(image1_dir, name));
        }

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
        vision_pilot_service_.reset();
        // 정리 코드
    }

    domain::model::ImagePacket createPacket(const cv::Mat &left, const cv::Mat &right, uint64_t frame_id, uint64_t ts)
    {
        domain::model::ImagePacket packet;
        packet.format = domain::model::ImageFormat::STEREO;
        packet.encoding = left.channels() == 3 ? domain::model::ImageEncoding::BGR8 : domain::model::ImageEncoding::MONO8;
        packet.frame_id = frame_id;
        packet.timestamp = ts;

        domain::model::StereoImagePacket stereo_payload;
        auto &left_frame = stereo_payload.left;
        auto &right_frame = stereo_payload.right;

        left_frame.width = left.cols;
        left_frame.height = left.rows;
        left_frame.channels = left.channels();
        left_frame.step = static_cast<int>(left.step);
        left_frame.data.assign(left.data, left.data + (left.total() * left.elemSize())); // NOLINT: OPENCV
        right_frame.width = right.cols;
        right_frame.height = right.rows;
        right_frame.channels = right.channels();
        right_frame.step = static_cast<int>(right.step);
        right_frame.data.assign(right.data, right.data + (right.total() * right.elemSize())); // NOLINT: OPENCV

        packet.payload = stereo_payload;

        return packet;
    }

    config::VslamAdapterConfig vslam_config_;
    config::VslamViewerConfig vslam_viewer_config_;

    std::vector<std::string> image0_filenames_;
    std::vector<std::string> image1_filenames_;
    std::vector<std::string> image0_full_paths_;
    std::vector<std::string> image1_full_paths_;
    std::vector<uint64_t> timestamps_;

    std::unique_ptr<adapter::out::StereoVSlamAdapter> localization_adapter_ = nullptr;
    std::unique_ptr<adapter::out::OpenCVViewerAdapter> visualization_adapter_ = nullptr;

    std::unique_ptr<VisionPilotService> vision_pilot_service_ = nullptr;
};

TEST_F(VisionPilotServiceStereoTest, InitializationTest)
{
    ASSERT_NE(vision_pilot_service_, nullptr);
}

TEST_F(VisionPilotServiceStereoTest, RunServiceTest)
{
    localization_adapter_->initialize();
    visualization_adapter_->start();

    int test_frame_count = image0_full_paths_.size();

    for (int i = 0; i < test_frame_count; ++i)
    {
        cv::Mat left_img = cv::imread(image0_full_paths_[i], cv::IMREAD_GRAYSCALE);  // NOLINT: OPENCV
        cv::Mat right_img = cv::imread(image1_full_paths_[i], cv::IMREAD_GRAYSCALE); // NOLINT: OPENCV
        auto packet = createPacket(left_img, right_img, i, timestamps_[i]);
        vision_pilot_service_->onFrameReceived(packet);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
} // namespace vp::service
