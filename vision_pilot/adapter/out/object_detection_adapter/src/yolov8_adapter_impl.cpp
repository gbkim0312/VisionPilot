#include "yolov8_adapter_impl.hpp"
#include "gaia_log.hpp"
#include <fstream>
#include <opencv2/core/mat.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>

namespace
{
cv::Mat addPadding(const cv::Mat &resized_img)
{
    LOG_TRA("");

    int new_height = resized_img.rows;

    if (new_height < 640)
    {
        int padding = (640 - new_height) / 2;
        cv::Mat padded_img;
        cv::copyMakeBorder(resized_img, padded_img, padding, 640 - new_height - padding, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
        return padded_img;
    }

    return resized_img;
}

cv::Mat resizeImage(const cv::Mat &input_img, int target_width)
{
    LOG_TRA("");

    int original_width = input_img.cols;
    int original_height = input_img.rows;

    float scale = static_cast<float>(target_width) / static_cast<float>(original_width);
    int new_height = static_cast<int>(original_height * scale);

    cv::Mat resized_img;
    cv::resize(input_img, resized_img, cv::Size(target_width, new_height));

    return resized_img;
}
} // namespace

namespace vp::adapter::out
{

YOLOv8AdapterImpl::YOLOv8AdapterImpl(const config::YoloConfig &config)
    : config_(config)
{
    LOG_TRA("");
    this->initialize();
}

YOLOv8AdapterImpl::~YOLOv8AdapterImpl()
{
    LOG_TRA("");
    this->deinitialize();
}

bool YOLOv8AdapterImpl::initialize()
{
    LOG_INF("Initializing YOLOv8 Adapter...");

    if (is_initialized_)
    {
        LOG_DBG("YOLOv8 Adapter is already initialized.");
        return true;
    }
    // 모델 파일 존재 확인
    std::ifstream f(config_.modelPath.c_str());
    if (!f.good())
    {
        LOG_ERR("YOLOv8 model file not found: {}", config_.modelPath);
        return false;
    }

    try
    {
        net_ = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromONNX(config_.modelPath));

        if (config_.useCuda)
        {
            LOG_INF("Trying to use CUDA backend...");
            net_->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            net_->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        }
        else
        {
            LOG_INF("Using CPU backend...");
            net_->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net_->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        }

        // 워밍업 (선택사항: 첫 추론 속도 향상)
        // cv::Mat dummy = cv::Mat::zeros(config_.inputHeight, config_.inputWidth, CV_32FC3);
        // cv::dnn::blobFromImage(dummy); // ...

        LOG_INF("YOLOv8 initialized successfully.");
        is_initialized_ = true;
        return true;
    }
    catch (const cv::Exception &e)
    {
        LOG_ERR("Failed to load YOLOv8 network: {}", e.what());
        return false;
    }
}

bool YOLOv8AdapterImpl::deinitialize()
{
    LOG_TRA("");
    if (!is_initialized_)
    {
        LOG_DBG("YOLOv8 Adapter is not initialized.");
        return true;
    }
    net_.reset();
    is_initialized_ = false;
    return true;
}

std::vector<vp::domain::model::Detection> YOLOv8AdapterImpl::detectObject(const vp::domain::model::ImagePacket &packet)
{
    if (!is_initialized_ || net_ == nullptr || net_->empty())
    {
        LOG_ERR("Network not initialized.");
        return {};
    }

    // 1. ImagePacket에서 cv::Mat 추출
    const vp::domain::model::RawImage *raw_ptr = nullptr;
    std::visit([&](auto &&arg)
               {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, vp::domain::model::MonoImagePacket>)
        {
            raw_ptr = &arg.frame;
        }
        else if constexpr (std::is_same_v<T, vp::domain::model::StereoImagePacket>)
        {
            raw_ptr = &arg.left;
        } }, packet.payload);

    if (!raw_ptr || raw_ptr->data.empty())
    {
        return {};
    }

    // Raw -> Mat 변환
    int type = (raw_ptr->channels == 3) ? CV_8UC3 : CV_8UC1;
    cv::Mat frame(raw_ptr->height, raw_ptr->width, type, const_cast<uint8_t *>(raw_ptr->data.data()), raw_ptr->step);

    // RGB 변환
    cv::Mat rgb_frame;
    if (packet.encoding == vp::domain::model::ImageEncoding::BGR8)
    {
        cv::cvtColor(frame, rgb_frame, cv::COLOR_BGR2RGB);
    }
    else if (packet.encoding == vp::domain::model::ImageEncoding::MONO8)
    {
        cv::cvtColor(frame, rgb_frame, cv::COLOR_GRAY2RGB);
    }
    else
    {
        rgb_frame = frame;
    }

