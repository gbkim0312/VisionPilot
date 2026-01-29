#!/bin/zsh

# sudo apt install -y build-essential cmake pkg-config libjpeg-dev libtiff5-dev libpng-dev ffmpeg libavcodec-dev libavformat-dev libswscale-dev libxvidcore-dev libx264-dev libxine2-dev libv4l-dev v4l-utils libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgtk-3-dev mesa-utils libgl1-mesa-dri libgtkglext1-dev libatlas-base-dev gfortran libeigen3-dev python3 python3-dev python3-numpy

# sudo apt install libprotobuf-dev protobuf-compiler

# sudo apt-get install -y libopenjp2-7-dev pkg-config
# sudo apt install curl libcurl4-openssl-dev
# 사용법 출력
function usage() {
    echo "Usage: $0 [build_type]"
    echo "  build_type: debug | release"
    exit 1
}

# 빌드 타입 인자 확인
if [ $# -ne 1 ]; then
    usage
fi

BUILD_TYPE=$1
BUILD_TYPE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

if [[ "$BUILD_TYPE" == "debug" ]]; then
    CMAKE_BUILD_TYPE="Debug"
elif [[ "$BUILD_TYPE" == "release" ]]; then
    CMAKE_BUILD_TYPE="Release"
else
    usage
fi

BUILD_DIR="build/${CMAKE_BUILD_TYPE}"
INSTALL_DIR="${BUILD_DIR}/test"

# 빌드 디렉토리 생성
mkdir -p "${BUILD_DIR}"

# CMake 설정 (compile_commands.json 생성)
cmake -S . -B "${BUILD_DIR}" -G Ninja \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
      -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DBUILD_opencv_highgui=ON \
      -DBUILD_opencv_videoio=ON \
      -DCMAKE_PREFIX_PATH="${PWD}/${BUILD_DIR}/_deps/opencv-build"

# 빌드
cmake --build "${BUILD_DIR}" --config "${CMAKE_BUILD_TYPE}"

# 설치
cmake --install "${BUILD_DIR}"

# compile_commands.json을 프로젝트 루트에 심볼릭 링크(이미 있으면 갱신)
ln -sf "${BUILD_DIR}/compile_commands.json" "./compile_commands.json"

echo "Build finished: ${CMAKE_BUILD_TYPE} mode"
echo "  - install: ${INSTALL_DIR}/"
echo "  - compile_commands.json -> ${BUILD_DIR}/compile_commands.json (symlink)"