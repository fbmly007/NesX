
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_JOYSTICKSNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_JOYSTICKSNAPSHOT_H_

#include "Common.h"

#pragma pack(push)
#pragma pack(1)
struct JoystickSnapshot
{
    bool strobe;

    // Joystick 1
    uint8_t index1;
    bool padStates1[8];

    // Joystick 2
    uint8_t index2;
    bool padStates2[8];
};
#pragma pack(pop)

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_JOYSTICKSNAPSHOT_H_
