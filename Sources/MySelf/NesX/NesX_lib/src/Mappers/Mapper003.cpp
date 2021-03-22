
#include "Mappers/Mapper003.h"

CMapper003::~CMapper003()
{
    Quit();
}

bool CMapper003::WritePRG(const Address &address, const uint8_t &data)
{
    // Bank select $8000-$FFFF
    if(address >= 0x8000u && address <= 0xFFFFu)
    {
        // Select 8 KB CHR ROM bank for PPU $0000-$1FFF
        // CNROM only implements the lowest 2 bits, capping it at 32 KiB CHR
        // for homebrewing:  assume bus conflicts, do not exceed 32k CHR(bank 0-3)
        SelectCHRBankIn8K(data & 0x3u);
    }
    return CBaseMapper::WritePRG(address, data);
}

