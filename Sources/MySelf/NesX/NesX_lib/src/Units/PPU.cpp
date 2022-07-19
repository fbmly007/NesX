
#include "Units/PPU.h"
#include "Units/MainBoard.h"
#include "Core/Monitor.h"

// https://wiki.nesdev.com/w/index.php/PPU_registers

// 这些参数依照模式可能是变化的
constexpr int s_StartOfVisible = 0;
constexpr int s_EndOfVisible = 239;
constexpr int s_StartOfPostRender = 240;
constexpr int s_EndOfPostRender = 240;
constexpr int s_StartOfVBlank = 241;
constexpr int s_EndOfVBlank = 260;
constexpr int s_StartOfPreRender = 261;
constexpr int s_EndOfPreRender = 261;

// 这些是不会变化的!
constexpr int kEndOfClocks = 341;

#include "Units/Palette.h"

CPPU::CPPU()
    : m_pRefMonitor(nullptr),
      m_uLastOpenBus(0u),

      
      m_uOAMAddr(0u),
      m_Reg6(0u),

      m_bScrollLatch(false),
      m_TmpReg6{0u},
      m_uScrollOffsetX(0u),

      m_bFrameOdd(false),
      m_nScanline(0),
      m_nClocks(0),

      // 精灵
      m_PrimaryOAM{},
      m_NextOAMData{},
      m_ActivedOAMData{},
      m_bClearNextOAMSignal(false),
      m_uTempOAMValue(0u),
      m_uTempOAMIndex(0u),
      m_uTempOAMByteOffset(0u),
      m_uNumberOfSpriteFound(0u),
      m_bEvaluationOverflows(false),

      // 背景
      m_uBGActivedPatternDataL(0u),
      m_uBGActivedPatternDataH(0u),
      m_uBGActivedAttributeDataL(0u),
      m_uBGActivedAttributeDataH(0u),
      m_uBGNextAttributeLatchL(false),
      m_uBGNextAttributeLatchH(false),
      m_uBGNextPatternDataL(0u),
      m_uBGNextPatternDataH(0u),
      m_bEnableBGShifter(false),
      m_uTmpNTByteLatch(0u),
      m_uTmpATByteLatch(0u),
      m_uTmpBGPatternLatchL(0u),
      m_uTmpBGPatternLatchH(0u),
      m_uTmpBGAddress(0u),

      m_NameTables{},
      m_Palette{},

      m_uColorEmphasis(0u),
      m_uGrayScale(0u)
{
	m_Reg0.Value = 0;
	m_Reg1.Value = 0;
	m_Reg2.Value = 0;
}

CPPU::~CPPU()
{
    Quit();
}

void CPPU::SaveSnapshot(PPUSnapshot& state)
{
    // BUS数据总线上最后一次写入的值*
    state.openBus = m_uLastOpenBus;

    // 0-7端口寄存器
    state.reg0 = m_Reg0.Value;
    state.reg1 = m_Reg1.Value;
    state.reg2 = m_Reg2.Value;
    state.oamAddr = m_uOAMAddr;
    state.reg6 = m_Reg6;


    // === 滚动 ===
    state.scrollLatch = m_bScrollLatch;
    state.tmpReg6 = m_TmpReg6;
    state.scrollOffsetX = m_uScrollOffsetX;

    // === 周期 ===
    state.frameOdd = m_bFrameOdd;
    state.scanline = m_nScanline;
    state.clocks = m_nClocks;

    // === 精灵渲染 ===
    memcpy(state.primaryOAM, this->m_PrimaryOAM, sizeof(this->m_PrimaryOAM));
    memcpy(state.nextOAMData, this->m_NextOAMData, sizeof(this->m_NextOAMData));
    memcpy(state.activedOAMData, this->m_ActivedOAMData, sizeof(this->m_ActivedOAMData));

    state.clearNextOAMSignal = m_bClearNextOAMSignal;
    // n and m (仅用再Sprite Evaluation阶段)

    state.tempOAMValue = m_uTempOAMValue;
    state.tempOAMIndex = m_uTempOAMIndex;
    state.tempOAMByteOffset = m_uTempOAMByteOffset;
    state.numberOfSpriteFound = m_uNumberOfSpriteFound;
    state.evaluationOverflows = m_bEvaluationOverflows;

    // >>===== 背景渲染 =====<<

    // 当前图块的8个像素
    state.bgActivedPatternDataL = m_uBGActivedPatternDataL;
    state.bgActivedPatternDataH = m_uBGActivedPatternDataH;

    // 当前图块的8个像素属性
    state.bgActivedAttributeDataL = m_uBGActivedAttributeDataL;
    state.bgActivedAttributeDataH = m_uBGActivedAttributeDataH;

    // 当前所使用的属性锁存器(每个是1 bit)
    state.bgNextAttributeLatchL = m_uBGNextAttributeLatchL;
    state.bgNextAttributeLatchH = m_uBGNextAttributeLatchH;

    // 下一个图块的8个像素
    state.bgNextPatternDataL = m_uBGNextPatternDataL;
    state.bgNextPatternDataH = m_uBGNextPatternDataH;

    // 8个获取周期时的临时锁存器
    state.enableBGShifter = m_bEnableBGShifter;

    state.tmpNTByteLatch = m_uTmpNTByteLatch;
    state.tmpATByteLatch = m_uTmpATByteLatch;
    state.tmpBGPatternLatchL = m_uTmpBGPatternLatchL;
    state.tmpBGPatternLatchH = m_uTmpBGPatternLatchH;

    state.tmpBGAddress = m_uTmpBGAddress;

    // >>===== Internal RAM =====<<
    memcpy(state.nameTables, this->m_NameTables, sizeof(this->m_NameTables));
    memcpy(state.palette, this->m_Palette, sizeof(this->m_Palette));
}

