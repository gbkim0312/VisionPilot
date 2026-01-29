MESSAGE(STATUS "==========================================")
MESSAGE(STATUS "AutoCrypt GAIA Package")
MESSAGE(STATUS "==========================================")

cmake_minimum_required(VERSION 3.22)

project(gaia)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(FATAL_ERROR "You should build this project at the root directory.")
endif()

# 별도 build 폴더가 아닌 경우 cmake 할 수 없도록 방지
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory.")
endif()

# #################
# 의존성 가져오기 #
# #################
include(FetchContent)

FetchContent_Declare(
    leveldb
    GIT_REPOSITORY https://github.com/google/leveldb.git
    GIT_TAG 1.23
)
FetchContent_GetProperties(leveldb)

if(NOT leveldb_POPULATED)
    FetchContent_Populate(leveldb)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch/leveldb/CMakeLists.txt DESTINATION ${leveldb_SOURCE_DIR}) # STATIC 빌드 강제하기 위해 넣음
    set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "Enable leveldb tests" FORCE)
    set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "Enable leveldb bench" FORCE)
    add_subdirectory(${leveldb_SOURCE_DIR} ${leveldb_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.10.0
)
FetchContent_GetProperties(spdlog)

if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "Enable spdlog tests" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "Enable spdlog bench" FORCE)
    set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Force spdlog to shared" FORCE)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# New ZIP lib miniz
FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz
    GIT_TAG 2.2.0
)
FetchContent_GetProperties(miniz)

if(NOT miniz_POPULATED)
    FetchContent_Populate(miniz)
    set(miniz_build_tests OFF CACHE BOOL "Enable miniz tests" FORCE)
    add_subdirectory(${miniz_SOURCE_DIR} ${miniz_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# ####################
# 의존성 가져오기 끝 #
# ####################

file(GLOB DEPS CONFIGURE_DEPENDS "src/*.cpp")
set(ALL_DEPS ${ALL_DEPS} ${DEPS})

file(GLOB DEPS CONFIGURE_DEPENDS "src/${_STANDARD_PATH}/*.cpp")
set(ALL_DEPS ${ALL_DEPS} ${DEPS})

add_library(${PROJECT_NAME} STATIC
    ${ALL_DEPS}
    ZoneDetect/library/zonedetect.c
)
add_library(gaia::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
PUBLIC
    include
PRIVATE
    ZoneDetect/library # TODO: 느린 ZoneDetect 대신 다른 빠른 도구가 없나 탐색 필요
)

# 의존성 설정
target_link_libraries(${PROJECT_NAME}
PUBLIC
    leveldb
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    miniz
)

file(GLOB DEPS CONFIGURE_DEPENDS "gtest/*" "gtest/${_STANDARD_PATH}/*")
add_executable(${PROJECT_NAME}_test ${DEPS})
target_include_directories(${PROJECT_NAME}_test PRIVATE gtest ZoneDetect/library)
target_link_libraries(${PROJECT_NAME}_test PRIVATE gaia::gaia gtest)

file(GLOB DEPS CONFIGURE_DEPENDS "gbench/*")
add_executable(${PROJECT_NAME}_bench ${DEPS})
target_include_directories(${PROJECT_NAME}_bench PRIVATE ZoneDetect/library)
target_link_libraries(${PROJECT_NAME}_bench PRIVATE gaia::gaia benchmark::benchmark)