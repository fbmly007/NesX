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
        inc/Units/Cartridge.h
        inc/Units/CPU.h
        inc/Units/Joystick.h
        inc/Units/MainBoard.h
        inc/Units/Palette.h
        inc/Units/PPU.h
        inc/Units/PPUDef.h
        inc/Units/Unit.h
        inc/Core/BaseMapper.h
        inc/Core/MapperFactory.h
        inc/Core/Monitor.h
        inc/Core/NesHeader.h
        inc/Core/NesXFrame.h

        inc/Common.h
        inc/Config.h

        inc/Mappers/Mapper000.h
        inc/Mappers/Mapper001.h
        inc/Mappers/Mapper002.h
        inc/Mappers/Mapper003.h
        inc/Mappers/Mapper004.h

        inc/Units/APU.h

        inc/Snapshots/MainBoardSnapshot.h
        inc/Snapshots/CartridgeSnapshot.h
        inc/Snapshots/JoystickSnapshot.h
        inc/Snapshots/MapperSnapshot.h
        inc/Snapshots/CPUSnapshot.h
        inc/Snapshots/PPUSnapshot.h inc/Snapshots/GameSnapshot.h)

# 设置源文件
set(SOURCE_FILES
        src/Units/Cartridge.cpp
        src/Units/CPU.cpp
        src/Units/Joystick.cpp
        src/Units/MainBoard.cpp
        src/Units/PPU.cpp
        src/Units/Unit.cpp
        src/Core/BaseMapper.cpp
        src/Core/MapperFactory.cpp
        src/Core/Monitor.cpp
        src/Core/NesHeader.cpp
        src/Core/NesXFrame.cpp

        src/Mappers/Mapper000.cpp
        src/Mappers/Mapper001.cpp
        src/Mappers/Mapper002.cpp
        src/Mappers/Mapper003.cpp
        src/Mappers/Mapper004.cpp

        src/Units/APU.cpp )

# 编译成静态库
add_library(NesX_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})

# 根据不同平台链接不同的库
IF (WIN32)
    target_link_libraries(NesX_lib
            PRIVATE NesSound_lib
            PRIVATE ${SDL2_LIBRARY}
            PRIVATE "-lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 \
        -lole32 -loleaut32 -lshell32 -lversion -luuid -lsetupapi"
            )
ELSE()
    target_link_libraries(NesX_lib
            PRIVATE NesSound_lib
            PRIVATE ${SDL2_LIBRARY}
            )
ENDIF()
