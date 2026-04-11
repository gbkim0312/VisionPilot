# VisionPilot

> [!NOTE]
> 본 README 파일은 CODEX에 의해 자동 생성되었음을 안내 드립니다.

`VisionPilot`은 입력 영상으로부터 차량/카메라의 위치를 추정하고, 객체를 검출하며, 동일 객체를 프레임 간 추적하고, 그 결과를 시각화하는 C++ 기반 비전 파이프라인입니다.
이 문서는 `vision_pilot` 디렉토리만을 기준으로 정리했으며, 분석 범위에서는 `build`와 `thirdparty`를 제외했습니다.

이 프로젝트의 핵심은 다음 세 가지입니다.

- 입력, 서비스, 출력 어댑터를 분리한 모듈형 아키텍처
- `VSLAM + Detection + Tracking + Visualization`을 하나의 프레임 파이프라인으로 통합한 구조
- 추후 센서/모델 교체가 가능하도록 `port`와 `registry` 중심으로 조립한 확장 가능한 설계

## 1. 프로젝트가 하는 일

한 프레임이 시스템 안에서 처리되는 흐름은 아래와 같습니다.

```text
VideoLoader
  -> EventQueue
  -> EventRouter
  -> VisionPilotService
      -> LocalizationPort      (Stella VSLAM / ORB-SLAM3 / None)
      -> ObjectDetectionPort   (YOLOv8 / None)
      -> ObjectTrackingPort    (Kalman Filter 기반 Tracker)
      -> VisualizationPort     (OpenCV / Pangolin / Socket / None)
```

즉, 이 프로젝트는 단순히 객체를 검출하는 데서 끝나지 않고, 다음을 한 번에 수행합니다.

- 영상 입력 수집
- 카메라/차량 위치 추정
- 객체 검출
- 객체 ID 유지 및 속도 추정
- 시각화 또는 로그 출력

## 2. 한눈에 보는 구조

```text
vision_pilot
├── main.cpp                     # 프로그램 진입점
├── assembly/                    # 의존성 조립, 레지스트리, 서비스 시작/종료
├── application/
│   ├── domain/model/            # Pose, Detection, Tracking, Image, Event 등 핵심 모델
│   ├── port/                    # in/out 인터페이스
│   └── service/                 # 프레임 단위 유스케이스 오케스트레이션
├── adapter/
│   ├── in/frame_loader/         # mono/stereo 영상 입력
│   └── out/
│       ├── vslam_adapter/       # Stella VSLAM, ORB-SLAM3, None
│       ├── object_detection_adapter/   # YOLOv8, None
│       ├── object_tracking_adapter/    # Kalman Filter tracker
│       └── visualization_adapter/      # OpenCV, Pangolin, Socket, None
├── infrastructure/event/        # EventQueue, EventRouter
├── config/                      # JSON 설정 로더 및 설정 구조체
├── gaia/                        # 공용 유틸리티/자료구조/로깅
└── res/etc/                     # 샘플 설정 파일, 모델, vocabulary
```

## 3. 아키텍처 설명

### 3.1 Entry Point

[`main.cpp`](main.cpp)는 다음 역할만 담당합니다.

- CLI 인자 파싱
- 로그 레벨 초기화
- 설정 파일 로딩
- `Assembly` 생성 및 서비스 시작/종료

즉, 비즈니스 로직은 `main`에 거의 없고, 실제 동작은 모두 조립된 서비스와 어댑터가 담당합니다.

### 3.2 Assembly Layer

`assembly`는 이 프로젝트의 조립기입니다.

- `OutAdapterRegistry`
  - 설정에 따라 VSLAM, Detection, Viewer 구현체를 선택합니다.
  - 예: `stellaVslam`, `opencv`, `yolov8`
- `InAdapterRegistry`
  - 입력 어댑터인 `VideoLoader`를 생성합니다.
- `AssemblyImpl`
  - `VisionPilotService`를 생성하고
  - `EventRouter`, 입력 어댑터, 출력 어댑터를 함께 시작/중지합니다.

이 구조 덕분에 서비스 코드는 구체 구현체를 몰라도 되고, 설정만 바꾸면 조합을 바꿀 수 있습니다.

### 3.3 Application Layer

`application`은 프로젝트의 중심입니다.

