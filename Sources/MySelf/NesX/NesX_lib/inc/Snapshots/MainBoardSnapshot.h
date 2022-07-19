
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_MAINBOARDSNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_MAINBOARDSNAPSHOT_H_

#include <SDL.h>
#include "apu_snapshot.h"
#include "CPUSnapshot.h"
#include "PPUSnapshot.h"

#pragma pack(push)
#pragma pack(1)
struct MainBoardSnapshot
{
    // 时钟状态
    Uint64 clockIndex;
    Uint64 clockValue;

    CPUSnapshot cpu;    // CPU状态
    PPUSnapshot ppu;    // PPU状态
    apu_snapshot_t apu; // APU状态
};
#pragma pack(pop)

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_MAINBOARDSNAPSHOT_H_
