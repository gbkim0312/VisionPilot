#pragma once
#include "video_loader.hpp"
#include <deque>
#include <opencv2/videoio.hpp>

namespace vp::adapter::in
{

struct FrameCache
{
    cv::Mat image;
    // 필요하다면 여기에 파일명에서 파싱한 timestamp 등을 추가 가능
};

class MonoVideoLoader : public VideoLoader
{
public:
    using VideoLoader::VideoLoader;
    ~MonoVideoLoader();

protected:
    bool initialize() override;
    bool fetchFrame() override;
    void release() override;

private:
    cv::VideoCapture video_capture_;

    std::vector<std::string> image_files_;
    size_t disk_read_index_ = 0; // 디스크에서 읽을 차례

    std::deque<FrameCache> frame_buffer_;

    std::mutex buffer_mutex_;
    std::condition_variable buffer_cv_; // 버퍼 상태 대기용

    std::thread prefetch_thread_;
    std::atomic_bool prefetch_running_{false};
    bool is_eof_ = false; // 모든 파일을 다 읽었는지 여부

    bool releaseImpl();

    bool fetchFrameFromVideo();
    bool fetchFrameFromFrameSet();

    bool scanDirectoryFiles();
    void prefetchLoop();
    void stopPrefetch();

    std::shared_ptr<domain::model::ImagePacket> createImagePacket(const cv::Mat &image);
};
} // namespace vp::adapter::in
