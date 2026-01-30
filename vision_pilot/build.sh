#!/bin/zsh

function usage() {
    echo "Usage: $0 [build_type]"
    echo "  build_type: debug | release"
    exit 1
}

if [ $# -ne 1 ]; then usage; fi

BUILD_TYPE=$(echo "$1" | tr '[:upper:]' '[:lower:]')
if [[ "$BUILD_TYPE" == "debug" ]]; then
    CMAKE_BUILD_TYPE="Debug"
elif [[ "$BUILD_TYPE" == "release" ]]; then
    CMAKE_BUILD_TYPE="Release"
else
    usage
fi

# --- 경로 설정 (모노레포 통합 구조) ---
PROJECT_ROOT=$(readlink -f "$(dirname "$0")/..")
COMMON_BUILD_DIR="${PROJECT_ROOT}/build"
SPECIFIC_BUILD_DIR="${COMMON_BUILD_DIR}/${BUILD_TYPE}/vision_pilot"
INSTALL_DIR="${COMMON_BUILD_DIR}/${BUILD_TYPE}/install"

echo "🚩 VP Build Target: ${SPECIFIC_BUILD_DIR}"
echo "🚩 Common Install Root: ${INSTALL_DIR}"

mkdir -p "${SPECIFIC_BUILD_DIR}"

# --- OpenCVConfig.cmake 위치 확인 ---
# 보통 lib/cmake/opencv4 에 있지만, 환경에 따라 lib64일 수도 있음
OPENCV_CMAKE_DIR="${INSTALL_DIR}/lib/cmake/opencv4"

# CMake 설정
cmake -S . -B "${SPECIFIC_BUILD_DIR}" -G Ninja \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
      -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
      -DCMAKE_PREFIX_PATH="${INSTALL_DIR}" \
      -DOpenCV_DIR="${OPENCV_CMAKE_DIR}" \
      -Dstella_vslam_DIR="${INSTALL_DIR}/lib/cmake/stella_vslam" \
      -DPROJECT_ROOT_DIR="${PROJECT_ROOT}" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 빌드 및 설치
cmake --build "${SPECIFIC_BUILD_DIR}" --config "${CMAKE_BUILD_TYPE}"
cmake --install "${SPECIFIC_BUILD_DIR}"

# IDE 지원용 심볼릭 링크
ln -sf "${SPECIFIC_BUILD_DIR}/compile_commands.json" "$(dirname "$0")/compile_commands.json"

echo "✅ VP Build finished: ${CMAKE_BUILD_TYPE}"