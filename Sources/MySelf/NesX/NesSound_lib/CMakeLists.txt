cmake_minimum_required(VERSION 3.15)
project(NesX_lib)

set(CMAKE_CXX_STANDARD 11)

# 设置CMake模块路径 (根CMakeLists也需要这个来查找SDL2)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmakes)

# 根据模块路径查找SDL2
find_package(SDL2 REQUIRED)

# 设置头文件路径 (inc和SDL2)
include_directories(inc ${SDL2_INCLUDE_DIR})


# 设置头文件
set(HEADER_FILES
        inc/apu_snapshot.h
        inc/blargg_common.h
        inc/blargg_source.h
        inc/Blip_Buffer.h
        inc/Blip_Synth.h
        inc/Multi_Buffer.h
        inc/Nes_Apu.h
        inc/Nes_Namco.h
        inc/Nes_Oscs.h
        inc/Nes_Vrc6.h
        inc/Nonlinear_Buffer.h
        inc/Simple_Apu.h
        inc/Sound_Queue.h
        inc/Wave_Writer.hpp
        inc/boost/config.hpp
        inc/boost/cstdint.hpp
        inc/boost/static_assert.hpp)

# 设置源文件
set(SOURCE_FILES
        src/apu_snapshot.cpp
        src/Blip_Buffer.cpp
        src/Multi_Buffer.cpp
        src/Nes_Apu.cpp
        src/Nes_Namco.cpp
        src/Nes_Oscs.cpp
        src/Nes_Vrc6.cpp
        src/Nonlinear_Buffer.cpp
        src/Simple_Apu.cpp
        src/Sound_Queue.cpp
        src/Wave_Writer.cpp)

# 编译成静态库
add_library(NesSound_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
