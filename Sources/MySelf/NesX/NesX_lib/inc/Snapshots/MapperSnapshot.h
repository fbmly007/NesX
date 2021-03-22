
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_MAPPERSNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_MAPPERSNAPSHOT_H_

#include "Common.h"

#pragma pack(push)
#pragma pack(1)
struct MapperSnapshot
{
    uint8_t prgBanks[4];
    uint8_t chrBanks[8];
    uint8_t chrBankData[0x2000]; // chrBanks里的8段数据(CHR-RAM)
    uint8_t sram[0x2000]; // [0x6000,0x7FFF]
    uint8_t customMirrorMode; // mapper自定义mirror, 如果无效, 在加载后则不改变函数指针
    bool chrRamEnabled;   // 是否允许使用CHR-RAM
    bool sramEnabled;     // 是否允许使用PRG-RAM
    uint8_t extraData[1024]; // 保留1024个字节, 以后需要再扩展
};
#pragma pack(pop)

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_MAPPERSNAPSHOT_H_