void CPPU::LoadSnapshot(const PPUSnapshot& state)
{
    // 先将一些影响字段的寄存器先还原
    OnWrite2000(state.reg0);
    OnWrite2001(state.reg1);
    OnWrite2003(state.oamAddr);

    m_uLastOpenBus = state.openBus;
    m_Reg2.Value = state.reg2;
    m_Reg6 = state.reg6;

    // === 滚动 ===
    m_bScrollLatch = state.scrollLatch;
    m_TmpReg6 = state.tmpReg6;
    m_uScrollOffsetX = state.scrollOffsetX;

    // === 周期 ===
    m_bFrameOdd = state.frameOdd;
    m_nScanline = state.scanline;
    m_nClocks = state.clocks;

    // === 精灵渲染 ===
    memcpy(m_PrimaryOAM, state.primaryOAM, sizeof(m_PrimaryOAM));
    memcpy(m_NextOAMData, state.nextOAMData, sizeof(m_NextOAMData));
    memcpy(m_ActivedOAMData, state.activedOAMData, sizeof(m_ActivedOAMData));
    m_bClearNextOAMSignal = state.clearNextOAMSignal;


    m_uTempOAMValue = state.tempOAMValue;
    m_uTempOAMIndex = state.tempOAMIndex;
    m_uTempOAMByteOffset = state.tempOAMByteOffset;
    m_uNumberOfSpriteFound = state.numberOfSpriteFound;
    m_bEvaluationOverflows = state.evaluationOverflows;

    // === 背景渲染 ===
    // 当前图块的8个像素
    m_uBGActivedPatternDataL = state.bgActivedPatternDataL;
    m_uBGActivedPatternDataH = state.bgActivedPatternDataH;

    // 当前图块的8个像素属性
    m_uBGActivedAttributeDataL = state.bgActivedAttributeDataL;
    m_uBGActivedAttributeDataH = state.bgActivedAttributeDataH;

    // 当前所使用的属性锁存器(每个是1 bit)
    m_uBGNextAttributeLatchL = state.bgNextAttributeLatchL;
    m_uBGNextAttributeLatchH = state.bgNextAttributeLatchH;

    // 下一个图块的8个像素
    m_uBGNextPatternDataL = state.bgNextPatternDataL;
    m_uBGNextPatternDataH = state.bgNextPatternDataH;

    // 8个获取周期时的临时锁存器
    m_bEnableBGShifter = state.enableBGShifter;

    m_uTmpNTByteLatch = state.tmpNTByteLatch;
    m_uTmpATByteLatch = state.tmpATByteLatch;
    m_uTmpBGPatternLatchL = state.tmpBGPatternLatchL;
    m_uTmpBGPatternLatchH = state.tmpBGPatternLatchH;
    m_uTmpBGAddress = state.tmpBGAddress;

    // >>===== Internal RAM =====<<
    memcpy(m_NameTables, state.nameTables, sizeof(m_NameTables));
    memcpy(m_Palette, state.palette, sizeof(m_Palette));
}

bool CPPU::Init(CMainBoard *pRefMainBoard)
{
    return CUnit::Init(pRefMainBoard);
}

void CPPU::ConnectMonitor(CMonitor *pMonitor)
{
    m_pRefMonitor = pMonitor;
}

void CPPU::Quit()
{
    CUnit::Quit();
    m_pRefMonitor = nullptr;
}

void CPPU::PowerUp()
{
    m_Reg0.Value = 0u;
    OnWrite2001(0u);
    m_Reg2.Value = 0xA0u; // +0+x xxxx //?????

    m_Reg6 = 0u;
    m_uScrollOffsetX = 0u;
    m_TmpReg6 = 0u;
    m_uLastOpenBus = 0u;
    m_bScrollLatch = false;
    m_bFrameOdd = false;
    m_nScanline = s_EndOfPostRender;
    m_nClocks = 1;

    memset(m_Palette, 0, sizeof(m_Palette));

    memset(m_NameTables, -1, sizeof(m_NameTables));

    memset(m_PrimaryOAM, 0, sizeof(m_PrimaryOAM));

    BeginSpriteEvaluation();
    BeginSpriteFetches();
    ClearTileDataCache();
}

void CPPU::Reset()
{
    m_Reg0.Value = 0;
    OnWrite2001(0u);
    m_nClocks   = 0;
    m_uOAMAddr  = 0u;

    m_Reg6 = 0u;
    m_uScrollOffsetX = 0u;
    m_TmpReg6 = 0u;
    m_uLastOpenBus = 0u;
    m_bScrollLatch = false;

    m_nScanline = s_EndOfPostRender;
    m_nClocks = 1;
    m_bFrameOdd = false;

    memset(m_Palette, 0, sizeof(m_Palette));
    memset(m_NameTables, -1, sizeof(m_NameTables));
    memset(m_PrimaryOAM, 0, sizeof(m_PrimaryOAM));

    BeginSpriteEvaluation();
    BeginSpriteFetches();
    ClearTileDataCache();
}

void CPPU::PowerDown()
{
}

uint8_t CPPU::ReadPort(const Address &address)
{
    switch (address)
    {
        case 2: return OnRead2002();
        case 4: return OnRead2004();
        case 7: return OnRead2007();
        default: return m_uLastOpenBus;
    }
}

bool CPPU::WritePort(const Address &address, const uint8_t &data)
{
    m_uLastOpenBus = data;
    switch (address)
    {
        case 0: OnWrite2000(data);
            break;
        case 1: OnWrite2001(data);
            break;
        case 3: OnWrite2003(data);
            break;
        case 4: OnWrite2004(data);
            break;
        case 5: OnWrite2005(data);
            break;
        case 6: OnWrite2006(data);
            break;
        case 7: OnWrite2007(data);
            break;
        default: return false;
    }
    return true;
}

void CPPU::DMA(uint8_t **pOAM, uint8_t& uOAMAddr)
{
    *pOAM = m_PrimaryOAM;
    uOAMAddr = m_uOAMAddr;
}

uint8_t CPPU::Read(const Address &address)
{
	// Pattern
	if (address >= 0x0000u && address <= 0x1FFFu)
	{
		return m_pRefMainBoard->CartridgeReadCHR(address);
	}

	// Name Table
	if (address >= 0x2000u && address <= 0x3EFFu)
	{
		return m_NameTables[m_pRefMainBoard->CartridgeNameTableMirroring(address)];
	}

	// Palette
	if (address >= 0x3F00u && address <= 0x3FFFu)
	{
		return m_uLastOpenBus = ReadPalette(address);
	}

	return 0u;
}

