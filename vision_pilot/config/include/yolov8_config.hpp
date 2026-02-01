#pragma once
#include <string>

namespace vp::config
{
struct YoloConfig
{
    std::string modelPath;
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;
    int inputWidth = 640;
    int inputHeight = 640;
    bool useCuda = false; // GPU 사용 여부
};
} // namespace vp::config