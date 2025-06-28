cmake_minimum_required(VERSION 4.0.3)
project(
  sock
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

include(GNUInstallDirs)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})

configure_file(libsock.pc.in libsock.pc @ONLY)

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Wno-unused-parameter
  -Wno-unused-value
  -Wno-missing-field-initializers
  -Wno-narrowing
  -Wno-pointer-arith)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  message(STATUS "Configuring libsock in Debug")
  add_compile_options(-O0 -g3)
  add_compile_definitions(DEBUG)
else()
  add_compile_options(-O3)
  message(STATUS "Configuring libsock in Release")
endif()

file(GLOB_RECURSE IFACES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/modules/*.ixx")
file(GLOB_RECURSE PARTS CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm")

add_library(sock SHARED)
target_sources(
  sock
  PUBLIC FILE_SET
         cxx_modules
         TYPE
         CXX_MODULES
         BASE_DIRS
         "${CMAKE_CURRENT_SOURCE_DIR}/modules"
         "${CMAKE_CURRENT_SOURCE_DIR}/src"
         FILES
         ${IFACES}
         ${PARTS})

target_include_directories(
  sock PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/modules>)

target_compile_definitions(
  sock
  PUBLIC LIBSOCK_VERSION="${PROJECT_VERSION}"
  PRIVATE $<$<CONFIG:Debug>:LIBSOCK_DEBUG>)

target_compile_options(
  sock PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter
               $<$<CONFIG:Release>:-O3>)

set_target_properties(sock PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

install(
  DIRECTORY modules/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/modules
  FILES_MATCHING
  PATTERN "*.ixx")

install(
  TARGETS sock
  EXPORT sockTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

configure_file(libsock.pc.in libsock.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libsock.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
