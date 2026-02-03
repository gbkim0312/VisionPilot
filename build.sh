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

# --- STEP 7: ORB_SLAM3 (FINAL FIX: Bsymbolic) ---
echo "📦 [7/8] Building ORB_SLAM3 with Symbol Isolation..."

ORB_ROOT="$THIRD_PARTY_DIR/ORB_SLAM3"
ORB_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/orb_slam3"

# 1. 찌꺼기 제거 (완전 박멸)
rm -rf "$ORB_BUILD_DIR"
rm -rf "$ORB_ROOT/Thirdparty/DBoW2/build"
rm -rf "$ORB_ROOT/Thirdparty/DBoW2/lib"
rm -rf "$ORB_ROOT/Thirdparty/g2o/build"
rm -rf "$ORB_ROOT/Thirdparty/g2o/lib"
rm -rf "$ORB_ROOT/lib/libORB_SLAM3.so"

# 2. C++14 패치
find "$ORB_ROOT" -name "CMakeLists.txt" -print0 | xargs -0 sed -i 's/CMAKE_CXX_STANDARD 11/CMAKE_CXX_STANDARD 14/g'
find "$ORB_ROOT" -name "CMakeLists.txt" -print0 | xargs -0 sed -i 's/-std=c++11/-std=c++14/g'

# [핵심 전략]
# 1. fPIC: 정적 라이브러리도 공유 라이브러리에 포함될 수 있게 함
# 2. hidden: 외부로 심볼 노출 금지
STATIC_FLAGS="-fPIC -fvisibility=hidden -fvisibility-inlines-hidden"

# 3. DBoW2 빌드 (Public Static - 헤더 의존성 때문)
echo "   -> Building internal DBoW2 (Public Static)..."
cd "$ORB_ROOT/Thirdparty/DBoW2"
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
         -DCMAKE_CXX_STANDARD=14 \
         -DCMAKE_CXX_FLAGS="-fPIC" \
         -DBUILD_SHARED_LIBS=OFF \
         "${COMMON_ARGS[@]}"
cmake --build . -j$CORES

# 4. g2o 빌드 (Hidden Static - 충돌 원흉 격리)
echo "   -> Building internal g2o (Hidden Static)..."
cd "$ORB_ROOT/Thirdparty/g2o"
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
         -DCMAKE_CXX_STANDARD=14 \
         -DCMAKE_CXX_FLAGS="$STATIC_FLAGS" \
         -DBUILD_SHARED_LIBS=OFF \
         "${COMMON_ARGS[@]}"
cmake --build . -j$CORES

# 5. Main ORB_SLAM3 빌드 (Shared + Bsymbolic)
echo "   -> Building ORB_SLAM3 Core (Shared with Bsymbolic)..."
mkdir -p "$ORB_BUILD_DIR"

# [가장 중요한 수정]
# -Wl,-Bsymbolic: 내부 심볼 우선 사용 (Stella의 g2o를 무시하고 내장 g2o 사용 강제)
# -Wl,--exclude-libs,libg2o.a: g2o 심볼은 절대 밖으로 내보내지 않음
LINKER_FLAGS="-Wl,-Bsymbolic -Wl,--exclude-libs,libg2o.a"

cmake -S "$ORB_ROOT" -B "$ORB_BUILD_DIR" "${COMMON_ARGS[@]}" \
    -DCMAKE_CXX_STANDARD=14 \
    -DCMAKE_SHARED_LINKER_FLAGS="$LINKER_FLAGS" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_RGBD_EXAMPLES=OFF \
    -DBUILD_STEREO_EXAMPLES=OFF \
    -DBUILD_MONO_EXAMPLES=OFF \
    -DBUILD_INERTIAL_EXAMPLES=OFF

# 빌드 실행
if ! cmake --build "$ORB_BUILD_DIR" -j$CORES; then
    echo "⚠️ Parallel build failed. Retrying with -j 1..."
    cmake --build "$ORB_BUILD_DIR" -j 1
fi

# 6. 설치
echo "   -> Manually installing libraries and headers..."
mkdir -p "$INSTALL_DIR/lib"
mkdir -p "$INSTALL_DIR/include/ORB_SLAM3"

if [ -f "$ORB_ROOT/lib/libORB_SLAM3.so" ]; then
    echo "   ✅ Found library in Source Tree. Copying..."
    cp "$ORB_ROOT/lib/libORB_SLAM3.so" "$INSTALL_DIR/lib/"
elif [ -f "$ORB_BUILD_DIR/lib/libORB_SLAM3.so" ]; then
    echo "   ✅ Found library in Build Tree. Copying..."
    cp "$ORB_BUILD_DIR/lib/libORB_SLAM3.so" "$INSTALL_DIR/lib/"
else
    echo "   ❌ Error: libORB_SLAM3.so not found! Build failed."
    exit 1
fi

cp -r "$ORB_ROOT/include/"* "$INSTALL_DIR/include/ORB_SLAM3/"
mkdir -p "$INSTALL_DIR/include/ORB_SLAM3/Thirdparty"
cp -r "$ORB_ROOT/Thirdparty/"* "$INSTALL_DIR/include/ORB_SLAM3/Thirdparty/" 2>/dev/null || true

cd "$ROOT_DIR"
echo "✨ ORB_SLAM3 Build Step Completed."

# --- STEP 7: VisionPilot ---
echo "📦 [8/8] Building VisionPilot..."
cd "$ROOT_DIR/vision_pilot"
./build.sh "$BUILD_TYPE_LOWER"

echo "✨ [Master] All builds completed successfully!"