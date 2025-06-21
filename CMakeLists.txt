cmake_minimum_required(VERSION 3.19)

project(
  sock
  VERSION 1.0.0
  DESCRIPTION "Minimal C++ socket library")

add_compile_definitions(LIBSOCK_VERSION="${PROJECT_VERSION}")

include(CTest)
include(GNUInstallDirs)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})

configure_file(libsock.pc.in libsock.pc @ONLY)

set(CMAKE_CXX_STANDARD 23)
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
  add_compile_definitions(LIBSOCK_DEBUG)
else()
  add_compile_options(-O3)
  message(STATUS "Configuring libsock in Release")
endif()

file(GLOB_RECURSE PRIVATE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE PRIVATE_HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS "include/*.hpp")

set(ALL_SOURCES ${PRIVATE_SOURCES} ${PUBLIC_HEADERS})

find_package(PkgConfig REQUIRED)

add_library(sock SHARED ${ALL_SOURCES})
target_include_directories(
  sock
  PUBLIC "./include"
  PRIVATE "./src"
  PRIVATE "./src/misc")
set_target_properties(sock PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# tests add_custom_target(tests)
#
# add_executable(libsock_socket "tests/socket.cpp")
# target_link_libraries(libsock_socket PRIVATE libsock) add_test( NAME "Socket"
# WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND libsock_socket "socket")
# add_dependencies(tests libsock_socket)
#
# add_executable(libsock_connection "tests/connection.cpp")
# target_link_libraries(libsock_connection PRIVATE libsock) add_test( NAME
# "Connection" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND
# libsock_connection "connection") add_dependencies(tests libsock_connection)
#
# Installation
install(TARGETS sock)
install(
  DIRECTORY "include/libsock"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PATTERN "misc" EXCLUDE)
install(FILES ${CMAKE_BINARY_DIR}/libsock.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
