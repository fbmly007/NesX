
#ifndef NESX_NESX_LIB_INC_UNITS_PPU_H_
#define NESX_NESX_LIB_INC_UNITS_PPU_H_

#include "Units/Unit.h"
#include "Snapshots/PPUSnapshot.h"
#include "Units/PPUDef.h"

class CMonitor;

class CPPU : public CUnit
{
public:
  CPPU();
  ~CPPU();

  void SaveSnapshot(PPUSnapshot& state);
  void LoadSnapshot(const PPUSnapshot& state);

  bool Init(CMainBoard *pRefMainBoard) override;
  void ConnectMonitor(CMonitor *pMonitor);
  void Quit() override;

  void PowerUp() override;
  void Reset() override;
  void PowerDown() override;

  uint8_t ReadPort(const Address &address);
  bool WritePort(const Address &address, const uint8_t &data);

  void DMA(uint8_t **pOAM, uint8_t& uOAMAddr);
private:
  uint8_t Read(const Address& address);
  bool Write(const Address &address, const uint8_t &data);

  uint8_t ReadPalette(const Address& address);
  void WritePalette(const Address& address, const uint8_t &data);

protected:
  void Tick() override;
  int Divider() const override;

private:
  // 端口(R/W)
  uint8_t OnRead2002();
  uint8_t OnRead2004();
  uint8_t OnRead2007();

  void OnWrite2000(const uint8_t &data);
  void OnWrite2001(const uint8_t &data);
  void OnWrite2003(const uint8_t &data);
  void OnWrite2004(uint8_t data);
  void OnWrite2005(const uint8_t &data);
  void OnWrite2006(const uint8_t &data);
  void OnWrite2007(const uint8_t &data);

private:
  void DEBUG_DrawTile(int tile, int x, int y, bool sprite, uint8_t attr = 0);
  void DEBUG_DrawNameTable(int nt, int ox, int oy);

  // 处理扫描线
  void HandleScanline();

private:
  // === 精灵渲染 ===
  void SpriteRun();

  uint8_t SpriteHeight() const;
  bool IsSpriteInRange(const uint8_t& uPositionY) const;
  Address GetSpritePatternAddress(const SpriteInfo& sprite);

  // Clear OAM
  void ClearNextOAMData();

  // Sprite Evaluation
  void BeginSpriteEvaluation();
  void OnSpriteEvaluation();

  // Sprite Fetches
  void BeginSpriteFetches();
  void OnSpriteFetches();

  // === 背景 ===
  void BackgroundRun();

  static Address GetBGAttributeOffset(uint16_t address);
  static Address GetBGAttributeBitShift(uint16_t address);
  static Address GetBGAttributeBitGroup(uint16_t address);
  Address GetBackgroundPatternAddress(const uint8_t& uTileIndex);

  void ReloadHorizontal();
  void ReloadVertical();
  void IncrementHorizontal();
  void IncrementVertical();

  void ShiftBackgroundRegisters();
  void ReloadBackgroundRegisters();

  // Tile Fetches
  void ClearTileDataCache();
  void BeginTileFetches();
  void OnTileFetches();

  // === 渲染 ===
  void Render();

private:
  bool IsSpriteRenderingEnabled() const;
  bool IsBackgroundRenderingEnabled() const;
  bool IsRenderingEnabled() const;
  bool IsDuringRendering() const;
  ScanLineType GetScanlineType() const;
  void IncrementPC();

private:
  // 显示器(用于绘制)
  CMonitor *m_pRefMonitor;

  // BUS数据总线上最后一次写入的值
  uint8_t m_uLastOpenBus;

  // >>===== 0-7端口寄存器 =====<<
  Reg2000 m_Reg0;       //(-/W)
  Reg2001 m_Reg1;       //(-/W)
  Reg2002 m_Reg2;       //(R/-)
  uint8_t m_uOAMAddr;   //2003(W)/ 2004(RW)
  uint16_t m_Reg6;       //(R/-)

  // >>===== Scroll =====<<
  bool m_bScrollLatch;
  uint16_t m_TmpReg6;
  uint8_t m_uScrollOffsetX;   // 仅使用3位! (x:Fine X scroll)

  // >>===== 周期 =====<<
  bool m_bFrameOdd;
  int  m_nScanline;
  int  m_nClocks;

  // >>===== 精灵渲染 =====<<

  // 主OAM和次级OAM
  uint8_t m_PrimaryOAM[4*64];
  SpriteInfo m_NextOAMData[PPU_MAX_VISIBLE_SPRITE + 1];     // 存放下一行的OAM数据
  SpriteInfo m_ActivedOAMData[PPU_MAX_VISIBLE_SPRITE];  // 当前要绘制的OAM数据

  bool m_bClearNextOAMSignal;
  // n and m (仅用再Sprite Evaluation阶段)

  uint8_t m_uTempOAMValue;       // 分周期获取值的临时值(以前是static, 不过为了保存状态, 这里放到成员里来!)
  uint8_t m_uTempOAMIndex;       // n(0-63)
  uint8_t m_uTempOAMByteOffset;  // m(0-3)
  uint8_t m_uNumberOfSpriteFound;// 找到的精灵个数
  bool m_bEvaluationOverflows;   // 溢出

  // >>===== 背景渲染 =====<<

  // 当前图块的8个像素
  uint8_t m_uBGActivedPatternDataL;
  uint8_t m_uBGActivedPatternDataH;

  // 当前图块的8个像素属性
  uint8_t m_uBGActivedAttributeDataL;
  uint8_t m_uBGActivedAttributeDataH;

  // 当前所使用的属性锁存器(每个是1 bit)
  bool m_uBGNextAttributeLatchL;
  bool m_uBGNextAttributeLatchH;

  // 下一个图块的8个像素
  uint8_t m_uBGNextPatternDataL;
  uint8_t m_uBGNextPatternDataH;


  // 8个获取周期时的临时锁存器
  bool m_bEnableBGShifter;  // 是否将临时锁存器的内容加载到 Next 中

  uint8_t m_uTmpNTByteLatch;         // 获取Name Table字节(Tile #)
  uint8_t m_uTmpATByteLatch;         // 获取属性(仅使用2位)
  uint8_t m_uTmpBGPatternLatchL;     // 获取Pattern低位字节(+0)
  uint8_t m_uTmpBGPatternLatchH;     // 获取Pattern高位字节(+8)

  Address m_uTmpBGAddress;           // 用于存放上个周期的背景图块或属性地址(以前也是放在static中的)


  // >>===== Internal RAM =====<<
  // Name Table
  uint8_t m_NameTables[0x1000]; // 额外定义的2KB 是为了方便在Cartridge中直接控制4-Screen.(其他镜像模式不使用后面的2KB)
  // Palette
  uint8_t m_Palette[0x20];

  // >>===== 颜色信息 =====<<
  uint16_t m_uColorEmphasis;
  uint8_t  m_uGrayScale;
};

#endif //NESX_NESX_LIB_INC_UNITS_PPU_H_
