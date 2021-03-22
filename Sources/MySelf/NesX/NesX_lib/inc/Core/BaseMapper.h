
#ifndef NESX_NESX_LIB_INC_CORE_BASEMAPPER_H_
#define NESX_NESX_LIB_INC_CORE_BASEMAPPER_H_

#include "Units/Unit.h"
#include "Snapshots/MapperSnapshot.h"
#include <istream>
using std::istream;

class CMainBoard;
class CNesHeader;

class CBaseMapper
{
  typedef Address (*Mirroring_Routine)(const Address&);
public:
  CBaseMapper();
  virtual ~CBaseMapper() = default;

  virtual void SaveSnapshot(MapperSnapshot& state);
  virtual void LoadSnapshot(const MapperSnapshot& state);

  bool Init(CMainBoard *pRefMainBoard, CNesHeader *pRefNesHeader, istream& ifs);
  virtual void Quit();

  virtual void PowerUp();
  virtual void Reset();
  virtual void PowerDown();

  virtual Address NameTableMirroring(const Address& address);
  virtual uint8_t ReadPRG(const Address& address);
  virtual bool WritePRG(const Address& address, const uint8_t &data);
  virtual uint8_t ReadCHR(const Address& address);
  virtual bool WriteCHR(const Address& address, const uint8_t &data);
  virtual void DataSignal(const uint8_t& signal, const Address& data);

protected:
  virtual bool OnInit();

  void InitRawPRG(istream& ifs);
  void InitRawCHR(istream& ifs);
  void InitSRAM();
  void InitDefaultBanks();
  void InitCommonNTMirroring();

protected:
  size_t BanksOfPRGIn8K() const;
  size_t BanksOfPRGIn16K() const;
  size_t BanksOfPRGIn32K() const;

  size_t BanksOfCHRIn1K() const;
  size_t BanksOfCHRIn2K() const;
  size_t BanksOfCHRIn4K() const;
  size_t BanksOfCHRIn8K() const;


  // PRG


  void SelectPRGBankIn8K(const int& slot, const size_t& uBank);
  void SelectPRGBankIn16K(const int& slot, const size_t& uBank);
  void SelectPRGBankIn32K(const size_t& uBank);

  // CHR
  void SelectCHRBankIn1K(const int& slot, const size_t& uBank);
  void SelectCHRBankIn2K(const int& slot, const size_t& uBank);
  void SelectCHRBankIn4K(const int& slot, const size_t& uBank);
  void SelectCHRBankIn8K(const size_t& uBank);

  void SetPRGRAMEnabled(bool bEnabled);

protected:
  static Address NT_H_Mirroring(const Address& address);
  static Address NT_V_Mirroring(const Address& address);
  static Address NT_1L_Mirroring(const Address& address);
  static Address NT_1U_Mirroring(const Address& address);
  static Address NT_NO_Mirroring(const Address& address);

protected:
  CMainBoard *m_pRefMainBoard;
  CNesHeader *m_pRefNesHeader;

  uint8_t *m_pPRGBanks[4];
  uint8_t m_cachedPrgBanks[4];   // 记录状态1
  uint8_t *m_pCHRBanks[8];
  uint8_t m_cachedChrBanks[8];   // 记录状态2

  uint8_t *m_pRawPRG;
  size_t m_uBanksOfPRG;
  uint8_t *m_pRawCHR;
  size_t m_uBanksOfCHR;
  uint8_t m_SRAM[0x2000];  // [0x6000,0x7FFF]

  Mirroring_Routine m_pfNameTableMirroring;

  bool m_bCHRRamEnabled;   // 是否允许使用CHR-RAM
  bool m_bSRAMEnabled;     // 是否允许使用PRG-RAM
};

#endif //NESX_NESX_LIB_INC_CORE_BASEMAPPER_H_
