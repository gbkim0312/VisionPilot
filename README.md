# VisionPilot

VisionPilot 모노레포입니다. 실제 핵심 애플리케이션 코드는 `vision_pilot` 디렉토리를 기준으로 보면 됩니다.

자세한 구조와 핵심 코드 설명은 `vision_pilot/README.md`를 참고하세요.

## Build

루트에서 전체 빌드를 실행합니다.

```bash
./build.sh debug
```

릴리즈 빌드는 아래와 같습니다.

```bash
./build.sh release
```

이미 의존성 빌드가 끝났고 `vision_pilot`만 다시 빌드하려면 아래처럼 실행합니다.

```bash
./build.sh debug --only-vp
```

## Thirdparty

이 저장소의 나머지 구성 요소는 `thirdparty` 하위에 직접 준비해야 합니다.

- `vision_pilot` 이외의 주요 의존성은 `thirdparty`에 직접 클론해야 합니다.
- 필요한 저장소 목록은 `thirdparty/clone_3rd.sh`를 참고하세요.
- 전체 빌드 스크립트는 `thirdparty` 경로에 소스가 이미 존재한다고 가정합니다.

## 참고

- 핵심 코드 및 아키텍처: `vision_pilot/README.md`
- 전체 빌드 진입점: `build.sh`
- 의존성 클론 목록: `thirdparty/clone_3rd.sh`
