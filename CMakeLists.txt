cmake_minimum_required(VERSION 3.18)
project(cppp VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

find_package(fmt)

add_executable(cppp "")
add_subdirectory(src)

# Version preprocessor definitions
target_compile_definitions(cppp PRIVATE PROJECT="${CMAKE_PROJECT_NAME}")
target_compile_definitions(cppp PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

target_link_libraries(cppp PRIVATE fmt::fmt)
target_link_libraries(cppp PRIVATE LLVM clang-cpp) #clang
