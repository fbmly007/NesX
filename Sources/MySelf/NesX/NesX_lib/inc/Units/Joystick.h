
#ifndef NESX_NESX_LIB_INC_UNITS_JOYSTICK_H_
#define NESX_NESX_LIB_INC_UNITS_JOYSTICK_H_

#include "Units/Unit.h"
#include "Snapshots/JoystickSnapshot.h"

class CJoystick : public CUnit
{
public:
  CJoystick();

  void SaveSnapshot(JoystickSnapshot& state);
  void LoadSnapshot(const JoystickSnapshot& state);

private:
  void PowerUp() override;
  void Reset() override;
  void PowerDown() override;
  void Tick() override;
  int Divider() const override;

public:
  uint8_t Read(int n);
  void Write(bool bStrobe);

private:
  uint8_t Read1();
  uint8_t Read2();
  void RefreshJoystick1();
  void RefreshJoystick2();

private:
  bool m_bStrobe;

  // Joystick 1
  uint8_t m_uIndex1;
  bool m_PadStates1[8];

  // Joystick 2
  uint8_t m_uIndex2;
  bool m_PadStates2[8];
};

#endif //NESX_NESX_LIB_SRC_UNITS_JOYSTICK_H_
