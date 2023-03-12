cmake_minimum_required(VERSION 3.24)
project(xulpp LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

enable_testing()
find_package(GTest REQUIRED)

add_library(xulpp INTERFACE
  include/xul/stripool.hpp
)
target_include_directories(xulpp
  INTERFACE
  include
)

add_executable(xulpp_tests
  test/main.cpp
)
target_link_libraries(xulpp_tests
  PUBLIC
  GTest::gmock_main
  xulpp
)

include(ExternalProject)
ExternalProject_Add(nanobench
  URL https://raw.githubusercontent.com/martinus/nanobench/master/src/include/nanobench.h
  DOWNLOAD_DIR nanobench
  DOWNLOAD_NAME nanobench.h
  DOWNLOAD_NO_EXTRACT true
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
add_executable(xulpp_benchmarks
  bench/main.cpp
  bench/bench_stripool.cpp
)
target_link_libraries(xulpp_benchmarks
  xulpp
)
add_dependencies(xulpp_benchmarks
  nanobench
)
target_include_directories(xulpp_benchmarks
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/nanobench
)
