cmake_minimum_required(VERSION 3.20)
project(vp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BINARY_DIR)
    set(CMAKE_BINARY_DIR "${PROJECT_ROOT_DIR}/build/debug/vision_pilot" CACHE PATH "Build directory" FORCE)
endif()

# --- 경로 설정 (마스터 스크립트와 동기화) ---
if(NOT CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # 별도 인자가 없으면 기존처럼 build/test를 쓰지만, 마스터 스크립트 호출 시에는 무시됨
    set(CMAKE_INSTALL_PREFIX "${PROJECT_ROOT_DIR}/build/debug/install" CACHE PATH "..." FORCE)
endif()

if(NOT ACV2X_TOOLCHAIN_NAME)
    set(ACV2X_TOOLCHAIN_NAME "x86-64" CACHE STRING "Toolchain name" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

if(NOT OpenCV_DIR)
    set(OpenCV_DIR "${PROJECT_ROOT_DIR}/${CMAKE_INSTALL_PREFIX}/lib/cmake/opencv4" CACHE PATH "OpenCV cmake path" FORCE)
endif()

if(NOT PROJECT_ROOT_DIR)
    set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Project root directory" FORCE)
endif()

if(NOT stella_vslam_DIR)
    set(stella_vslam_DIR "${PROJECT_ROOT_DIR}/${CMAKE_INSTALL_PREFIX}/lib/cmake/stella_vslam" CACHE PATH "Stella-VSLAM cmake path" FORCE)
endif()

message(STATUS "🚩 CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "🚩 ACV2X_TOOLCHAIN_NAME: ${ACV2X_TOOLCHAIN_NAME}")
message(STATUS "🚩 CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "🚩 OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "🚩 stella_vslam_DIR: ${stella_vslam_DIR}")

# 기본 출력 경로
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# CCache 설정
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy-18" "clang-tidy")
if(NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found! Static analysis will be skipped."  )
endif()

find_program(CLANG_EXE NAMES "clang-18" "clang")
if(NOT CLANG_EXE)
    message(FATAL_ERROR "clang compiler not found! Static analysis will be skipped.")
endif()


# --- 정적 분석 설정 ---
include(FetchContent)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_ROOT_DIR}/cmake")
include(ClangTidyFlags)

# --- 1. nlohmann_json (Header Only) ---
find_package(nlohmann_json CONFIG QUIET)
if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# --- 2. OpenCV (모노레포 install 디렉토리 참조) ---
# 마스터 스크립트에서 -DCMAKE_PREFIX_PATH 또는 -DOpenCV_DIR로 경로를 넘겨줌
find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio dnn imgcodecs objdetect highgui features2d calib3d)
message(STATUS "✅ Found OpenCV: ${OpenCV_VERSION} at ${OpenCV_DIR}")

# --- 3. Stella-VSLAM (모노레포 install 디렉토리 참조) ---
# stella_vslam_DIR은 마스터 스크립트의 ./build.sh 호출 시 전달됨
find_package(stella_vslam REQUIRED)
message(STATUS "✅ Found Stella-VSLAM at ${stella_vslam_DIR}")

# --- 4. 기타 의존성 (gtest, gbench) ---
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.15.2)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(googlebenchmark GIT_REPOSITORY https://github.com/google/benchmark GIT_TAG v1.5.3)
FetchContent_MakeAvailable(googlebenchmark)

# --- 하위 디렉토리 추가 ---
add_subdirectory(gaia)
add_subdirectory_with_static_analysis(config)
add_subdirectory_with_static_analysis(infrastructure)
add_subdirectory_with_static_analysis(adapter)
add_subdirectory_with_static_analysis(application)
add_subdirectory_with_static_analysis(assembly)

# --- 타겟 생성 및 링크 ---
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        vp::assembly
        vp::config
        ${OpenCV_LIBS}
        stella_vslam::stella_vslam  # 스텔라 라이브러리 타겟 명칭 확인 필요
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/assembly/include
)

# 1. 메인 실행 파일 설치
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT vp_executables
)

# 2. 직접 만든 서브 모듈 라이브러리들 설치 (vp::config, vp::adapter 등)
# 하위 CMakeLists.txt에서 이미 정의되어 있다면 여기서 타겟 이름으로 모아서 설치 가능
install(TARGETS config event vp_frame_loader port_in port_out assembly gaia adapter_vslam
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    COMPONENT vp_libraries
)

# 3. FetchContent로 빌드된 외부 라이브러리들 설치
# spdlog, gtest, benchmark 등은 각 프로젝트에서 정의한 타겟 이름을 사용합니다.
install(TARGETS spdlog gtest gtest_main gmock benchmark benchmark_main
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    COMPONENT vp_dependencies
)

# 4. 리소스 파일 설치
install(FILES res/etc/sample.mp4
    DESTINATION etc
    COMPONENT vp_samples
)