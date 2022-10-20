cmake_minimum_required(VERSION 3.18)
project(cpppp VERSION 0.1)

option(BUILD_TESTS "Build test programs" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

# required for linter
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt)

# Library
add_library(cpppp_lib STATIC "")
add_subdirectory(src)
target_link_libraries(cpppp_lib PUBLIC fmt::fmt)
target_link_libraries(cpppp_lib PUBLIC LLVM clang-cpp)


if(BUILD_TESTS)
  # Test application
  find_package(Catch2 REQUIRED)

  add_executable(cpppp_test "")
  add_subdirectory(test)
  target_link_libraries(cpppp_test PRIVATE cpppp_lib)
  target_link_libraries(cpppp_test PUBLIC Catch2::Catch2WithMain)

  include(CTest)
  include(Catch)
  catch_discover_tests(cpppp_test)
endif()


# Application
add_executable(cpppp main.cpp)
target_link_libraries(cpppp PRIVATE cpppp_lib)

# Version preprocessor definitions
target_compile_definitions(cpppp PRIVATE PROJECT="${CMAKE_PROJECT_NAME}")
target_compile_definitions(cpppp PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")
