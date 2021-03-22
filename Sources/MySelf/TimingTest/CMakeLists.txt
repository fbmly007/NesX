cmake_minimum_required(VERSION 3.15)
project(TimingTest)

set(CMAKE_CXX_STANDARD 11)

# 设置CMake模块路径 (根CMakeLists也需要这个来查找SDL2)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmakes)

# 根据模块路径查找SDL2
find_package(SDL2 REQUIRED)

# 设置头文件路径 (inc和SDL2)
include_directories(inc ${SDL2_INCLUDE_DIR})

add_executable(TimingTest main.cpp)

# 根据不同平台链接不同的库
IF (WIN32)
    target_link_libraries(TimingTest
            PRIVATE ${SDL2_LIBRARY}
            PRIVATE "-lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 \
        -lole32 -loleaut32 -lshell32 -lversion -luuid -lsetupapi"
            )
ELSE()
    target_link_libraries(TimingTest
            PRIVATE ${SDL2_LIBRARY}
            )
ENDIF()