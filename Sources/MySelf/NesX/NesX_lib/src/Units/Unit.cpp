
#include "Units/Unit.h"

CUnit::CUnit()
        : m_pRefMainBoard(nullptr),
          m_nClockDividerCounter(1),
          m_uInternalClocks(0)
{
}

bool CUnit::Init(CMainBoard *pRefMainBoard)
{
    m_pRefMainBoard = pRefMainBoard;
    m_nClockDividerCounter = 1;
    return m_pRefMainBoard != nullptr;
}

void CUnit::Quit()
{
    m_pRefMainBoard = nullptr;
}

void CUnit::Clock()
{
    if (--m_nClockDividerCounter) return;
    m_uInternalClocks++;
    m_nClockDividerCounter = Divider();
    Tick();
}