bool CPPU::Write(const Address &address, const uint8_t &data)
{
	// Pattern
	if (address >= 0x0000u && address <= 0x1FFFu)
	{
		return m_pRefMainBoard->CartridgeWriteCHR(address, data);
	}

	// Name Table
	if (address >= 0x2000u && address <= 0x3EFFu)
	{
		m_NameTables[m_pRefMainBoard->CartridgeNameTableMirroring(address)] = data;
		return true;
	}

	// Palette
	if (address >= 0x3F00u && address <= 0x3FFFu)
	{
		WritePalette(address, data);
		return true;
	}

	return false;
}

uint8_t CPPU::ReadPalette(const Address &address)
{
    Address addr = address;
    if ((addr & 0x13u) == 0x10u) addr &= ~0x10u;
    return m_Palette[addr & 0x1Fu];
}

void CPPU::WritePalette(const Address &address, const uint8_t &data)
{
    Address addr = address;
    if ((addr & 0x13u) == 0x10u) addr &= ~0x10u;
    m_Palette[addr & 0x1Fu] = data;
}

void CPPU::Tick()
{
    HandleScanline();
    Render();

    if(IsDuringRendering())
    {
		SpriteRun();
        BackgroundRun();
        m_pRefMainBoard->CartridgeDataSignal(DATA_SIGNAL_SCANLINE_COUNTER, m_nClocks);
    }

    // 341个周期=1个扫描线
    // 262条扫描线=1帧
    m_nClocks++;
    if (m_nClocks >= kEndOfClocks)
    {
        m_nScanline++;
        m_nClocks = 0;
        if (m_nScanline > s_EndOfPreRender)
        {
            // 扫描线 = 0
            m_nScanline = s_StartOfVisible;

            // 切换奇偶帧
            m_bFrameOdd = !m_bFrameOdd;
        }
    }
}

int CPPU::Divider() const
{
    return 1;
}

uint8_t CPPU::OnRead2002()
{
    // 说明: m_uLastOpenBus & 0x1Fu 因为 2002端口的前5位是不用的
    uint8_t tmp = m_Reg2.Value | (m_uLastOpenBus & 0x1Fu);

    // 重置滚动锁存器
    m_bScrollLatch = false;

    // 如果在VBlank中的话, 立即取消VBlank
    m_Reg2.InVBlank = false;

    // race conditions
    if (m_nScanline == s_StartOfVBlank)
    {
        // InVBlank=0
        if (m_nClocks == 0u)
            tmp &= ~0x80u;

        if (m_nClocks < 3)
        {
            // cpu.nmi = false;
            m_pRefMainBoard->CPUSignal(SIGNAL_CANCEL_NMI);
        }
    }

    return m_uLastOpenBus = tmp;
}

uint8_t CPPU::OnRead2004()
{
    if(m_bClearNextOAMSignal)
        return 0xFFu;
    return m_uLastOpenBus = m_PrimaryOAM[m_uOAMAddr];
}

uint8_t CPPU::OnRead2007()
{
    // * Reads are delayed by one cycle, discard the first byte read
    uint8_t tmp = m_uLastOpenBus;

    m_uLastOpenBus = Read(m_Reg6 & 0x3FFFu);

    // * if the address is in the palette range, that is (0x3F00 to 0x3FFF)
    // accesses the address, without the delayed read
    if((m_Reg6 & 0x3F00u) == 0x3F00u)
    {
        tmp = m_uLastOpenBus;
    }

    // 自增
    // * PPUADDR is incremented with the value set in PPUCTRL after every read or write to 0x2007.
    IncrementPC();

    return tmp;
}

void CPPU::OnWrite2000(const uint8_t &data)
{
    /*
        2000 write:
            t:xxxxABxxxxxxxxxx=d:xxxxxxAB
            t is m_uTmpReg6 in this case
	*/
    auto pTmpData = (Reg2000 *)&data;

    if(GetScanlineType() == ScanLineType::VBlank &&
        m_Reg2.InVBlank &&
        !m_Reg0.NMIOnVBlank &&
        pTmpData->NMIOnVBlank)
    {
        // cpu.nmi = true;
        m_pRefMainBoard->CPUSignal(SIGNAL_REQUEST_NMI);
    }

    // if ((SLnum == SLStartNMI) && !(Val & 0x80) && (Clockticks < 3))
    if (m_nScanline == s_StartOfVBlank && !pTmpData->NMIOnVBlank && m_nClocks < 3)
    {
        // cpu.nmi = false;
        m_pRefMainBoard->CPUSignal(SIGNAL_CANCEL_NMI);
    }
    m_Reg0.Value = data;

    // 仅设置NameTable, 其他不变
    m_TmpReg6 &= 0x73FFu;
    m_TmpReg6 |= (uint16_t)(m_Reg0.NameTableIndex << 10u);
}

void CPPU::OnWrite2001(const uint8_t &data)
{
    m_Reg1.Value = data;

    // 设置颜色强度和灰度
    m_uColorEmphasis = (data & 0xE0u) << 1u;
    m_uGrayScale = m_Reg1.Greyscale ? 0x30u : 0x3Fu;
}

void CPPU::OnWrite2003(const uint8_t &data)
{
    m_uOAMAddr = data;
}

void CPPU::OnWrite2004(uint8_t data)
{
    // For emulation purposes, it is probably best to completely ignore writes during rendering.
    if (IsDuringRendering())
    {
        return;
    }

    // 如果当前要写入的地址是属性字节 (m_uOAMAddr % 4 == 2, 对应着数据C)
    // 属性字节的2-4位是不用的, 因此需要考虑这一点
    // 将不用的位清0!
    if ((m_uOAMAddr & 0x03u) == 0x02u)
        data &= 0xE3u;

    m_PrimaryOAM[m_uOAMAddr++] = data;
}

