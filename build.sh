#!/bin/zsh
set -e

ROOT_DIR=$(readlink -f .)
THIRD_PARTY_DIR="$ROOT_DIR/thirdparty"
COMMON_BUILD_DIR="$ROOT_DIR/build"
CORES=$(nproc)

BUILD_TYPE=${1:-debug}
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
CMAKE_BUILD_TYPE="${(C)BUILD_TYPE_LOWER}"
INSTALL_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/install"


echo "🚀 [Master] Starting Unified Monorepo Build ($CMAKE_BUILD_TYPE)"
mkdir -p "$INSTALL_DIR"

COMMON_ARGS=(
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
    -DCMAKE_PREFIX_PATH="$INSTALL_DIR"
    -DOpenGL_GL_PREFERENCE=GLVND
    -DCMAKE_POLICY_DEFAULT_CMP0072=NEW
    -G Ninja
)

# --- STEP 1: OpenCV 4.9.0 ---
echo "📦 [1/8] Building OpenCV..."
OPENCV_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/opencv"
mkdir -p "$OPENCV_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/opencv" -B "$OPENCV_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_LIST=core,imgproc,videoio,dnn,imgcodecs,objdetect,highgui,calib3d,features2d,flann \
    -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_SHARED_LIBS=ON
cmake --build "$OPENCV_BUILD_DIR" -j$CORES
cmake --install "$OPENCV_BUILD_DIR"

# --- STEP 2: g2o (외부 클론본) ---
echo "📦 [2/8] Building g2o..."
G2O_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/g2o"
mkdir -p "$G2O_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/g2o" -B "$G2O_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_SHARED_LIBS=ON -DBUILD_UNITTESTS=OFF -DG2O_USE_CHOLMOD=OFF -DG2O_USE_CSPARSE=ON -DG2O_USE_OPENGL=OFF
cmake --build "$G2O_BUILD_DIR" -j$CORES
cmake --install "$G2O_BUILD_DIR"

# --- STEP 3: Pangolin ---
echo "📦 [3/8] Building Pangolin Ecosystem..."
# Pangolin 엔진 패치 (cstdint 추가)
sed -i '1i #include <cstdint>' "$THIRD_PARTY_DIR/Pangolin/components/pango_image/src/image_io_jpg.cpp" || true
sed -i '1i #include <cstdint>' "$THIRD_PARTY_DIR/Pangolin/components/pango_packetstream/include/pangolin/log/packetstream_tags.h" || true
sed -i '1i #include <cstdint>' "$THIRD_PARTY_DIR/Pangolin/components/pango_image/src/image_io_bmp.cpp" || true

PAN_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/pangolin"
mkdir -p "$PAN_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/Pangolin" -B "$PAN_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_PANGOLIN_VIDEO=OFF \
    -DBUILD_PANGOLIN_GUI=ON \
    -DCHECK_IF_NVIDIA=OFF

cmake --build "$PAN_BUILD_DIR" -j$CORES
cmake --install "$PAN_BUILD_DIR"


# --- STEP 4: Stella-VSLAM ---
echo "📦 [4/8] Building Stella-VSLAM Core..."
# 이 단계에서 앞서 설치된 FBoW, Pangolin_Viewer, Socket_Publisher를 자동으로 찾습니다.
STELLA_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/stella_vslam"
mkdir -p "$STELLA_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/stella_vslam" -B "$STELLA_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_SHARED_LIBS=ON \
    -DUSE_PANGOLIN_VIEWER=ON \
    -DUSE_SOCKET_PUBLISHER=ON
cmake --build "$STELLA_BUILD_DIR" -j$CORES
cmake --install "$STELLA_BUILD_DIR"

