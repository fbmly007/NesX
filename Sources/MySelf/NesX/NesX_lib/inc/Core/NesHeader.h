
#ifndef NESX_NESX_LIB_INC_CORE_NESHEADER_H_
#define NESX_NESX_LIB_INC_CORE_NESHEADER_H_

// 详细参考
// http://wiki.nesdev.com/w/index.php/INES
// http://wiki.nesdev.com/w/index.php/NES_2.0

#include <istream>
#include <string>
using std::istream;
using std::string;

class CNesHeader
{
public:
  // Nes版本
  enum NesVariant
  {
    Variant_Invalid     = 0,
    Variant_ArchaiciNES = 1,
    Variant_iNES		= 2,
    Variant_iNES_20		= 3
  };

  // 主机类型
  enum ConsoleType
  {
    // Nintendo Entertainment System/Family Computer
    Console_NES			 = 0,

    // Nintendo Vs. System
    Console_VSSystem	 = 1,

    // Nintendo Playchoice 10
    Console_Playchoice10 = 2,

    // Extended Console Type
    Console_Extended	 = 3
  };

  // TV系统模式(TVSystem or TimingMode)
  enum TVSystem
  {
    // RP2C02 ("NTSC NES")
    TV_NTSC     = 0,

    // RP2C07("Licensed PAL NES")
    TV_PAL      = 1,

    // Multiple-region
    TV_Multiple = 2,

    // UMC 6527P ("Dendy")
    TV_Dendy    = 3
  };

  // Nes属性
  enum NesHeaderFlag
  {
    // 水平镜像
    Flag_HorizontalMirroring = 0,

    // 垂直镜像
    Flag_VerticalMirroring = 1,

    // 后背电池存档 (UNROM 512 and GTROM use flash memory to store their game state by rewriting the PRG-ROM area.)
    // This usually takes the form of battery - backed PRG - RAM at $6000, but there are some mapper - specific exceptions :
    // UNROM 512 and GTROM use flash memory to store their game state by rewriting the PRG - ROM area.
    Flag_Battery = 2,

    // Trainer
    // 512-byte trainer at $7000-$71FF (stored before PRG data)
    Flag_Trainer = 4,

    // 全4个屏幕显存
    Flag_FourScreenVRAM = 8
  };

public:
  CNesHeader();
  ~CNesHeader();

  bool Load(istream& stream);
  void Unload();

public: // QUERY
  bool HasTrainer() const;
  bool HasBettery() const;
  uint16_t Mapper() const;
  uint8_t SubMapper() const;

  uint16_t NumberOfPRG() const;
  uint16_t NumberOfCHR() const;

  bool HasVerticalMirroring() const;
  bool HasFourScreenVRAM() const;

  void GetTrainer(string& data) const;

private:
  static NesVariant CheckVariant(const uint8_t header[16]);
private:
  uint16_t	  m_uNumberOfPRG;
  uint16_t	  m_uNumberOfCHR;
  uint8_t	  m_uFlags;
  uint16_t	  m_uMapper;
  uint8_t	  m_uSubMapper;

  uint32_t    m_uSizeOfPRGRAM;
  uint32_t    m_uSizeOfPRGNVRAM;
  uint32_t    m_uSizeOfCHRRAM;
  uint32_t    m_uSizeOfCHRNVRAM;

  // m_consoleType == VSSystem
  uint8_t     m_uPPUType;
  uint8_t     m_uHardwareType;

  // m_consoleType == Extended
  uint8_t     m_uExtendedConsoleType;

  uint8_t	  m_uMiscellaneousROMs;

  uint8_t     m_uDefaultExpansionDevice;

  ConsoleType m_ConsoleType;
  NesVariant  m_Variant;
  TVSystem    m_TVSystem;
  string	  m_TrainerData;
};

#endif //NESX_NESX_LIB_INC_CORE_NESHEADER_H_