- `domain/model`
  - 시스템이 주고받는 공통 데이터 모델
  - `ImagePacket`, `Pose`, `DetectionResult`, `TrackingResult`, `Event`
- `port/in`
  - 입력 유스케이스 인터페이스
  - 현재는 `FrameReceiveUseCase`
- `port/out`
  - 외부 기능 인터페이스
  - `LocalizationPort`, `ObjectDetectionPort`, `ObjectTrackingPort`, `VisualizationPort`
- `service`
  - 프레임이 들어왔을 때 실제 처리 순서를 정의

핵심 서비스는 `VisionPilotServiceImpl::onFrameReceived()`입니다.

```text
frame
  -> localization_port.update()
  -> object_detection_port.detectObject()
  -> object_tracking_port.update()
  -> visualization_port.render()
```

즉, 이 프로젝트의 실제 “유스케이스”는 서비스 계층에 응집되어 있습니다.

### 3.4 Infrastructure Layer

`infrastructure/event`는 실시간 파이프라인을 단순한 이벤트 기반 구조로 연결합니다.

- `EventQueue`
  - 고정 크기 큐
  - 큐가 가득 차면 가장 오래된 이벤트를 버려 실시간성을 유지
- `EventRouter`
  - 워커 스레드에서 이벤트를 소비
  - 현재는 `IMAGE` 이벤트를 받아 `FrameReceiveUseCase`로 전달

이 구조는 이후 IMU, CAN, LiDAR 이벤트를 추가하기 쉬운 형태입니다.

### 3.5 Adapter Layer

#### Input Adapter

`adapter/in/frame_loader`는 입력 프레임을 생성합니다.

- `MonoVideoLoader`
- `StereoVideoLoader`
- `VideoLoaderFactory`

지원 입력 형태:

- 비디오 파일
- 프레임 이미지 세트
- 카메라 디바이스
- RTSP 스트림

특징:

- `FRAME_SET` 모드에서는 prefetch thread로 디스크 I/O를 분리
- mono/stereo 모두 `ImagePacket`으로 통일해 상위 계층에 전달

#### Output Adapter

`adapter/out`은 외부 알고리즘/출력 기능을 담당합니다.

- `vslam_adapter`
  - `StellaVslamAdapter`: 현재 실질적으로 동작하는 VSLAM 구현
  - `OrbSlamAdapter`: 인터페이스는 존재하지만 아직 개발 중
  - `NoVslamAdapter`: 위치 추정 비활성화용
- `object_detection_adapter`
  - `YOLOv8Adapter`: OpenCV DNN 기반 ONNX 추론
  - `NoDetectionAdapter`: 검출 비활성화용
- `object_tracking_adapter`
  - Kalman Filter 기반 다중 객체 추적
- `visualization_adapter`
  - `OpenCVViewerAdapter`
  - `PangolinViewerAdapter`
  - `SocketViewerAdapter`
  - `NoneViewerAdapter`

참고로 viewer 구현 성숙도는 서로 다릅니다.

- `OpenCVViewerAdapter`: detection + tracking overlay까지 포함된 주력 시각화 경로
- `PangolinViewerAdapter`: pose/trajectory 중심 3D 시각화
- `SocketViewerAdapter`: 현재는 인터페이스 성격의 최소 구현
- `NoneViewerAdapter`: 로그 기반 확인용

## 4. 핵심 파이프라인 상세

### 4.1 입력

`VideoLoader`는 `ImagePacket`을 생성해서 `EventQueue`에 넣습니다.

- mono는 `MonoImagePacket`
- stereo는 `StereoImagePacket`
- 공통 메타데이터
  - `frame_id`
  - `timestamp`
  - `encoding`
  - `format`

### 4.2 위치 추정

`LocalizationPort`는 프레임을 받아 `Pose`를 반환합니다.

현재 기준 실사용 구현은 `StellaVslamAdapter`입니다.

- 모노/스테레오 입력 지원
- 내부 스레드에서 프레임 큐를 소비
- 필요 시 trajectory / keyframe trajectory / map database 저장 가능
- map database를 로드하면 localization-only 모드처럼 동작 가능

`Pose`는 아래 정보를 포함합니다.

- 위치: `x, y, z`
- 자세: quaternion `qw, qx, qy, qz`
- 추적 실패 여부: `is_lost`

