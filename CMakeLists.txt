cmake_minimum_required(VERSION 4.0.3)
project(
  sock
  VERSION 1.0.0
  DESCRIPTION "Minimal C++ socket library"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
# set(CMAKE_CXX_EXTENSIONS      OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

add_compile_definitions(LIBSOCK_VERSION="${PROJECT_VERSION}")

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Wno-unused-parameter
  -Wno-unused-value
  -Wno-missing-field-initializers
  -Wno-narrowing
  -Wno-pointer-arith)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Configuring libsock in Debug")
  add_compile_definitions(LIBSOCK_DEBUG)
else()
  message(STATUS "Configuring libsock in Release")
  add_compile_options(-O3)
endif()

include(CTest)
include(GNUInstallDirs)
configure_file(libsock.pc.in libsock.pc @ONLY)

file(GLOB_RECURSE MODULE_IFACE CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/modules/*.cppm")
file(GLOB_RECURSE MODULE_IMPL CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(sock SHARED)
target_sources(
  sock
  PUBLIC FILE_SET
         cxx_modules
         TYPE
         CXX_MODULES
         BASE_DIRS
         "${CMAKE_CURRENT_SOURCE_DIR}/modules"
         FILES
         ${MODULE_IFACE}
  PRIVATE ${MODULE_IMPL})
target_include_directories(sock PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules")

set_target_properties(
  sock
  PROPERTIES CXX_SCAN_FOR_MODULES ON
             VERSION ${PROJECT_VERSION}
             SOVERSION 1)

install(TARGETS sock DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(
  DIRECTORY modules/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/modules
  FILES_MATCHING
  PATTERN "*.cppm"
  PATTERN "*.ixx")
install(FILES ${CMAKE_BINARY_DIR}/libsock.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
