#include <gtest/gtest.h>

#include "pangolin_viewer_adapter.hpp"
#include "vslam_config.hpp"
#include <cstdlib>
#include <thread>

namespace vp::adapter::out
{

class PangolinViewerAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // headless 환경이면 스킵
        if (std::getenv("DISPLAY") == nullptr)
        {
            GTEST_SKIP() << "DISPLAY not set. Skipping Pangolin viewer test.";
        }
    }
};

TEST_F(PangolinViewerAdapterTest, StartAndStopDoesNotCrash)
{
    config::VslamViewerConfig cfg{};
    cfg.viewerType = config::VslamViewerType::PANGOLIN;

    PangolinViewerAdapter viewer(cfg);

    EXPECT_TRUE(viewer.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // 바로 stop (렌더 루프 X)
    EXPECT_TRUE(viewer.stop());
}

} // namespace vp::adapter::out
