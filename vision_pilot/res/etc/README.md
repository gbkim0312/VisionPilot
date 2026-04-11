# vp_config Guide

`res/etc` 폴더에는 VisionPilot 실행에 사용하는 샘플 설정 파일이 들어 있습니다.

- `vp_config_mono.json`: mono 입력 + Stella VSLAM + YOLOv8 + OpenCV Viewer
- `vp_config_stereo.json`: stereo 입력 + Stella VSLAM + YOLOv8 + OpenCV Viewer
- `vp_config_mono_orbslam.json`: ORB-SLAM3 기반 실험용 예시

실행할 때는 보통 아래처럼 `-c`로 설정 파일을 직접 지정합니다.

```bash
./build/debug/vision_pilot/bin/vp -c vision_pilot/res/etc/vp_config_mono.json info
```

## 상위 구조

설정은 크게 아래 4개 블록으로 구성됩니다.

- `videoLoaderConfig`
- `vslamAdapterConfig`
- `viewerConfig`
- `detectionConfig`

이 네 항목이 입력 방식, 위치 추정기, 시각화 방식, 객체 검출기를 각각 결정합니다.

## 1. videoLoaderConfig

입력 데이터의 종류와 형식을 정합니다.

주요 키:

- `dataType`: `videoFile`, `frameSet`, `cameraDevice`, `rtspStream`
- `cameraFormat`: `mono`, `stereo`, `rgbd`
- `fps`: 입력 재생 속도
- `frameSize`: 강제 리사이즈 크기, `0`이면 원본 유지
- `mono` / `stereo` / `rgbd`: 포맷별 상세 소스 경로

영향:

- `dataType`이 `frameSet`이면 디렉토리의 이미지들을 순서대로 읽습니다.
- `cameraFormat`이 `mono`인지 `stereo`인지에 따라 다른 로더가 생성됩니다.
- `fps`가 낮으면 전체 파이프라인 처리 주기도 낮아집니다.
- `frameSize`를 줄이면 처리량은 좋아질 수 있지만, 검출과 추적 품질은 떨어질 수 있습니다.
- `mono.source`, `stereo.leftSource`, `stereo.rightSource`는 실행 환경에 맞는 실제 경로로 바꿔야 합니다.

## 2. vslamAdapterConfig

위치 추정 백엔드와 저장 옵션을 정합니다.

주요 키:

- `type`: `stellaVslam`, `orbSlam3`, `none`
- `method`: `monocular`, `stereo`, `rgbd`
- `vslamConfigFilePath`: SLAM 알고리즘 설정 파일 경로
- `vocabPath`: vocabulary 파일 경로
- `useInternalViewer`: SLAM 내부 viewer 사용 여부
- `frameQueueSize`: SLAM 입력 큐 크기
- `loadConfig.loadMapDatabase`: 기존 map database 로드 여부
- `saveConfig`: trajectory, map database 저장 설정

영향:

- `type`을 `none`으로 두면 포즈 추정이 비활성화됩니다.
- `method`는 입력 포맷과 맞아야 합니다. 예를 들어 mono 입력에 stereo SLAM 설정을 주면 정상 동작을 기대하기 어렵습니다.
- `frameQueueSize`를 키우면 순간적인 입력 버스트에는 덜 민감하지만, 지연이 늘어날 수 있습니다.
- `useInternalViewer`를 켜면 Stella VSLAM 내부 viewer 초기화가 추가됩니다.
- `saveConfig`를 켜면 trajectory나 map database 파일이 생성됩니다.

실무적으로는 `vslamConfigFilePath`, `vocabPath`, `loadConfig.path`, `saveConfig.path`를 가장 먼저 확인하면 됩니다.

## 3. viewerConfig

외부 시각화 어댑터를 정합니다.

주요 키:

- `viewerType`: `none`, `opencv`, `pangolin`, `socket`
- `renderOption.renderDetection`: detection 박스와 라벨 표시 여부
- `renderOption.renderTracking`: tracking 박스, 상태, 속도 화살표, 트랙 요약 표시 여부

영향:

- `viewerType`이 `opencv`면 현재 가장 풍부한 2D 오버레이를 볼 수 있습니다.
- `viewerType`이 `none`이면 화면 출력 없이 로그 중심으로 실행됩니다.
- `renderOption`은 현재 OpenCV Viewer에만 적용됩니다.
- `renderDetection`을 끄면 detection 결과는 계산되더라도 노란 박스와 detection 라벨은 그리지 않습니다.
- `renderTracking`을 끄면 tracking 결과는 계산되더라도 track ID, 상태, 속도 화살표, track summary는 그리지 않습니다.

예시:

```json
"viewerConfig": {
  "viewerType": "opencv",
  "renderOption": {
    "renderDetection": true,
    "renderTracking": false
  }
}
```

## 4. detectionConfig

객체 검출기를 정합니다.

주요 키:

- `type`: `yolov8`, `none`
- `yolov8.modelPath`: ONNX 모델 경로
- `yolov8.confThreshold`: confidence threshold
- `yolov8.nmsThreshold`: NMS threshold
- `yolov8.inputWidth`, `yolov8.inputHeight`: 입력 해상도
- `yolov8.frameQueueSize`: detection 입력 큐 크기
- `yolov8.useCuda`: CUDA 사용 여부

영향:

- `type`이 `none`이면 detection과 이후 tracking 입력이 사실상 비활성화됩니다.
- `confThreshold`를 높이면 오검출은 줄지만 놓치는 객체는 늘 수 있습니다.
- `nmsThreshold`는 겹치는 박스를 얼마나 적극적으로 제거할지 결정합니다.
- `inputWidth`, `inputHeight`가 커지면 작은 객체 검출에는 유리할 수 있지만 속도는 느려질 수 있습니다.
- `useCuda`를 켜면 GPU 환경에서는 추론 속도 개선을 기대할 수 있습니다.
- `frameQueueSize`를 키우면 프레임 유실은 줄 수 있지만 결과가 늦게 보일 수 있습니다.

## 먼저 볼 항목

설정이 기대대로 안 움직일 때는 아래 순서로 확인하는 편이 빠릅니다.

1. 입력 경로가 현재 머신에 맞는지 확인
2. `cameraFormat`과 `vslamAdapterConfig.method`가 서로 맞는지 확인
3. `viewerConfig.viewerType`이 원하는 출력 방식인지 확인
4. `detectionConfig.type`과 `yolov8.modelPath`가 유효한지 확인
5. 필요 이상으로 큰 `frameQueueSize`를 주지 않았는지 확인

## 주의사항

- 샘플 설정의 데이터 경로는 로컬 절대 경로이므로 그대로는 다른 환경에서 동작하지 않을 수 있습니다.
- `vp_config_mono_orbslam.json`은 현재 코드가 기대하는 `viewerConfig` 대신 `vslamViewerConfig` 키를 사용하고 있어 바로 호환되지 않을 수 있습니다.
- `renderOption`은 현재 OpenCV Viewer 전용입니다.