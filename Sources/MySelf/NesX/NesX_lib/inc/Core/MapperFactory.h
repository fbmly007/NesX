
#ifndef NESX_NESX_LIB_SRC_CORE_MAPPERFACTORY_H_
#define NESX_NESX_LIB_SRC_CORE_MAPPERFACTORY_H_

#include "Common.h"

class CBaseMapper;

class CMapperFactory
{
public:
  static CBaseMapper * Get(const uint16_t& uMapper);
};

#endif //NESX_NESX_LIB_SRC_CORE_MAPPERFACTORY_H_
