
#ifndef NESX_NESX_LIB_INC_MAPPERS_MAPPER004_H_
#define NESX_NESX_LIB_INC_MAPPERS_MAPPER004_H_

#include "Core/BaseMapper.h"

// https://wiki.nesdev.com/w/index.php/MMC3
// http://kevtris.org/mappers/mmc3/index.html
// http://nesdev.com/mmc3.txt

class CMapper004 : public CBaseMapper
{
public:
  CMapper004();
  ~CMapper004() override;

  void SaveSnapshot(MapperSnapshot& state) override;
  void LoadSnapshot(const MapperSnapshot& state) override;

  void Reset() override;
  bool WritePRG(const Address& address, const uint8_t &data) override;
  void DataSignal(const uint8_t &signal, const Address &data) override;

private:
  // 统一更换所有Bank函数(便于传参)
  void CHR_Bank(uint8_t chr0,
                uint8_t chr1,
                uint8_t chr2,
                uint8_t chr3,
                uint8_t chr4,
                uint8_t chr5,
                uint8_t chr6,
                uint8_t chr7);
  void PRG_Bank(uint8_t prg0, uint8_t prg1, uint8_t prg2, uint8_t prg3);

  // 更新Banks(需要在8001时调用)
  void UpdateBanks();

  // 寄存器的单独实现
  void OnBankSelect(const uint8_t &data);
  void OnBankData(const uint8_t& data);
  void OnChangeMirroring(const uint8_t& data);
  void OnChangePRGProtecting(const uint8_t& data);

  void DisableIRQ();
  void EnableIRQ();

private:
  uint8_t m_uRegs[8];       // 8个寄存器的值
  uint8_t m_uRegSelector;   // 8000传入寄存器下标[0-7]
  bool m_bCHRInversion;     // CHR地址是否异或上0x1000
  uint8_t m_uPRGBankMode;   // 是否将 (6,7,-2) 变为 (-2,7,6)
  bool m_bPRGWriteProtected;
  uint8_t m_uCustomMirrorMode; // 自定义镜像模式 0-纵向, 1-横向, 2-4RAM

  uint8_t m_uIRQLatch;      // IRQ的计数初始值
  uint8_t m_uIRQCounter;    // IRQ当前的计数值

  bool m_bIRQEnabled;       // 是否允许MMC3的中断
};

#endif //NESX_NESX_LIB_INC_MAPPERS_MAPPER004_H_
