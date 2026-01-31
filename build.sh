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
    -G Ninja
)

# --- STEP 1: OpenCV 4.9.0 ---
echo "📦 [1/7] Building OpenCV..."
OPENCV_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/opencv"
mkdir -p "$OPENCV_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/opencv" -B "$OPENCV_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_LIST=core,imgproc,videoio,dnn,imgcodecs,objdetect,highgui,calib3d,features2d,flann \
    -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_SHARED_LIBS=ON
cmake --build "$OPENCV_BUILD_DIR" -j$CORES
cmake --install "$OPENCV_BUILD_DIR"

# --- STEP 2: g2o (외부 클론본) ---
echo "📦 [2/7] Building g2o..."
G2O_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/g2o"
mkdir -p "$G2O_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/g2o" -B "$G2O_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_SHARED_LIBS=ON -DBUILD_UNITTESTS=OFF -DG2O_USE_CHOLMOD=OFF -DG2O_USE_CSPARSE=ON -DG2O_USE_OPENGL=OFF
cmake --build "$G2O_BUILD_DIR" -j$CORES
cmake --install "$G2O_BUILD_DIR"

# --- STEP 3: Pangolin ---
echo "📦 [3/7] Building Pangolin Ecosystem..."
# Pangolin 엔진 패치 (cstdint 추가)
sed -i '1i #include <cstdint>' "$THIRD_PARTY_DIR/Pangolin/components/pango_image/src/image_io_jpg.cpp" || true
sed -i '1i #include <cstdint>' "$THIRD_PARTY_DIR/Pangolin/components/pango_packetstream/include/pangolin/log/packetstream_tags.h" || true
sed -i '1i #include <cstdint>' "$THIRD_PARTY_DIR/Pangolin/components/pango_image/src/image_io_bmp.cpp" || true

PAN_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/pangolin"
mkdir -p "$PAN_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/Pangolin" -B "$PAN_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DBUILD_EXAMPLES=OFF
cmake --build "$PAN_BUILD_DIR" -j$CORES
cmake --install "$PAN_BUILD_DIR"


# --- STEP 4: Stella-VSLAM ---
echo "📦 [4/7] Building Stella-VSLAM Core..."
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
echo "📦 [5/7] Building Stella-VSLAM Related Modules..."
# Stella 전용 Pangolin Viewer 모듈
PAN_VIEW_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/pangolin_viewer"
mkdir -p "$PAN_VIEW_BUILD_DIR"
cmake -S "$THIRD_PARTY_DIR/pangolin_viewer" -B "$PAN_VIEW_BUILD_DIR" "${COMMON_ARGS[@]}"
cmake --build "$PAN_VIEW_BUILD_DIR" -j$CORES
cmake --install "$PAN_VIEW_BUILD_DIR"


echo "📦 [6/7] Building SocketViewer Ecosystem..."
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

# --- STEP 7: VisionPilot ---
echo "📦 [7/7] Building VisionPilot..."
cd "$ROOT_DIR/vision_pilot"
./build.sh "$BUILD_TYPE_LOWER"

echo "✨ [Master] All builds completed successfully!"