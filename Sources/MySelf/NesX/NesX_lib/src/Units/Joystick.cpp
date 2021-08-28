
#include "Units/Joystick.h"
#include <SDL2/SDL.h>

CJoystick::CJoystick()
    : m_bStrobe(false),
      m_uIndex1(0u),
      m_PadStates1{},
      m_uIndex2(0u),
      m_PadStates2{}
{

}

void CJoystick::SaveSnapshot(JoystickSnapshot &state)
{
    state.strobe = m_bStrobe;

    // Joystick 1
    state.index1 = m_uIndex1;
    memcpy(state.padStates1, this->m_PadStates1, sizeof(m_PadStates1));

    // Joystick 2
    state.index2 = m_uIndex2;
    memcpy(state.padStates2, this->m_PadStates2, sizeof(m_PadStates2));
}

void CJoystick::LoadSnapshot(const JoystickSnapshot &state)
{
//    bool strobe;
//
//    // Joystick 1
//    uint8_t index1;
//    bool padStates1[8];
//
//    // Joystick 2
//    uint8_t index2;
//    bool padStates2[8];
    m_bStrobe = state.strobe;

    // Joystick 1
    m_uIndex1 = state.index1;
    memcpy(this->m_PadStates1, state.padStates1, sizeof(m_PadStates1));

    // Joystick 2
    m_uIndex2 = state.index2;
    memcpy(this->m_PadStates2, state.padStates2, sizeof(m_PadStates2));
}

void CJoystick::PowerUp()
{
}

void CJoystick::Reset()
{
}

void CJoystick::PowerDown()
{
}

void CJoystick::Tick()
{
}

int CJoystick::Divider() const
{
    return 0;
}


uint8_t CJoystick::Read(int n)
{
    if(n == 1) return Read1() | 0x40u;
    return Read2() | 0x40u;
}

void CJoystick::Write(bool bStrobe)
{
    if(m_bStrobe != bStrobe)
    {
        if(bStrobe)
        {
            // 重置下标
            m_uIndex1 = 0u;
            m_uIndex2 = 0u;
        }
        else
        {
            RefreshJoystick1();
            RefreshJoystick2();
        }

    }
    m_bStrobe = bStrobe;
}

uint8_t CJoystick::Read1()
{
    if(m_uIndex1 > 7u) return 0x1u;
    return m_PadStates1[m_uIndex1++];
}

uint8_t CJoystick::Read2()
{
    if(m_uIndex2 > 7u) return 0x1u;
    return m_PadStates2[m_uIndex2++];
}

void CJoystick::RefreshJoystick1()
{
    // 将状态读回
    // A
    m_PadStates1[0] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_G] & 1u;

    // B
    m_PadStates1[1] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_H] & 1u;

    // Select
    m_PadStates1[2] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_C] & 1u;

    // Start
    m_PadStates1[3] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_V] & 1u;

    // Up
    m_PadStates1[4] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_W] & 1u;

    // Down
    //if(!m_PadStates1[4])
        m_PadStates1[5] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_S] & 1u;

    // Left
    m_PadStates1[6] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_A] & 1u;

    // Right
    //if(!m_PadStates1[6])
        m_PadStates1[7] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_D] & 1u;
}

void CJoystick::RefreshJoystick2()
{
    // A
    m_PadStates2[0] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_1] & 1u;

    // B
    m_PadStates2[1] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_2] & 1u;

    // Select
    m_PadStates2[2] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_0] & 1u;

    // Start
    m_PadStates2[3] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_RETURN2] & 1u;

    // Up
    m_PadStates2[4] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_UP] & 1u;

    // Down
    m_PadStates2[5] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_DOWN] & 1u;

    // Left
    m_PadStates2[6] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_LEFT] & 1u;

    // Right
    m_PadStates2[7] = SDL_GetKeyboardState(nullptr)[SDL_Scancode::SDL_SCANCODE_RIGHT] & 1u;
}