### 4.3 객체 검출

`YOLOv8Adapter`는 OpenCV DNN으로 ONNX 모델을 읽어 객체를 검출합니다.

처리 흐름:

1. `ImagePacket`에서 `cv::Mat` 복원
2. RGB 변환
3. letterbox 전처리
4. ONNX 추론
5. confidence threshold 적용
6. NMS 수행
7. `DetectionResult`로 변환

출력 데이터는 `DetectionResult`이며, 각 객체는 다음 정보를 가집니다.

- `class_id`
- `confidence`
- `bbox(x, y, width, height)`

## 5. KF 및 트래킹

이 프로젝트의 트래킹은 `adapter/out/object_tracking_adapter`에 구현되어 있으며, README에서 별도 섹션으로 이해해야 할 만큼 독립적인 핵심 모듈입니다.

### 5.1 역할

트래커는 검출 결과를 단순 전달하지 않고, 프레임 간 동일 객체를 연결해 아래 정보를 만들어 냅니다.

- stable `track_id`
- 객체 상태(`NEW`, `TRACKED`, `LOST`, `REMOVED`)
- 속도 벡터 `velocity`
- 추적 나이 `tracking_age`
- 연속 미검출 수 `lost_count`

즉, Detection이 “이번 프레임에 무엇이 보이는가”를 말한다면, Tracking은 “지금 보이는 것이 이전 프레임의 어떤 객체와 같은가”를 답합니다.

### 5.2 상태 벡터와 측정 벡터

트래커는 6차원 상태를 사용합니다.

```text
state = [cx, cy, vx, vy, w, h]
```

- `cx, cy`: bbox 중심 좌표
- `vx, vy`: 이미지 평면 기준 속도
- `w, h`: bbox 크기

검출 결과는 4차원 측정값으로 변환됩니다.

```text
measurement = [cx, cy, w, h]
```

즉, 검출기는 위치와 크기를 제공하고, 속도는 Kalman Filter가 내부 상태로 추정합니다.

### 5.3 프레임마다 일어나는 일

트래커의 `update()`는 아래 순서로 동작합니다.

1. 현재 프레임과 이전 프레임의 시간 차이로 `dt` 계산
2. 상태 전이 행렬과 process noise 갱신
3. 모든 트랙 예측 단계 수행
4. 이전 ego pose가 있으면 ego-motion 보정 수행
5. detection을 measurement로 변환
6. IoU 기반 매칭 수행
7. 매칭된 트랙은 Kalman update
8. 매칭되지 않은 트랙은 `LOST` 또는 `REMOVED`
9. 매칭되지 않은 detection은 새 트랙 생성

### 5.4 매칭 전략

매칭은 Hungarian이 아니라, 현재 구현 기준으로 다음 규칙의 greedy IoU matching입니다.

- 클래스가 같아야 함
- IoU가 `0.2` 이상이어야 함
- 후보들을 IoU 내림차순 정렬
- 이미 사용된 track / detection은 재사용하지 않음

이 방식은 구현이 단순하고 빠르며, 현재 구조에서는 실시간 데모 용도로 적절합니다.

### 5.5 Ego-motion 보정

이 트래커의 특징 중 하나는 ego vehicle의 움직임을 반영한다는 점입니다.

- 이전 pose와 현재 pose의 yaw 차이를 계산
- 월드 좌표계에서의 이동량을 이전 ego frame으로 변환
- 각 track의 위치와 속도를 반대로 보정

즉, 카메라가 움직여서 bbox가 이동한 경우와, 실제 객체가 이동한 경우를 완전히 구분하지는 못하더라도, 단순 화면 좌표 추적보다 훨씬 안정적인 예측을 하도록 설계되어 있습니다.

### 5.6 트랙 생명주기

트랙 상태는 아래처럼 관리됩니다.

- `NEW`
  - 막 생성된 트랙
- `TRACKED`
  - 일정 프레임 이상 안정적으로 매칭된 트랙
- `LOST`
  - 이번 프레임에는 검출되지 않았지만 예측으로 유지 중인 트랙
- `REMOVED`
  - 너무 오래 매칭되지 않아 제거 대상이 된 트랙

현재 주요 파라미터:

