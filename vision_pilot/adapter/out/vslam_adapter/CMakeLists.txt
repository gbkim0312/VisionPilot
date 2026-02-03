project(adapter_vslam)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Pangolin REQUIRED)

set(_INCLUDE_PUBLIC include)
set(_INCLUDE_PRIVATE src)

set(ORB_SLAM3_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/ORB_SLAM3)
set(ORB_SLAM3_LIB_PATH ${CMAKE_INSTALL_PREFIX}/lib/libORB_SLAM3.so)

set(_LINK_PUBLIC_LIBRARIES vp::port_out vp::config)
set(_LINK_PRIVATE_LIBRARIES
    stella_vslam::stella_vslam
    ${OpenCV_LIBS}
    ${ORB_SLAM3_LIB_PATH}
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    ${Pangolin_LIBRARIES}
)

file(GLOB DEPS CONFIGURE_DEPENDS "src/*")
set(ALL_DEPS ${ALL_DEPS} ${DEPS})

add_library(${PROJECT_NAME} STATIC
    ${ALL_DEPS})

add_library(vp::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    HAVE_GLEW
    _LINUX_
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${_INCLUDE_PUBLIC}
  PRIVATE ${_INCLUDE_PRIVATE}
  PRIVATE ${CMAKE_INSTALL_PREFIX}/include
  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}
  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}/Thirdparty/Sophus
  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}/CameraModels


  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}/Sophus
  PRIVATE ${GLEW_INCLUDE_DIRS}
  PRIVATE ${Pangolin_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC ${_LINK_PUBLIC_LIBRARIES}
  PRIVATE ${_LINK_PRIVATE_LIBRARIES}
)

# 테스트 설정
file(GLOB DEPS CONFIGURE_DEPENDS "gtest/*")
set(ALL_DEPS ${ALL_DEPS} ${DEPS})

set(_INCLUDE_PRIVATE ${_INCLUDE_PRIVATE} gtest)
set(_LINK_PRIVATE_LIBRARIES ${_LINK_PRIVATE_LIBRARIES} gtest gmock opencv_highgui)

add_executable(${PROJECT_NAME}_test ${ALL_DEPS})

target_compile_definitions(${PROJECT_NAME}_test PRIVATE
    HAVE_GLEW
    _LINUX_
)

target_include_directories(
  ${PROJECT_NAME}_test
  PUBLIC ${_INCLUDE_PUBLIC}
  PRIVATE ${_INCLUDE_PRIVATE}
  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}
  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}/Thirdparty/Sophus
  PRIVATE ${ORB_SLAM3_INCLUDE_DIR}/CameraModels
  PRIVATE ${GLEW_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}_test
  PUBLIC ${_LINK_PUBLIC_LIBRARIES}
  PRIVATE ${_LINK_PRIVATE_LIBRARIES})

add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

install(TARGETS ${PROJECT_NAME}_test RUNTIME DESTINATION sample
                                     COMPONENT vp_debugs)