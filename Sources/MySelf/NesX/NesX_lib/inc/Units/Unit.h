
#ifndef NESX_NESX_LIB_INC_UNITS_UNIT_H_
#define NESX_NESX_LIB_INC_UNITS_UNIT_H_

#include "Common.h"

class CMainBoard;

class CUnit
{
public:
    CUnit();
    virtual bool Init(CMainBoard *pRefMainBoard);
    virtual void Quit();
    void Clock();

protected:
    virtual void PowerUp() = 0;
    virtual void Reset() = 0;
    virtual void PowerDown() = 0;
    virtual void Tick() = 0;
    virtual int Divider() const = 0;

protected:
    CMainBoard *m_pRefMainBoard;

private:
    int m_nClockDividerCounter;

protected:
    uint64_t m_uInternalClocks;
};

#endif //NESX_NESX_LIB_INC_UNITS_UNIT_H_
