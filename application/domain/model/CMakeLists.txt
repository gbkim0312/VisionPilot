project(model)

set(_INCLUDE_PUBLIC include)
set(_INCLUDE_PRIVATE src)

set(_LINK_PUBLIC_LIBRARIES gaia::gaia)
set(_LINK_PRIVATE_LIBRARIES )

file(GLOB DEPS CONFIGURE_DEPENDS "include/*")
set(ALL_DEPS ${ALL_DEPS} ${DEPS})

add_library(${PROJECT_NAME} INTERFACE)
add_library(vp::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  INTERFACE ${_INCLUDE_PUBLIC}
)
target_link_libraries(${PROJECT_NAME}
  INTERFACE ${_LINK_PUBLIC_LIBRARIES}
)
