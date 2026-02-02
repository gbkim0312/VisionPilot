#include "yolov8_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "yolov8_adapter_impl.hpp"

namespace vp::adapter::out
{
YOLOv8Adapter::YOLOv8Adapter(const config::DetectionConfig &config)
{
    LOG_TRA("");

    const auto *yolov8_config = std::get_if<config::YoloConfig>(&config.modelConfig);

    if (yolov8_config == nullptr)
    {
        THROWLOG(SysException, "YOLOv8Adapter requires YoloConfig in DetectionConfig.");
    }
    impl_ = std::make_unique<YOLOv8AdapterImpl>(*yolov8_config);
}

YOLOv8Adapter::~YOLOv8Adapter()
{
    LOG_TRA("");
}

bool YOLOv8Adapter::initialize()
{
    return impl_->initialize();
}

std::vector<vp::domain::model::Detection> YOLOv8Adapter::detectObject(const vp::domain::model::ImagePacket &image)
{
    return impl_->detectObject(image);
}

bool YOLOv8Adapter::deinitialize()
{
    return impl_->deinitialize();
}
} // namespace vp::adapter::out