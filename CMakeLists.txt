cmake_minimum_required(VERSION 3.14)
project(
  CppGen
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CppGen INTERFACE)
target_include_directories(
  CppGen INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                   $<INSTALL_INTERFACE:include>)
target_compile_features(CppGen INTERFACE cxx_std_17)

install(TARGETS CppGen)
install(
  DIRECTORY include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "*.hpp")

# Tests
option(CPPGEN_BUILD_TESTS "Build CppGen tests" ON)
if(CPPGEN_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  string(REPLACE "\\" "/" _compiler_path "${CMAKE_CXX_COMPILER}")
  set(CPPGEN_COMPILER_PATH "${_compiler_path}")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/compiler_config.hpp.in
                 ${CMAKE_CURRENT_BINARY_DIR}/compiler_config.hpp)

  add_executable(cppgen_tests tests/cppgen_tests.cpp)
  target_include_directories(cppgen_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(cppgen_tests PRIVATE CppGen GTest::gtest
                                             GTest::gtest_main)
  add_test(NAME CppGenTests COMMAND cppgen_tests)
endif()
