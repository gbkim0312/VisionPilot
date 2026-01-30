#include "event_router.hpp"
#include "video_loader.hpp"
#include "video_loader_config.hpp"
#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

namespace vp::adapter::in::frame_loader
{

class MockFrameReceivePort : public vp::port::in::FrameReceiveUseCase
{
public:
    MOCK_METHOD(void, onFrameReceived, (const domain::model::ImagePacket &frame), (override));
};

class VideoLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        config_.source = "etc/sample.mp4";
        config_.fps = 30;
        config_.sourceType = config::SourceType::VIDEO_FILE;

        event_queue_ = std::make_unique<infrastructure::event::EventQueue>();
        event_router_ = std::make_unique<infrastructure::event::EventRouter>(*event_queue_, mock_receive_port_);
        event_router_->start();
        loader_ = std::make_unique<VideoLoader>(config_, *event_queue_);
    }

    void TearDown() override
    {
        event_router_->stop();
        loader_->stop();
    }

    std::unique_ptr<infrastructure::event::EventQueue> event_queue_;
    std::unique_ptr<infrastructure::event::EventRouter> event_router_;
    testing::NiceMock<class MockFrameReceivePort> mock_receive_port_;
    config::VideoLoaderConfig config_;
    std::unique_ptr<VideoLoader> loader_;
};

TEST_F(VideoLoaderTest, StartAndStop)
{
    EXPECT_TRUE(loader_->start());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(loader_->stop());
}

TEST_F(VideoLoaderTest, FrameReception)
{
    EXPECT_CALL(mock_receive_port_, onFrameReceived(testing::_)).Times(testing::AtLeast(1));
    EXPECT_TRUE(loader_->start());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(loader_->stop());
}

TEST_F(VideoLoaderTest, FrameReceptionWithDisplay)
{
    // Mock 객체가 호출될 때 실제 동작(이미지 출력)을 정의
    EXPECT_CALL(mock_receive_port_, onFrameReceived(testing::_))
        .WillRepeatedly(testing::Invoke([](const domain::model::ImagePacket &packet)
                                        {
            if (packet.data.empty()){ return;}

            cv::Mat frame(packet.height,
                          packet.width,
                          packet.channels == 3 ? CV_8UC3 : CV_8UC1, //NOLINT: OPENCV
                          const_cast<void*>(static_cast<const void*>(packet.data.data())));

            if (!frame.empty()) {
                cv::imshow("Test Debug Display", frame);
                cv::waitKey(1); // 창을 갱신하기 위해 필수
            } }));

    EXPECT_TRUE(loader_->start());

    // 영상이 나오는 것을 확인하기 위해 충분한 시간 대기
    std::this_thread::sleep_for(std::chrono::seconds(5));

    EXPECT_TRUE(loader_->stop());
    cv::destroyAllWindows();
}
} // namespace vp::adapter::in::frame_loader