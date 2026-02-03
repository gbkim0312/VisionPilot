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

# --- STEP 7: ORB_SLAM3 (ULTIMATE FIX: Force C++14) ---
echo "📦 [7/8] Building ORB_SLAM3..."

ORB_ROOT="$THIRD_PARTY_DIR/ORB_SLAM3"
ORB_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/orb_slam3"

# 기존 빌드 캐시가 꼬였을 수 있으므로 삭제 (매우 중요)
rm -rf "$ORB_BUILD_DIR"
rm -rf "$ORB_ROOT/Thirdparty/DBoW2/build"
rm -rf "$ORB_ROOT/Thirdparty/g2o/build"

# [핵심] 모든 CMakeLists.txt에서 C++11을 C++14로 강제 치환
echo "   -> Patching ALL CMakeLists.txt to enforce C++14..."
find "$ORB_ROOT" -name "CMakeLists.txt" -print0 | xargs -0 sed -i 's/CMAKE_CXX_STANDARD 11/CMAKE_CXX_STANDARD 14/g'
find "$ORB_ROOT" -name "CMakeLists.txt" -print0 | xargs -0 sed -i 's/-std=c++11/-std=c++14/g'

# 1. DBoW2 빌드
echo "   -> Building internal DBoW2..."
cd "$ORB_ROOT/Thirdparty/DBoW2"
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
         -DCMAKE_CXX_STANDARD=14 \
         "${COMMON_ARGS[@]}"
cmake --build . -j$CORES

# 2. g2o 빌드
echo "   -> Building internal g2o..."
cd "$ORB_ROOT/Thirdparty/g2o"
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
         -DCMAKE_CXX_STANDARD=14 \
         "${COMMON_ARGS[@]}"
cmake --build . -j$CORES

# 3. Main ORB_SLAM3 빌드
echo "   -> Building ORB_SLAM3 Core..."
mkdir -p "$ORB_BUILD_DIR"

cmake -S "$ORB_ROOT" -B "$ORB_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DCMAKE_CXX_STANDARD=14 \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_RGBD_EXAMPLES=OFF \
    -DBUILD_STEREO_EXAMPLES=OFF

# 빌드 실행 (메모리 부족 방지를 위해 실패 시 단일 코어로 재시도)
if ! cmake --build "$ORB_BUILD_DIR" -j$CORES; then
    echo "⚠️ Parallel build failed. Retrying with -j 1 (Please wait)..."
    cmake --build "$ORB_BUILD_DIR" -j 1
fi

# 4. 수동 설치 (Manual Install)
# ORB_SLAM3는 install 타겟이 없으므로 직접 파일을 옮겨줘야 합니다.
echo "   -> Manually installing libraries and headers..."
mkdir -p "$INSTALL_DIR/lib"
mkdir -p "$INSTALL_DIR/include/ORB_SLAM3"

# 라이브러리 복사 (빌드된 위치에서 찾아서 복사)
if [ -f "$ORB_BUILD_DIR/lib/libORB_SLAM3.so" ]; then
    cp "$ORB_BUILD_DIR/lib/libORB_SLAM3.so" "$INSTALL_DIR/lib/"
    echo "   ✅ libORB_SLAM3.so copied successfully."
elif [ -f "$ORB_ROOT/lib/libORB_SLAM3.so" ]; then
    # 혹시 소스 트리 내부에 생겼을 경우
    cp "$ORB_ROOT/lib/libORB_SLAM3.so" "$INSTALL_DIR/lib/"
    echo "   ✅ libORB_SLAM3.so copied successfully (from source tree)."
else
    echo "   ❌ Error: libORB_SLAM3.so not found! Build must have failed."
    exit 1
fi

# 헤더 파일 통째로 복사
cp -r "$ORB_ROOT/include/"* "$INSTALL_DIR/include/ORB_SLAM3/"
# ORB_SLAM3는 include 경로가 좀 지저분해서, 소스 루트의 Thirdparty도 필요할 수 있음
mkdir -p "$INSTALL_DIR/include/ORB_SLAM3/Thirdparty"
cp -r "$ORB_ROOT/Thirdparty/"* "$INSTALL_DIR/include/ORB_SLAM3/Thirdparty/" 2>/dev/null || true

# 원래 위치로 복귀
cd "$ROOT_DIR"

# --- STEP 7: VisionPilot ---
echo "📦 [8/8] Building VisionPilot..."
cd "$ROOT_DIR/vision_pilot"
./build.sh "$BUILD_TYPE_LOWER"

echo "✨ [Master] All builds completed successfully!"