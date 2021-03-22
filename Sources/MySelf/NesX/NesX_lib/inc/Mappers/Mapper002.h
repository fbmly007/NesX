
#ifndef NESX_NESX_LIB_INC_MAPPERS_MAPPER002_H_
#define NESX_NESX_LIB_INC_MAPPERS_MAPPER002_H_

#include "Core/BaseMapper.h"

// https://wiki.nesdev.com/w/index.php/UxROM

class CMapper002 : public CBaseMapper
{
public:
  ~CMapper002() override;
  bool WritePRG(const Address& address, const uint8_t &data) override;
};

#endif //NESX_NESX_LIB_INC_MAPPERS_MAPPER002_H_
