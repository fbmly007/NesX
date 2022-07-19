
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_CARTRIDGESNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_CARTRIDGESNAPSHOT_H_

#include "Common.h"
#include "MapperSnapshot.h"

#pragma pack(push)
#pragma pack(1)
struct CartridgeSnapshot
{
    // FIXME: hash值用于校验是否继续加载
    uint32_t prgHash;
    uint32_t chrHash;

    // mapper
    MapperSnapshot mapper; // mapper的状态
};
#pragma pack(pop)

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_CARTRIDGESNAPSHOT_H_