void CPPU::OnWrite2005(const uint8_t &data)
{
    /*
        2005 first write:
            t:xxxxxxxxxxxABCDE=d:ABCDExxx
            x=d:xxxxxABC
        2005 second write:
            t:xxxxxxABCDExxxxx=d:ABCDExxx
            t:xABCxxxxxxxxxxxx=d:xxxxxABC
    */
    if(!m_bScrollLatch)
    {
        // 仅改变X滚动, 其他不变
        m_TmpReg6 &= 0x7FE0u;
        m_TmpReg6 |= (uint16_t)(((data & 0xF8u) >> 3u));
        m_uScrollOffsetX = data & 0x7u;
    }
    else
    {
        // 仅改变Y滚动和Y偏移, 其他不变
        m_TmpReg6 &= 0xC1Fu;
        m_TmpReg6 |= (uint16_t)((data & 0xF8u) << 2u); // CoarseY
        m_TmpReg6 |= (data & 0x7u) << 12u;             // FineY
    }
    m_bScrollLatch = !m_bScrollLatch;
}

void CPPU::OnWrite2006(const uint8_t &data)
{
    /* OOO.NN.YYYYY.XXXXX
        2006 first write:
            t:xxABCDEFxxxxxxxx=d:xxABCDEF
            t:ABxxxxxxxxxxxxxx=0 (bits 14,15 cleared)
        2006 second write:
            t:xxxxxxxxABCDEFGH=d:ABCDEFGH
            v=t

            * v is PC in this case.
    */
    // 0x2005 and 0x2006 share the same latch
    if(!m_bScrollLatch)
    {
        // Y偏移. name table, Y滚动高2位, 其余不变

        // 低8位不变
        m_TmpReg6 &= 0xFFu;
        // data赋给高8位(最高2位为0)
        m_TmpReg6 |= ((data & 0x3Fu) << 8u);
    }
    else
    {
        // 高8位不变
        m_TmpReg6 &= 0x7F00u;
        m_TmpReg6 |= data;

        // 第二次写入时, 修改寄存器2006
        m_Reg6 = m_TmpReg6;
    }
    m_bScrollLatch = !m_bScrollLatch;
}

void CPPU::OnWrite2007(const uint8_t &data)
{
    Write(m_Reg6 & 0x3FFFu, data);
    IncrementPC();
}

void CPPU::DEBUG_DrawTile(int tile, int x1, int y1, bool sprite, uint8_t attr)
{
    Address baseAddress = (m_Reg0.ScreenPatternAddress << 12u) + ((uint32_t)tile << 4u);
    if(sprite)
    {
        baseAddress = (m_Reg0.SpritePatternAddress << 12u) + ((uint32_t)tile << 4u);
    }


    for(int y = 0; y < 8; ++y)
    {
        uint8_t dataLow = Read(baseAddress + y);
        uint8_t dataHigh = Read(baseAddress + y + 8);

        for(int x = 0; x < 8; ++x)
        {
            uint32_t patternIndex = ((uint32_t)dataHigh >> (7u-x)) & 1u;
            patternIndex <<= 1u;
            patternIndex |= (((uint32_t)dataLow >> (7u-x)) & 1u);
            patternIndex &= 0x3u;
            patternIndex |= attr << 2u;
            m_pRefMonitor->DEBUG_Draw(
                x1+x, y1+y,
                kVideoColorTable[ReadPalette(patternIndex)]
            );
        }
    }
}

void CPPU::DEBUG_DrawNameTable(int nt, int ox, int oy)
{
    Address baseAddress = 0x2000u + ((Address)nt << 10u);
    uint32_t offset = 0;
    for(int y = 0; y < 30; ++y)
    {
        for(int x = 0; x < 32; ++x)
        {
            uint8_t tile = Read(baseAddress+offset);
            Address propertiesAddress = 0x23C0u | ((Address)nt << 10u) | GetBGAttributeOffset(baseAddress+offset);
            uint8_t at = Read(propertiesAddress);
            at >>= GetBGAttributeBitShift(baseAddress+offset);
            at &= 0x3u;
            DEBUG_DrawTile(tile, x * 8 + ox, y * 8 + oy, false, at);
            offset++;
        }
    }
}

