cmake_minimum_required(VERSION 3.20)

project(vp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/test" CACHE PATH "Standard install prefix" FORCE)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# 기본 출력 경로
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# ---------- nlohmann_json ----------
# 1) find_package 우선 (패키지 매니저/시스템 설치본)
find_package(nlohmann_json CONFIG QUIET)

# 2) 없으면 FetchContent로 가져오기 (header-only)
if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )

    # 헤더온리라 별도 옵션 거의 없음
    FetchContent_MakeAvailable(nlohmann_json)

    # 일부 환경에서 타깃 이름이 다를 수 있으니 alias 보정
    if(TARGET nlohmann_json AND NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    endif()
endif()


set(OPENCV_DEBUG_POSTFIX "d")

option(BUILD_OPEN_CV_FROM_SOURCE "Fetch&Build OpenCV" ON)

if(BUILD_OPEN_CV_FROM_SOURCE)
    set(BUILD_LIST core,imgproc,videoio,dnn,imgcodecs,objdetect,highgui CACHE STRING "" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_PERF_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_opencv_world OFF CACHE BOOL "" FORCE)
    set(WITH_IPP OFF CACHE BOOL "" FORCE)
    set(WITH_OPENGL OFF CACHE BOOL "" FORCE)
    set(WITH_QT OFF CACHE BOOL "" FORCE)
    set(WITH_TBB OFF CACHE BOOL "" FORCE)
    set(WITH_GSTREAMER OFF CACHE BOOL "" FORCE) # 필요 시 ON
    set(WITH_1394 OFF CACHE BOOL "" FORCE)
    set(WITH_V4L ON CACHE BOOL "" FORCE) # Linux 카메라면 ON
    set(WITH_FFMPEG ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.9.0
    )
    FetchContent_MakeAvailable(opencv)

    # OpenCV 헤더 경로 설정
    if(BUILD_OPEN_CV_FROM_SOURCE)
        # 하위 모듈들을 위한 인터페이스 경로 전파
        set(_OPENCV_MODULES calib3d core imgproc videoio dnn imgcodecs objdetect highgui features2d flann ml photo stitching video)

        foreach(_mod ${_OPENCV_MODULES})
            if(TARGET opencv_${_mod})
                target_include_directories(opencv_${_mod} INTERFACE
                "$<BUILD_INTERFACE:${opencv_BINARY_DIR}>"
                "$<BUILD_INTERFACE:${opencv_SOURCE_DIR}/include>"
                "$<BUILD_INTERFACE:${opencv_SOURCE_DIR}/modules/${_mod}/include>"
                "$<BUILD_INTERFACE:${opencv_BINARY_DIR}/modules/${_mod}/include>"
                "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
                )
            endif()
        endforeach()
    endif()
else()
    find_package(OpenCV REQUIRED COMPONENTS calib3d core imgproc videoio dnn imgcodecs)
endif()

# 의존성 가져오기
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
 # GCC 5.0 미만을 사용하는 경우 is_trivially_copy_constructible 지원이 되지 않으므로 1.11.0 이후는 사용 불가
)
FetchContent_GetProperties(googletest)

if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  set(gtest_build_tests OFF CACHE BOOL "Enable gtest tests" FORCE)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark
  GIT_TAG v1.5.3
)
FetchContent_GetProperties(googlebenchmark)

if(NOT googlebenchmark_POPULATED)
  FetchContent_Populate(googlebenchmark)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable gbench tests" FORCE)
  add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_subdirectory(gaia)
add_subdirectory(config)
add_subdirectory(infrastructure)
add_subdirectory(adapter)
add_subdirectory(application)
add_subdirectory(assembly)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    vp::assembly
    vp::config
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/assembly/include
)

install(TARGETS gtest gmock spdlog
  LIBRARY
  DESTINATION lib
  COMPONENT vp_libraries)

install(FILES res/etc/sample.mp4
  DESTINATION etc
  COMPONENT vp_samples
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME
  DESTINATION bin
  COMPONENT vp_executables
)