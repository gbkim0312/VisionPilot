#include "event_router.hpp"
#include "video_loader.hpp"
#include "video_loader_config.hpp"
#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace vp::adapter::in::frame_loader
{

class MockFrameReceivePort : public vp::port::in::FrameReceiveUseCase
{
public:
    MOCK_METHOD(void, onFrameReceived, (std::shared_ptr<domain::model::ImagePacket> frame), (override));
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

} // namespace vp::adapter::in::frame_loader