- `kMinTrackedAge = 2`
- `kMaxLostCount = 10`
- `kMinIoUForMatch = 0.2`
- `kMaxDtSec = 0.2`

### 5.7 결과 시각화

`OpenCVViewerAdapter`는 트래킹 결과를 매우 직관적으로 보여줍니다.

- Detection box: 노란색
- `NEW`: 파란색
- `TRACKED`: 초록색
- `LOST`: 빨간색
- Track ID / 클래스 / confidence / 속도 / age / lost count 표시
- 속도 벡터를 화살표로 표시

즉, 이 프로젝트는 “검출 결과를 보는 시스템”이 아니라, “객체 상태 변화를 해석할 수 있는 시스템”으로 볼 수 있습니다.

## 6. 설정 구조

설정은 JSON 기반이며 `ConfigLoader`가 `AssemblyConfig`로 로드합니다.

상위 구조는 아래 4개입니다.

- `videoLoaderConfig`
- `vslamAdapterConfig`
- `viewerConfig`
- `detectionConfig`

### 6.1 videoLoaderConfig

입력 소스와 형식을 정의합니다.

- `dataType`: `videoFile`, `frameSet`, `cameraDevice`, `rtspStream`
- `cameraFormat`: `mono`, `stereo`, `rgbd`
- `fps`
- `frameSize`
- `mono` 또는 `stereo` 상세 설정

### 6.2 vslamAdapterConfig

위치 추정 백엔드를 정의합니다.

- `type`: `stellaVslam`, `orbSlam3`, `none`
- `method`: `monocular`, `stereo`, `rgbd`
- `vslamConfigFilePath`
- `vocabPath`
- `useInternalViewer`
- `frameQueueSize`
- `loadConfig`
- `saveConfig`

### 6.3 viewerConfig

외부 시각화 방식을 선택합니다.

- `none`
- `opencv`
- `pangolin`
- `socket`

### 6.4 detectionConfig

객체 검출 방식을 정의합니다.

- `type`: `yolov8`, `none`
- `yolov8.modelPath`
- `confThreshold`
- `nmsThreshold`
- `inputWidth`, `inputHeight`
- `useCuda`

## 7. 샘플 실행 시나리오

`res/etc`에는 실행 예제가 들어 있습니다.

- [`res/etc/vp_config_mono.json`](res/etc/vp_config_mono.json)
  - mono frame set + Stella VSLAM + YOLOv8 + OpenCV viewer
- [`res/etc/vp_config_stereo.json`](res/etc/vp_config_stereo.json)
  - stereo frame set + Stella VSLAM + YOLOv8 + OpenCV viewer
- [`res/etc/vp_config_mono_orbslam.json`](res/etc/vp_config_mono_orbslam.json)
  - ORB-SLAM3 기반 예시 설정

## 8. 빌드 및 실행

### 8.1 Build

프로젝트 내부 `build.sh`를 사용합니다.

```bash
cd vision_pilot
./build.sh debug
```

또는

```bash
cd vision_pilot
./build.sh release
```

빌드 스크립트는 다음을 수행합니다.

- Ninja 기반 CMake configure
- `build/<type>/vision_pilot`에 바이너리 생성
- `build/<type>/install`에 설치
- `compile_commands.json` 심볼릭 링크 생성

### 8.2 Run

빌드 산출물은 저장소 루트 기준 `build/debug/vision_pilot/bin/vp`에 생성됩니다.
즉, 실행은 저장소 루트에서 아래처럼 하는 것이 가장 명확합니다.

```bash
./build/debug/vision_pilot/bin/vp -c vision_pilot/res/etc/vp_config_mono.json info
```

또는 stereo:

```bash
./build/debug/vision_pilot/bin/vp -c vision_pilot/res/etc/vp_config_stereo.json info
```

`vision_pilot` 디렉토리 안에서 실행한다면 경로를 이렇게 바꿔야 합니다.

```bash
../build/debug/vision_pilot/bin/vp -c res/etc/vp_config_mono.json info
```

## 9. 테스트 관점에서 본 신뢰도

프로젝트에는 모듈 단위 gtest가 비교적 잘 들어가 있습니다.

- `frame_loader`
  - mono/stereo frame set 로딩
- `vslam_adapter`
  - 초기화, dataset 로딩, KITTI sequence 처리
- `object_detection_adapter`
  - YOLO 초기화 및 샘플 이미지 검출
