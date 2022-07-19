
#include "Core/MapperFactory.h"

#include "Mappers/Mapper000.h"
#include "Mappers/Mapper001.h"
#include "Mappers/Mapper002.h"
#include "Mappers/Mapper003.h"
#include "Mappers/Mapper004.h"

CBaseMapper *CMapperFactory::Get(const uint16_t &uMapper)
{
    switch(uMapper)
    {
        case 0: return new CMapper000;
        case 1: return new CMapper001;
        case 2: return new CMapper002;
        case 3: return new CMapper003;
        case 4: return new CMapper004;
        default: break;
    }
    return nullptr;
}
