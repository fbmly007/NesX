
#ifndef NESX_NESX_LIB_INC_MAPPERS_MAPPER003_H_
#define NESX_NESX_LIB_INC_MAPPERS_MAPPER003_H_

#include "Core/BaseMapper.h"

// https://wiki.nesdev.com/w/index.php/INES_Mapper_003

class CMapper003 : public CBaseMapper
{
public:
  ~CMapper003() override;
  bool WritePRG(const Address& address, const uint8_t &data) override;
};

#endif //NESX_NESX_LIB_INC_MAPPERS_MAPPER003_H_
