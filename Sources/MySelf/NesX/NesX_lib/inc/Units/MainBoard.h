
#ifndef NESX_NESX_LIB_INC_UNITS_MAINBOARD_H
#define NESX_NESX_LIB_INC_UNITS_MAINBOARD_H

#include "Common.h"
#include "Snapshots/MainBoardSnapshot.h"
#include <SDL2/SDL.h>

class CMonitor;
class CCartridge;
class CJoystick;
class CCPU;
class CPPU;
class CAPU;

class CMainBoard
{
public:
  CMainBoard();
  ~CMainBoard();

  void SaveSnapshot(MainBoardSnapshot& state);
  void LoadSnapshot(const MainBoardSnapshot& state);

  bool Init(CMonitor *pRefMonitor);
  void Quit();
  void Run();

  void PowerUp();
  void Reset();
  void PowerDown();

  void InsertCartridge(CCartridge *pRefCartridge);
  void ConnectJoystick(CJoystick *pRefJoystick);

public: // BUS Interfaces
  // PPU <-> CPU
  uint8_t PPURead(const Address& address);
  bool PPUWrite(const Address& address, const uint8_t& data);

  // APU <-> CPU
  uint8_t APURead(const Address& address);
  bool APUWrite(const Address& address, const uint8_t& data);

  // Cartridge <-> PPU/CPU/APU
  Address CartridgeNameTableMirroring(const Address& address);
  uint8_t CartridgeReadPRG(const Address& address);
  bool CartridgeWritePRG(const Address& address, const uint8_t &data);
  uint8_t CartridgeReadCHR(const Address& address);
  bool CartridgeWriteCHR(const Address& address, const uint8_t &data);

  void CartridgeDataSignal(const uint8_t& signal, const Address& data);

  void CPUSignal(const uint8_t& signal);
  uint8_t CPUCountOfWrite() const;
  void CPUDMA(uint8_t **pOAM, uint8_t& uOAMAddr);
  uint8_t CPURead(const Address& address);

  void JoystickStrobe(const uint8_t& data);
  uint8_t JoystickRead(int n);

private:
  void SetClockIndex(const Uint64& index);

private:
  CCartridge *m_pRefCartridge;
  CJoystick  *m_pRefJoystick;
  // 组件
  CCPU *m_pCPU;
  CPPU *m_pPPU;
  CAPU *m_pAPU;

  // >> 周期控制相关

  // 每帧的Counter数量
  const Uint64 m_kCountersPerFrame;

  // 当前的时钟状态
  Uint64 m_uCurrentClockIndex;
  Uint64 m_uCurrentClockValue;
};

#endif //NESX_NESX_LIB_INC_UNITS_MAINBOARD_H
