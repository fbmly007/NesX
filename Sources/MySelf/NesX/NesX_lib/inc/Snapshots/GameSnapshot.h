
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_GAMESNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_GAMESNAPSHOT_H_

#include "MainBoardSnapshot.h"
#include "CartridgeSnapshot.h"
#include "JoystickSnapshot.h"

struct GameSnapshot
{
    CartridgeSnapshot cartridge;
    MainBoardSnapshot mainboard;
    JoystickSnapshot joystick;
};

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_GAMESNAPSHOT_H_
