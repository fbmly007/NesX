
#include "Mappers/Mapper002.h"

CMapper002::~CMapper002()
{
    Quit();
}

bool CMapper002::WritePRG(const Address &address, const uint8_t &data)
{
    // Bank select $8000-$FFFF
    if(address >= 0x8000u && address <= 0xFFFFu)
    {
        // Emulator implementations of iNES mapper 2 treat this as a full 8-bit bank select register
        // for homebrewing:  assume bus conflicts, do not exceed 256k(bank 0-15)
        SelectPRGBankIn16K(0, data & 0xFu);
    }
    return CBaseMapper::WritePRG(address, data);
}