# --- STEP 5: Stella-VSLAM 관련 모듈들 ---
echo "📦 [5/8] Building Stella-VSLAM Related Modules..."
# Stella 전용 Pangolin Viewer 모듈
PAN_VIEW_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/pangolin_viewer"
mkdir -p "$PAN_VIEW_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/pangolin_viewer" -B "$PAN_VIEW_BUILD_DIR" "${COMMON_ARGS[@]}"
cmake --build "$PAN_VIEW_BUILD_DIR" -j$CORES
cmake --install "$PAN_VIEW_BUILD_DIR"


echo "📦 [6/8] Building SocketViewer Ecosystem..."
SIO_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/sio"
mkdir -p "$SIO_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/socket.io-client-cpp" -B "$SIO_BUILD_DIR" "${COMMON_ARGS[@]}" -DBUILD_UNIT_TESTS=OFF
cmake --build "$SIO_BUILD_DIR" -j$CORES
cmake --install "$SIO_BUILD_DIR"

SOCK_PUB_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/socket_publisher"
mkdir -p "$SOCK_PUB_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/socket_publisher" -B "$SOCK_PUB_BUILD_DIR" "${COMMON_ARGS[@]}"
cmake --build "$SOCK_PUB_BUILD_DIR" -j$CORES
cmake --install "$SOCK_PUB_BUILD_DIR"

# --- STEP 7: ORB_SLAM3 (FIXED: C++14 Forced) ---
echo "📦 [7/8] Building ORB_SLAM3..."

ORB_ROOT="$THIRD_PARTY_DIR/ORB_SLAM3"
ORB_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/orb_slam3"

# [핵심 수정] C++11 -> C++14로 강제 변경
# 1. CMAKE_CXX_STANDARD 변수 수정
sed -i 's/set(CMAKE_CXX_STANDARD 11)/set(CMAKE_CXX_STANDARD 14)/g' "$ORB_ROOT/CMakeLists.txt" || true

# 2. [핵심] CMAKE_CXX_FLAGS 내에 하드코딩된 "-std=c++11" 문자열 치환
# ORB_SLAM3는 보통 set(CMAKE_CXX_FLAGS "... -std=c++11 ...") 이렇게 작성되어 있어 이 부분이 필수입니다.
sed -i 's/-std=c++11/-std=c++14/g' "$ORB_ROOT/CMakeLists.txt" || true

# 1. ORB_SLAM3 내장 DBoW2 빌드 (이걸 해야 libDBoW2.so가 생김)
echo "   -> Building internal DBoW2 for ORB_SLAM3..."
cd "$ORB_ROOT/Thirdparty/DBoW2"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" "${COMMON_ARGS[@]}"
cmake --build . -j$CORES

# 2. ORB_SLAM3 내장 g2o 빌드 (시스템 g2o가 있어도 내부 버전을 써야 함)
echo "   -> Building internal g2o for ORB_SLAM3..."
cd "$ORB_ROOT/Thirdparty/g2o"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" "${COMMON_ARGS[@]}"
cmake --build . -j$CORES

# 3. Main ORB_SLAM3 빌드
echo "   -> Building ORB_SLAM3 Core..."
mkdir -p "$ORB_BUILD_DIR"
# 소스 경로는 그대로 두고 빌드 폴더만 분리
# C++14 적용 확인을 위해 cmake 재설정
cmake -S "$ORB_ROOT" -B "$ORB_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_RGBD_EXAMPLES=OFF \
    -DBUILD_STEREO_EXAMPLES=OFF

cmake --build "$ORB_BUILD_DIR" -j$CORES

# 설치 시도 (실패해도 넘어가도록 처리)
echo "Installing ORB_SLAM3..."
cmake --install "$ORB_BUILD_DIR" || echo "⚠️ ORB_SLAM3 install skipped (No install target)."

# 원래 위치로 복귀
cd "$ROOT_DIR"

# --- STEP 7: VisionPilot ---
echo "📦 [8/8] Building VisionPilot..."
cd "$ROOT_DIR/vision_pilot"
./build.sh "$BUILD_TYPE_LOWER"

echo "✨ [Master] All builds completed successfully!"