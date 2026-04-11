project(assembly)

set(_INCLUDE_PUBLIC include)
set(_INCLUDE_PRIVATE src)

set(_LINK_PUBLIC_LIBRARIES
  vp::config
  gaia::gaia)

set(_LINK_PRIVATE_LIBRARIES
  vp::event
  vp::port_in
  vp::port_out
  vp::vp_frame_loader
  vp::adapter_yolov8
  vp::adapter_visualization
  vp::adapter_object_tracking
  vp::adapter_vslam
  vp::model
  vp::service
)

file(GLOB DEPS CONFIGURE_DEPENDS "src/*")
set(ALL_DEPS ${ALL_DEPS} ${DEPS})

add_library(${PROJECT_NAME} STATIC ${ALL_DEPS})

add_library(vp::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC ${_INCLUDE_PUBLIC}
  PRIVATE ${_INCLUDE_PRIVATE}
)
target_link_libraries(${PROJECT_NAME}
  PUBLIC ${_LINK_PUBLIC_LIBRARIES}
  PRIVATE ${_LINK_PRIVATE_LIBRARIES}
)
