cmake_minimum_required(VERSION 2.8.5)

project(copper)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")

include_directories(${PROJECT_SOURCE_DIR}/inc)

if(CMAKE_HOST_UNIX)
  include(GNUInstallDirs)
else()
  set(CMAKE_INSTALL_LIBDIR lib)
  set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

# Libraries

add_library(${PROJECT_NAME} src/copper.c src/copper-util.c)
if(UNIX)
  target_link_libraries(copper m)
  set(LIBM -lm)
endif()
if(BUILD_SHARED_LIBS)
  set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION}
                                                   SOVERSION ${VERSION_MAJOR})
endif()

# Examples

add_executable(copper-example example/copper-example.c)
target_link_libraries(copper-example ${PROJECT_NAME})

# Tests

add_executable(copper-test test/copper-test.c)
target_link_libraries(copper-test ${PROJECT_NAME})

enable_testing()
macro(add_unit_test target)
  add_test(${target} ${EXECUTABLE_OUTPUT_PATH}/${target})
endmacro(add_unit_test)

add_unit_test(copper-test)

# pkg-config

set(PKG_CONFIG_EXEC_PREFIX "\${prefix}")
set(PKG_CONFIG_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
set(PKG_CONFIG_CFLAGS "-I\${includedir}")

set(PKG_CONFIG_NAME "${PROJECT_NAME}")
set(PKG_CONFIG_DESCRIPTION "Control Performance with Power")
set(PKG_CONFIG_LIBS "-L\${libdir} -l${PROJECT_NAME}")
set(PKG_CONFIG_LIBS_PRIVATE "${LIBM}")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
)

# Install

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES inc/copper.h inc/copper-util.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Uninstall

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  @ONLY
)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