- `object_tracking_adapter`
  - track 생성, ID 유지, lost/removal, ego-motion 보정, bbox convention
- `application/service`
  - mono/stereo 서비스 흐름 검증
- `visualization_adapter`
  - viewer 시작/종료 안정성
- `gaia`
  - 유틸리티 라이브러리 단위 테스트 다수

즉, 이 프로젝트는 단일 데모 코드라기보다, 파이프라인 각 단을 독립적으로 검증할 수 있도록 구성되어 있습니다.

## 10. 현재 구현 상태와 주의점


### 현재 구현 현황

- Stella VSLAM 경로는 실제 사용 가능한 주력 구현입니다.
- ORB-SLAM3 어댑터는 현재 인터페이스 중심의 스텁에 가깝습니다.
- `SocketViewerAdapter` 역시 현재는 렌더링 로직이 거의 없는 최소 구현입니다.
- 샘플 설정 파일의 데이터셋 경로는 로컬 절대 경로 기반이라 환경에 맞게 수정이 필요합니다.
- `main.cpp`의 기본 config 경로는 `vision_pilot/res/etc/assembly_config.json`을 가리키지만, 저장소에는 `vp_config_*.json`이 제공되므로 실행 시 `-c`를 명시하는 편이 안전합니다.
- `vp_config_mono_orbslam.json`은 코드가 기대하는 `viewerConfig` 대신 `vslamViewerConfig` 키를 사용하고 있어 그대로는 설정 호환성이 맞지 않을 수 있습니다.

### 향후 개발 계획 (TODO)

#### 차량 제어 Out 포트 추가
- `steering_adapter` 생성: VSLAM + Detection + Tracking 결과를 바탕으로 조향 명령 생성
- 모의 조향(Mock Steering) 구현: 실제 차량 API 연동 전 파이프라인 검증
- 차량 제어 인터페이스: CAN/ROS 등을 통한 실제 하드웨어 연동

#### 미흡한 어댑터 개선
- **ORB-SLAM3 어댑터**: 현재 스텁 상태 → 완전한 SLAM 파이프라인 구현
  - Map persistence (맵 저장/로드)
  - 루프 클로징(Loop Closure) 활용
  - BA(Bundle Adjustment) 최적화
- **SocketViewerAdapter**: 렌더링 로직 강화
  - 실시간 3D 포인트 클라우드 시각화
  - 추적 객체의 궤적 표시
  - SLAM 불확실성 시각화
- **Object Detection Adapter**: 다중 모델 지원
  - YOLO v8/v10 등 최신 모델 대응
  - 커스텀 모델 추론 파이프라인
- **Object Tracking Adapter**: 고급 추적 알고리즘
  - 장기 재식별(Re-ID) 추가
  - 다중 가설 추적(MHT) 고려
  - 예측 기반 추적(Kalman 필터 고도화)

## 11. 이 프로젝트를 어떻게 읽으면 좋은가

처음 보는 사람이면 아래 순서로 보면 가장 빠르게 이해할 수 있습니다.

1. [`main.cpp`](main.cpp)
2. [`assembly/src/assembly_impl.cpp`](assembly/src/assembly_impl.cpp)
3. [`application/service/src/vision_pilot_service_impl.cpp`](application/service/src/vision_pilot_service_impl.cpp)
4. `adapter/in/frame_loader`
5. `adapter/out/vslam_adapter`
6. `adapter/out/object_detection_adapter`
7. `adapter/out/object_tracking_adapter`
8. `adapter/out/visualization_adapter`

## 12. 요약

`VisionPilot`은 다음 질문에 답하는 프로젝트입니다.

- 지금 카메라가 어디에 있는가?
- 지금 화면에 어떤 객체가 있는가?
- 그 객체는 이전 프레임의 어떤 객체와 같은가?
- 그 결과를 사람이 바로 해석할 수 있게 어떻게 보여줄 것인가?

구현 관점에서 보면, 이 프로젝트의 가장 큰 강점은 단순 알고리즘 나열이 아니라 `입력 → 이벤트 → 서비스 → 어댑터`로 이어지는 구조적 분리와, 그 위에 `VSLAM + Detection + Tracking`을 하나의 실행 파이프라인으로 묶어냈다는 점입니다.
