cmake_minimum_required(VERSION 3.15)
project(NesX)

set(CMAKE_CXX_STANDARD 11)

# 设置CMake模块路径 (根CMakeLists也需要这个来查找SDL2)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmakes)

# 根据模块路径查找SDL2
find_package(SDL2 REQUIRED)

# 设置头文件路径 (inc和SDL2)
include_directories(inc ${SDL2_INCLUDE_DIR})

# 设置源文件
set(SOURCE_FILES main.cpp)

add_executable(NesX ${SOURCE_FILES})

# 设置头文件路径 (inc和SDL2)
include_directories(NesSound_lib/inc NesX_lib/inc)
add_subdirectory(NesX_lib)
add_subdirectory(NesSound_lib)

# 链接
target_link_libraries(NesX NesX_lib)