//#include <fstream>
//using namespace std;
void CPPU::HandleScanline()
{
    // from http://wiki.nesdev.com/w/images/d/d1/Ntsc_timing.png
    if (m_nScanline == s_StartOfVisible && m_nClocks == 0)
    {
        m_pRefMonitor->BeginDraw();
    }
    else if(m_nScanline == s_StartOfPostRender && m_nClocks == 0)
    {
//        static bool bload = false;
//        if(!bload && SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_R])
//        {
//            ifstream ifs("Others/metroidus.bin", ios::binary);
//            uint8_t tempv=0;
//            for(Address addr = 0x2000; addr < 0x2FFFu; ++addr)
//            {
//                ifs.read((char *)&tempv, 1);
//                Write(addr, tempv);
//            }
//            bload = true;
//        }

        // 绘制调色板
//        for(int y = 0; y < 2; ++y)
//        {
//            for(int x = 0; x < 16; ++x)
//            {
//                m_pRefMonitor->DEBUG_DrawRect(
//                    {x * 15 + 300,y * 15,15,15},
//                    kVideoColorTable[ReadPalette(y*16+x)],
//                    true
//                );
//                m_pRefMonitor->DEBUG_DrawRect(
//                    {x * 15 + 300,y * 15,15,15},
//                    {255,255,255,255},
//                    false
//                );
//            }
//        }

        // $0000
//        for(int tile = 0; tile < 256; ++tile)
//        {
//            Address baseAddress = (0u) + ((uint32_t)tile << 4u);
//
//            for(int y = 0; y < 8; ++y)
//            {
//                uint8_t dataLow = Read(baseAddress + y);
//                uint8_t dataHigh = Read(baseAddress + y + 8);
//
//                for(int x = 0; x < 8; ++x)
//                {
//                    uint32_t patternIndex = ((uint32_t)dataHigh >> (7u-x)) & 1u;
//                    patternIndex <<= 1u;
//                    patternIndex |= (((uint32_t)dataLow >> (7u-x)) & 1u);
//                    patternIndex &= 0x3u;
//                    m_pRefMonitor->DEBUG_DrawRect(
//                        {tile%16*8+x + 300, tile/16*8+y + 50, 1, 1},
//                        kVideoColorTable[ReadPalette(patternIndex)],
//                        true
//                    );
//                }
//            }
//        }

        // $1000
//        for(int tile = 0; tile < 256; ++tile)
//        {
//            Address baseAddress = (0x1000u) + ((uint32_t)tile << 4u);
//
//            for(int y = 0; y < 8; ++y)
//            {
//                uint8_t dataLow = Read(baseAddress + y);
//                uint8_t dataHigh = Read(baseAddress + y + 8);
//
//                for(int x = 0; x < 8; ++x)
//                {
//                    uint32_t patternIndex = ((uint32_t)dataHigh >> (7u-x)) & 1u;
//                    patternIndex <<= 1u;
//                    patternIndex |= (((uint32_t)dataLow >> (7u-x)) & 1u);
//                    patternIndex &= 0x3u;
//                    m_pRefMonitor->DEBUG_DrawRect(
//                        {tile%16*8+x + 450, tile/16*8+y + 50, 1, 1},
//                        kVideoColorTable[ReadPalette(patternIndex)],
//                        true
//                    );
//                }
//            }
//        }

        // 绘制Name tables

        // NT0
//        DEBUG_DrawNameTable(0, 0, 250);

        // NT1
//        DEBUG_DrawNameTable(1, 270, 250);

        // NT2
//        DEBUG_DrawNameTable(2, 0, 500);

        // NT3
//        DEBUG_DrawNameTable(3, 270, 500);

        m_pRefMonitor->EndDraw();
    }
    else if (m_nScanline == s_StartOfVBlank && m_nClocks == 1)
    {
        // 在VBlank的第一条扫描线的周期1时, 设置VBL标志
        m_Reg2.InVBlank = true;

        // 清除精灵地址(一些游戏会依靠这一点)
        // 这里不清除次级OAM
        // m_uOAMAddr = 0u;

        // 发送NMI请求
        if (m_Reg0.NMIOnVBlank)
        {
            m_pRefMainBoard->CPUSignal(SIGNAL_REQUEST_NMI);
        }
    }
    else if (m_nScanline == s_StartOfPreRender)
    {
        // 倒数第2个周期的时候, 确定是否执行最后一个周期
        if (m_nClocks == kEndOfClocks - 2)
        {
            // 跳过下一帧
            if (IsRenderingEnabled() && m_bFrameOdd)
                ++m_nClocks;
        }
        else if (m_nClocks == 1)
        {
            // 在PreRender的第一条扫描的周期1时, 清除VBL, sprite 0, overflow标志
            m_Reg2.InVBlank = false;           // VBL
            m_Reg2.LostSprites = false;        // overflow
            m_Reg2.Hit = false;                // sprite 0
        }
    }
}

void CPPU::SpriteRun()
{
    // 按照周期来处理精灵与背景
	if (m_nClocks >= 1 && m_nClocks <= 64)
	{
		// 1-64周期, 清除次级OAM buffer (32个字节) -> 移到BeginSpriteEvaluation中
		ClearNextOAMData();
		return;
	}

	// 65-256周期, Sprite evaluation
	if (m_nClocks >= 65 && m_nClocks <= 256)
	{
		// Secondary OAM clear and sprite evaluation do not occur on the pre-render line.
		if (m_nClocks == 65) BeginSpriteEvaluation();
		if (GetScanlineType() != ScanLineType::PreRender)
			OnSpriteEvaluation();
		return;
	}

	// 257-320周期, Sprite fetches (8 sprites total, 8 cycles per sprite)
	if (m_nClocks >= 257 && m_nClocks <= 320)
	{
		if (m_nClocks == 257) BeginSpriteFetches();
		OnSpriteFetches();
		//return;
	}
}

uint8_t CPPU::SpriteHeight() const
{
    return m_Reg0.SpriteSize ? 16u : 8u;
}

bool CPPU::IsSpriteInRange(const uint8_t &uPositionY) const
{
    return m_nScanline >= uPositionY && m_nScanline < uPositionY + SpriteHeight();
}

Address CPPU::GetSpritePatternAddress(const SpriteInfo &sprite)
{
    // 根据 背景和精灵的顺序(2000), 8x8/8x16, 精灵自身的属性 来计算出Pattern地址

    //    Byte 1
    //    Tile index number
    //    For 8x8 sprites, the tile number of this sprite. For 8x16 sprites:
    //    76543210
    //    ||||||||
    //    |||||||+- Bank (0x0000 or 0x1000 address line used for tile fetching)
    //    +++++++-- Tile number of top of sprite (0 to 254;
    //    bottom half gets the next tile)
    //    0-256代表着精灵的上半部分的图块, 下半部分位于邻接的下一个图块(也就是说当y绘制到一半以下时, 应该用
    //    这里的Tile number 加上16的!)

    // (感谢LaiNES!)
    // 计算基址
    Address baseAddress;
    if(SpriteHeight() == 8)
        baseAddress = (m_Reg0.SpritePatternAddress << 12u) + (sprite.TileIndex << 4u);
    else
        baseAddress = ((sprite.TileIndex & 0x1u) << 12u) + ((sprite.TileIndex & 0xFEu) << 4u);

    // 计算偏移
    Address y = (Address)(m_nScanline - sprite.PositionY) & (SpriteHeight()-1u);
    //if(sprite.Attribute.FlipVertical) y = SpriteHeight() - y - 1u;
    if(sprite.Attribute & 0x80u) y = SpriteHeight() - y - 1u;
    y += y & 0x8u; // y & 8 is bank(or group).

    // 返回最终地址
    return baseAddress + y;
}