    // 2. Pre-processing: Letterbox
    int img_w = rgb_frame.cols;
    int img_h = rgb_frame.rows;
    int target_w = config_.inputWidth;
    int target_h = config_.inputHeight;

    // 스케일 계산
    float scale = std::min((float)target_w / img_w, (float)target_h / img_h);

    int new_w = std::round(img_w * scale);
    int new_h = std::round(img_h * scale);

    // 리사이즈 수행
    cv::Mat resized_img;
    cv::resize(rgb_frame, resized_img, cv::Size(new_w, new_h));

    // 패딩 계산 (중앙 정렬)
    int dw = (target_w - new_w) / 2;
    int dh = (target_h - new_h) / 2;

    // 캔버스 생성
    cv::Mat input_blob_img(target_h, target_w, CV_8UC3, cv::Scalar(114, 114, 114));

    // 리사이즈된 이미지를 캔버스 중앙에 복사
    resized_img.copyTo(input_blob_img(cv::Rect(dw, dh, new_w, new_h)));

    // 640x640 이미지 생성 완료. 이제 blobFromImage는 리사이즈 없이 값 정규화만 수행
    cv::Mat blob;
    cv::dnn::blobFromImage(input_blob_img, blob, 1.0 / 255.0, cv::Size(), cv::Scalar(), false, false);

    net_->setInput(blob);

    // 3. Inference
    std::vector<cv::Mat> outputs;
    net_->forward(outputs, net_->getUnconnectedOutLayersNames());

    // 4. Post-processing
    cv::Mat &output = outputs[0];

    // YOLOv8 Output: [Batch, 4+Classes, Anchors] -> [1, 84, 8400]
    int dimensions = output.size[1];
    int rows = output.size[2];
    float *pdata = (float *)output.data;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int r = 0; r < rows; ++r)
    {
        float max_conf = 0.f;
        int max_class_id = -1;

        // Class Score 파싱
        for (int c = 4; c < dimensions; ++c)
        {
            float score = pdata[c * rows + r];
            if (score > max_conf)
            {
                max_conf = score;
                max_class_id = c - 4;
            }
        }

        if (max_conf >= config_.confThreshold)
        {
            // 예측 좌표 (640x640 Letterbox 기준)
            float cx = pdata[0 * rows + r];
            float cy = pdata[1 * rows + r];
            float w = pdata[2 * rows + r];
            float h = pdata[3 * rows + r];

            // =========================================================
            // 좌표 복원 (Coordinate Restoration)
            // =========================================================
            // 1. 패딩 제거 (Letterbox 좌표 -> 리사이즈 이미지 좌표)
            float original_cx = (cx - dw);
            float original_cy = (cy - dh);

            // 2. 스케일 역변환 (리사이즈 이미지 좌표 -> 원본 이미지 좌표)
            original_cx /= scale;
            original_cy /= scale;
            float original_w = w / scale;
            float original_h = h / scale;

            // 3. Top-Left 변환
            int left = static_cast<int>(original_cx - 0.5f * original_w);
            int top = static_cast<int>(original_cy - 0.5f * original_h);
            int width = static_cast<int>(original_w);
            int height = static_cast<int>(original_h);

            // 경계값 처리 (이미지 밖으로 나가는 경우 방지)
            left = std::max(0, std::min(left, img_w - 1));
            top = std::max(0, std::min(top, img_h - 1));
            width = std::max(1, std::min(width, img_w - left));
            height = std::max(1, std::min(height, img_h - top));

            boxes.push_back(cv::Rect(left, top, width, height));
            confidences.push_back(max_conf);
            class_ids.push_back(max_class_id);
        }
    }

    // 5. NMS
    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, config_.confThreshold, config_.nmsThreshold, nms_result);

    // 6. 결과 변환
    std::vector<vp::domain::model::Detection> detections;
    for (int idx : nms_result)
    {
        vp::domain::model::Detection det;
        det.class_id = static_cast<vp::domain::model::ClassId>(class_ids[idx]);
        det.confidence = confidences[idx];
        det.bbox.x = static_cast<float>(boxes[idx].x);
        det.bbox.y = static_cast<float>(boxes[idx].y);
        det.bbox.width = static_cast<float>(boxes[idx].width);
        det.bbox.height = static_cast<float>(boxes[idx].height);
        detections.push_back(det);
    }

    return detections;
}

} // namespace vp::adapter::out