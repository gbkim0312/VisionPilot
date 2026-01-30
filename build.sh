#!/bin/zsh
set -e

ROOT_DIR=$(readlink -f .)
COMMON_BUILD_DIR="$ROOT_DIR/build"
CORES=$(nproc)

BUILD_TYPE=${1:-debug}
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
CMAKE_BUILD_TYPE="${(C)BUILD_TYPE_LOWER}"
INSTALL_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/install"


echo "🚀 [Master] Starting Unified Monorepo Build ($CMAKE_BUILD_TYPE)"
mkdir -p "$INSTALL_DIR"

# --- STEP 1: OpenCV 4.9.0 ---
echo "📦 [1/4] Building OpenCV..."
OPENCV_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/opencv"
mkdir -p "$OPENCV_BUILD_DIR"
cmake -S "$ROOT_DIR/opencv" -B "$OPENCV_BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_LIST=core,imgproc,videoio,dnn,imgcodecs,objdetect,highgui,calib3d,features2d,flann \
    -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_SHARED_LIBS=ON
cmake --build "$OPENCV_BUILD_DIR" -j$CORES
cmake --install "$OPENCV_BUILD_DIR"

# --- STEP 2: g2o (외부 클론본) ---
echo "📦 [2/4] Building g2o -> $INSTALL_DIR"
G2O_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/g2o"
mkdir -p "$G2O_BUILD_DIR"
cmake -S "$ROOT_DIR/g2o" -B "$G2O_BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_UNITTESTS=OFF -DG2O_USE_CHOLMOD=OFF -DG2O_USE_CSPARSE=ON -DG2O_USE_OPENGL=OFF
cmake --build "$G2O_BUILD_DIR" -j$CORES
cmake --install "$G2O_BUILD_DIR"

# --- STEP 3: Stella-VSLAM ---
echo "📦 [3/4] Building Stella-VSLAM..."
# FBoW 빌드를 위해 3rd/FBoW 경로가 포함되어야 할 수 있음
STELLA_BUILD_DIR="$COMMON_BUILD_DIR/$BUILD_TYPE_LOWER/stella_vslam"
mkdir -p "$STELLA_BUILD_DIR"
cmake -S "$ROOT_DIR/stella_vslam" -B "$STELLA_BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_PREFIX_PATH="$INSTALL_DIR" \
    -DBUILD_SHARED_LIBS=ON
cmake --build "$STELLA_BUILD_DIR" -j$CORES
cmake --install "$STELLA_BUILD_DIR"

# --- STEP 4: VisionPilot ---
echo "📦 [4/4] Building VisionPilot..."
cd "$ROOT_DIR/vision_pilot"
./build.sh "$BUILD_TYPE_LOWER"

echo "✨ [Master] All builds completed successfully!"