void CPPU::ClearNextOAMData()
{
    m_bClearNextOAMSignal = true; // 开启信号
    if(((uint32_t)m_nClocks & 0x7u) == 0x1u)
    {
        uint8_t index = (uint8_t)m_nClocks >> 3u;
        m_NextOAMData[index].id = 0xFFu;
        m_NextOAMData[index].PositionY = 0xFFu;
        m_NextOAMData[index].TileIndex = 0xFFu;
        m_NextOAMData[index].Attribute = 0xFFu;
        m_NextOAMData[index].PositionX = 0xFFu;

        m_NextOAMData[index].PatternDataL = 0xFFu;
        m_NextOAMData[index].PatternDataH = 0xFFu;
    }
}

void CPPU::BeginSpriteEvaluation()
{
    m_bClearNextOAMSignal = false; // 关闭信号
    m_uTempOAMValue = 0u;
    m_uTempOAMIndex = 0u;
    m_uTempOAMByteOffset = 0u;
    m_uNumberOfSpriteFound = 0u;
    m_bEvaluationOverflows = false;
}


void CPPU::OnSpriteEvaluation()
{
    // If exactly 8 sprites have been found,
    // disable writes to secondary OAM because it is full.
    // This causes sprites in back to drop out.
    if(m_uNumberOfSpriteFound > PPU_MAX_VISIBLE_SPRITE)
    {
        return;
    }


    // n overflows to 0??
    if(m_uTempOAMIndex > 63)
    {
        m_uTempOAMIndex = 0;
        m_bEvaluationOverflows = true;
    }

    // 失败了就是第4步了!!
    if(m_bEvaluationOverflows)
    {
        m_NextOAMData[m_uNumberOfSpriteFound].PositionY = m_PrimaryOAM[4 * m_uTempOAMIndex + 0];
        m_uTempOAMIndex++;
        return;
    }

    if((uint32_t)m_nClocks & 0x1u)
    {
        // 获取当前属性
        m_uTempOAMValue = m_PrimaryOAM[4 * m_uTempOAMIndex + m_uTempOAMByteOffset];
    }
    else
    {
        switch(m_uTempOAMByteOffset)
        {
            case 0://Y
                if(!IsSpriteInRange(m_uTempOAMValue))
                {
                    // TODO Y不在当前扫描线上
                    if(m_uNumberOfSpriteFound >= PPU_MAX_VISIBLE_SPRITE)
                    {
                        // If the value is not in range, increment n and m (without carry).
                        // If n overflows to 0, go to 4; otherwise go to 3
                        // FIXME: BUG?? 这块应该是这么实现的!, 不过需要再看一下文档才行额
                        m_uTempOAMIndex++;
                        m_uTempOAMByteOffset++;
                        return;
                    }

                    // 继续下一个
                    m_uTempOAMByteOffset = 3;
                    break;
                }

                // If the value is in range,
                // set the sprite overflow flag in $2002 and read the next 3 entries of OAM
                // (incrementing 'm' after each byte and incrementing 'n' when 'm' overflows);
                // if m = 3, increment n
                if(m_uNumberOfSpriteFound >= PPU_OVERFLOW_SPRITES)
                {
                    m_Reg2.LostSprites = true;
                }

                // 在范围内
                m_NextOAMData[m_uNumberOfSpriteFound].PositionY = m_uTempOAMValue;
                m_NextOAMData[m_uNumberOfSpriteFound].id = m_uTempOAMIndex;
                break;

            case 1://N
                m_NextOAMData[m_uNumberOfSpriteFound].TileIndex = m_uTempOAMValue;
                break;

            case 2://C
                // TODO 不使用位串, 以防是这里出的问题
                m_NextOAMData[m_uNumberOfSpriteFound].Attribute = m_uTempOAMValue;
                break;

            case 3://X
                m_NextOAMData[m_uNumberOfSpriteFound].PositionX = m_uTempOAMValue;
                m_uNumberOfSpriteFound++;
                break;
        }

        // m++
        m_uTempOAMByteOffset++;
        if(m_uTempOAMByteOffset > 3)
        {
            m_uTempOAMByteOffset = 0;

            // n++
            m_uTempOAMIndex++;
        }
    }
}


void CPPU::BeginSpriteFetches()
{
    memset(m_ActivedOAMData, -1, sizeof(m_ActivedOAMData));
}

void CPPU::OnSpriteFetches()
{
    if(((uint32_t)m_nClocks & 0x7u) == 0x1u)
    {
        uint8_t index = ((uint32_t)m_nClocks & 0x3Fu) >> 3u;
        if(index >= m_uNumberOfSpriteFound) return;

        m_ActivedOAMData[index] = m_NextOAMData[index];
        Address address = GetSpritePatternAddress(m_NextOAMData[index]);
        m_ActivedOAMData[index].PatternDataL = Read(address);
        m_ActivedOAMData[index].PatternDataH = Read(address + 8);
    }
}

void CPPU::BackgroundRun()
{
	// 处理背景
	if ((m_nClocks >= 1 && m_nClocks <= 257) || (m_nClocks >= 321 && m_nClocks <= 339))
	{
        if(m_nClocks == 338) return;

		if (m_nClocks == 257)
		{
			// hori(v) = hori(t)
			ShiftBackgroundRegisters();
			ReloadBackgroundRegisters();
			ReloadHorizontal();
			return;
		}

		if (m_nClocks == 1 || m_nClocks == 321 || m_nClocks == 339)
		{
			BeginTileFetches();
		}

		OnTileFetches();
		m_bEnableBGShifter = true;
		return;
	}

	if (m_nClocks >= 280 && m_nClocks <= 304)
	{
		if (GetScanlineType() == ScanLineType::PreRender)
			ReloadVertical();
		//return;
	}
}

// 获取属性的偏移地址(最后的64个字节的偏移)
Address CPPU::GetBGAttributeOffset(uint16_t address)
{
    address = address & 0x3FFu;
    uint16_t y = address >> 7u;
    uint16_t x = (uint16_t)(address >> 2u) & 0x7u;
    return (y << 3u) + x;
}

// 需要右移多少位才能在最低2位拿到当前NT对应的属性 返回值{0,2,4,6}
Address CPPU::GetBGAttributeBitShift(uint16_t address)
{
    return GetBGAttributeBitGroup(address) << 1u;
}

// 属性分4组, 每2位1组, 返回值 {0,1,2,3}
Address CPPU::GetBGAttributeBitGroup(uint16_t address)
{
    address = address & 0x3FFu;
    uint8_t y = (uint16_t)(address >> 6u) & 0x1u;
    uint8_t x = (uint16_t)(address >> 0x1u) & 0x1u;
    return (uint8_t)(y << 1u) | x;
}

Address CPPU::GetBackgroundPatternAddress(const uint8_t& uTileIndex)
{
    Address baseAddress = (m_Reg0.ScreenPatternAddress << 12u) + (uTileIndex << 4u);
    uint8_t fineY = ((uint16_t)(m_Reg6 >> 12u) & 0x7u);
    return baseAddress + fineY;
}

void CPPU::ReloadHorizontal()
{
    // 还原X
    m_Reg6 &= 0x7FE0u;
    m_Reg6 |= m_TmpReg6 & 0x1Fu;

    // 恢复Nametable的低位
    m_Reg6 &= 0x7BFFu;
    m_Reg6 |= m_TmpReg6 & 0x400u;
}

void CPPU::ReloadVertical()
{
    // 恢复Y
    m_Reg6 &= 0xC1Fu;
    m_Reg6 |= m_TmpReg6 & 0x3E0u;  // 恢复CoarseY
    m_Reg6 |= m_TmpReg6 & 0x7000u; // 恢复FineY

    // 恢复Nametable的高位
    m_Reg6 &= 0x77FFu;
    m_Reg6 |= m_TmpReg6 & 0x800u;
}

void CPPU::IncrementHorizontal()
{
    if((m_Reg6 & 0x1Fu) != 0x1Fu)
    {
        m_Reg6++;
    }
    else
    {
        // CoarseX = 0;
        // NameTable水平控制低位 反转.
        m_Reg6 ^= 0x41Fu;
    }
}

void CPPU::IncrementVertical()
{
    if((m_Reg6 & 0x7000u) != 0x7000u)
    {
        m_Reg6 += 0x1000u;
    }
    else
    {
        // Fine Y = 0
        m_Reg6 ^= 0x7000u;

        if((m_Reg6 & 0x3E0u) == 0x3A0u)
        {
            // reg6 ^= 0xBA0
            // 如果Coarse Y等于29
            m_Reg6 ^= 0x3A0u; // CoarseY = 0
            m_Reg6 ^= 0x800u; // NameTable垂直控制低位 反转.
        }
        else if((m_Reg6 & 0x3E0u) == 0x3E0u)
        {
            // 如果Coarse Y等于31
            m_Reg6 ^= 0x3E0u; // CoarseY = 0
        }
        else
        {
            m_Reg6 += 0x20;
        }
    }
}

void CPPU::ShiftBackgroundRegisters()
{
    // Next Pattern ==> Active Pattern
    m_uBGActivedPatternDataL <<= 1u;
    m_uBGActivedPatternDataL |= (m_uBGNextPatternDataL & 0x80u) != 0;

    m_uBGActivedPatternDataH <<= 1u;
    m_uBGActivedPatternDataH |= (m_uBGNextPatternDataH & 0x80u) != 0;

    // 将刚移到Actived的位移出!
    m_uBGNextPatternDataL <<= 1u;
    m_uBGNextPatternDataH <<= 1u;

    // 属性
    m_uBGActivedAttributeDataL <<= 1u;
    m_uBGActivedAttributeDataL |= m_uBGNextAttributeLatchL;

    m_uBGActivedAttributeDataH <<= 1u;
    m_uBGActivedAttributeDataH |= m_uBGNextAttributeLatchH;
}

void CPPU::ReloadBackgroundRegisters()
{
    m_uBGNextPatternDataL = m_uTmpBGPatternLatchL;
    m_uBGNextPatternDataH = m_uTmpBGPatternLatchH;

    m_uBGNextAttributeLatchL = ((m_uTmpATByteLatch & 0x1u) != 0) ? 1u : 0u;
    m_uBGNextAttributeLatchH = ((m_uTmpATByteLatch & 0x2u) != 0) ? 1u : 0u;
}

void CPPU::ClearTileDataCache()
{
    m_uBGActivedPatternDataL = 0u;
    m_uBGActivedPatternDataH = 0u;
    m_uBGActivedAttributeDataL = 0u;
    m_uBGActivedAttributeDataH = 0u;
    m_uBGNextAttributeLatchL = 0u;
    m_uBGNextAttributeLatchH = 0u;
    m_uBGNextPatternDataL = 0u;
    m_uBGNextPatternDataH = 0u;
    m_bEnableBGShifter = false;
    m_uTmpNTByteLatch = 0u;
    m_uTmpATByteLatch = 0u;
    m_uTmpBGPatternLatchL = 0u;
    m_uTmpBGPatternLatchH = 0u;
    m_uTmpBGAddress = 0u;
}

void CPPU::BeginTileFetches()
{
    m_bEnableBGShifter = false;
}

void CPPU::OnTileFetches()
{
    // 绘制之后, 将移位寄存器移动一位, 指向下一个像素数据
    if(m_bEnableBGShifter)
        ShiftBackgroundRegisters();

    // 加载当前扫描线的 (先不管下一个扫描线的预加载)
    switch((uint32_t)m_nClocks & 0x7u)
    {
        // NT
        case 1:
            m_uTmpBGAddress = 0x2000u | (m_Reg6 & 0xFFFu);
            if(m_bEnableBGShifter) ReloadBackgroundRegisters();
            break;
        case 2:
            m_uTmpNTByteLatch = Read(m_uTmpBGAddress);
            break;

        // AT
        case 3:
            m_uTmpBGAddress = 0x23C0u | (m_Reg6 & 0xC00u) | GetBGAttributeOffset(m_Reg6 & 0x3FFFu);
            break;
        case 4:
            m_uTmpATByteLatch = Read(m_uTmpBGAddress);
            m_uTmpATByteLatch >>= GetBGAttributeBitShift(m_Reg6 & 0x3FFFu);
            m_uTmpATByteLatch &= 0x3u;
            break;

        // Pattern Table Tile Low (+0)
        case 5:
            m_uTmpBGAddress = GetBackgroundPatternAddress(m_uTmpNTByteLatch);
            break;
        case 6:
            m_uTmpBGPatternLatchL = Read(m_uTmpBGAddress);
            break;

        // Pattern Table Tile High(+8)
        case 7:
            m_uTmpBGAddress = GetBackgroundPatternAddress(m_uTmpNTByteLatch) + 0x8u;
            break;
        case 0:
            m_uTmpBGPatternLatchH = Read(m_uTmpBGAddress);

            if(m_nClocks == 256) IncrementVertical();
            IncrementHorizontal();
            break;
    }
}

void CPPU::Render()
{
    if(GetScanlineType() != ScanLineType::Visible) return;
    
    uint8_t paletteIndex = 0u;

    int x = m_nClocks - 2;

    if(!(x >= 0 && x < NES_SCREEN_WIDTH)) return;

    // 开启了渲染
    if(IsRenderingEnabled())
    {
        // 背景
        if(IsBackgroundRenderingEnabled() && (x >= 8 || m_Reg1.ImageClip))
        {
            // 从当前的8个像素中选择一个
            uint32_t andValue = 1u << (7u - m_uScrollOffsetX);

            paletteIndex =  (m_uBGActivedPatternDataH & andValue) != 0;
            paletteIndex <<= 1u;
            paletteIndex |= (m_uBGActivedPatternDataL & andValue) != 0;

            // 非透明色就继续取高2位
            if(paletteIndex)
            {
                uint32_t attributeIndex = (m_uBGActivedAttributeDataH & andValue) != 0;
                attributeIndex <<= 1u;
                attributeIndex |= (m_uBGActivedAttributeDataL & andValue) != 0;
                attributeIndex <<= 2u;
                paletteIndex |= attributeIndex;
            }
        }

        // 精灵
        if(IsSpriteRenderingEnabled() && (x >= 8 || m_Reg1.SpriteClip))
        {
            uint8_t spritePaletteIndex;
            for(auto & activedSprite : m_ActivedOAMData)
            {
                if (activedSprite.id > 63) break;  // Void entry.
                uint8_t j = x - activedSprite.PositionX;
                if (j & ~7u) // j > 7
                    continue;

                // 水平翻转
                if(activedSprite.Attribute & 0x40u)
                    j = 7u - j;

                // NOTE: 文档上是按照每帧移位的方式进行实现的, 但是那种方式
                //  会根据精灵翻转属性 来进行左移或右移, 所以直接按位来选取了!

                // 拿Pattern数据
                spritePaletteIndex = ((uint32_t)activedSprite.PatternDataH >> (7u - j)) & 0x1u;
                spritePaletteIndex <<= 1u;
                spritePaletteIndex |= ((uint32_t)activedSprite.PatternDataL >> (7u - j)) & 0x1u;

                // 如果是透明色
                if(!spritePaletteIndex) continue;

                spritePaletteIndex |= ((uint32_t)activedSprite.Attribute & 0x3u) << 2u;
                spritePaletteIndex |= 0x10u;

                // Set when a nonzero pixel of sprite 0 'hits' a nonzero background pixel
                if(activedSprite.id == 0 && paletteIndex && x != 255)
                {
                    m_Reg2.Hit = true;
                }

                // 如果精灵在背景前则覆盖之前背景的[非透明颜色]
                if(!paletteIndex || (activedSprite.Attribute & 0x20u) == 0)
                    paletteIndex = spritePaletteIndex;

                // 仅需要一个点!!!, 而且按序号优先
                break;
            }
        }
    }
    else if((m_Reg6 & 0x3F00u) == 0x3F00u)
    {
        // 未开启渲染 && PC在3F00的范围内, 就选择这里的索引
        paletteIndex = m_Reg6 & 0x1Fu;
    }

    // 用背景/精灵测试的调色板索引 取出 m_uColorEmphasis对应的颜色索引(这里多定义一个colorIndex是为了可读性)
    uint32_t colorIndex = ReadPalette(paletteIndex);

    // 设置灰度和颜色
    colorIndex &= m_uGrayScale;

    // 目前理解是 m_uColorEmphasis 对 基本颜色的分组
    // 一共8组(0-7), 每组有64个颜色, 一共512个颜色
    colorIndex |= m_uColorEmphasis;

    // 绘制
    m_pRefMonitor->Draw(x, m_nScanline, kVideoColorTable[colorIndex]);
}

bool CPPU::IsSpriteRenderingEnabled() const
{
    return m_Reg1.SpriteDisplay;
}

bool CPPU::IsBackgroundRenderingEnabled() const
{
    return m_Reg1.ScreenDisplay;
}

bool CPPU::IsRenderingEnabled() const
{
    return IsSpriteRenderingEnabled() || IsBackgroundRenderingEnabled();
}

bool CPPU::IsDuringRendering() const
{
    return (GetScanlineType() == ScanLineType::PreRender
        || GetScanlineType() == ScanLineType::Visible)
        && IsRenderingEnabled();
}

ScanLineType CPPU::GetScanlineType() const
{
    if (m_nScanline <= s_EndOfVisible)           // Visible
        return ScanLineType::Visible;
    else if (m_nScanline <= s_EndOfPostRender)   // Post-Render
        return ScanLineType::PostRender;
    else if (m_nScanline <= s_EndOfVBlank)       // VBlank
        return ScanLineType::VBlank;
    else                                         // Pre-Render
        return ScanLineType::PreRender;
}

void CPPU::IncrementPC()
{
    // 不超过14位地址 0000-3FFF
    if (m_Reg0.Increment)
        m_Reg6 += 32u;
    else
        ++m_Reg6;
    m_Reg6 &= 0x3FFFu;
}





