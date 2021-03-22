
#include "Mappers/Mapper004.h"
#include "Units/MainBoard.h"
#include "Core/NesHeader.h"

CMapper004::CMapper004()
    : m_uRegs{},
      m_uRegSelector(0u),
      m_uCustomMirrorMode(2),
      m_bCHRInversion(false),
      m_uPRGBankMode(0u),
      m_bPRGWriteProtected(false),
      m_uIRQLatch(0u),
      m_uIRQCounter(0u),
      m_bIRQEnabled(false)
{
}

CMapper004::~CMapper004()
{
    Quit();
}

void CMapper004::SaveSnapshot(MapperSnapshot &state)
{
    CBaseMapper::SaveSnapshot(state);

    state.customMirrorMode = m_uCustomMirrorMode;

    state.extraData[0] = m_uRegs[0];
    state.extraData[1] = m_uRegs[1];
    state.extraData[2] = m_uRegs[2];
    state.extraData[3] = m_uRegs[3];
    state.extraData[4] = m_uRegs[4];
    state.extraData[5] = m_uRegs[5];
    state.extraData[6] = m_uRegs[6];
    state.extraData[7] = m_uRegs[7];
    state.extraData[8] = m_uRegSelector;
    state.extraData[9] = m_bCHRInversion;
    state.extraData[10] = m_uPRGBankMode;
    state.extraData[11] = m_bPRGWriteProtected;
    state.extraData[12] = m_uCustomMirrorMode;
    state.extraData[13] = m_uIRQLatch;
    state.extraData[14] = m_uIRQCounter;
    state.extraData[15] = m_bIRQEnabled;
}

void CMapper004::LoadSnapshot(const MapperSnapshot &state)
{
    CBaseMapper::LoadSnapshot(state);

    m_uRegs[0] = state.extraData[0];
    m_uRegs[1] = state.extraData[1];
    m_uRegs[2] = state.extraData[2];
    m_uRegs[3] = state.extraData[3];
    m_uRegs[4] = state.extraData[4];
    m_uRegs[5] = state.extraData[5];
    m_uRegs[6] = state.extraData[6];
    m_uRegs[7] = state.extraData[7];
    m_uRegSelector = state.extraData[8];
    m_bCHRInversion = state.extraData[9];
    m_uPRGBankMode = state.extraData[10];
    m_bPRGWriteProtected = state.extraData[11];
    m_uCustomMirrorMode = state.extraData[12];
    m_uIRQLatch = state.extraData[13];
    m_uIRQCounter = state.extraData[14];
    m_bIRQEnabled = state.extraData[15];

    // 更新
    // UpdateBanks(); // 不需要更新Bank了, 因为基类记录过这些了, 在此时已经恢复了CHR和PRG的bank和数据
    OnChangeMirroring(m_uCustomMirrorMode); // 更改镜像(如果在保存之后有变更)

    // 这里也不需要通知, 因为如果这里不允许irq, 那么cpu的irq状态也一定是false, 因此CPU还原状态时会恢复的
    // if(!m_bIRQEnabled) DisableIRQ();
}

void CMapper004::Reset()
{
    CBaseMapper::Reset();
    SetPRGRAMEnabled(!m_pRefNesHeader->HasFourScreenVRAM());
    memset(m_uRegs, 0, sizeof(m_uRegs));
    m_uRegSelector = 0;
    m_uCustomMirrorMode = 2;
    m_bCHRInversion = false;
    m_uPRGBankMode = 0;
    m_bPRGWriteProtected = false;
    m_uIRQLatch = 0;
    m_uIRQCounter = 0;
    m_bIRQEnabled = false;}


bool CMapper004::WritePRG(const Address &address, const uint8_t &data)
{
    // The chip uses A0, A13, A14, and A15 for decoding
    // Registers are: 8000h, 8001h, A000h, A001h, C000h, C001h, E000h, and E001h
    switch(address & 0xE001u)
    {
        // "control" register (Bank select ($8000-$9FFE, even))
        case 0x8000u:
            OnBankSelect(data);
            break;

        // "data" register
        case 0x8001u:
            // Data register for the desired bank#.
            OnBankData(data);
            break;

        //Mirroring ($A000-$BFFE, even)
        case 0xA000u:
            OnChangeMirroring(data);
            break;

        //PRG RAM protect ($A001-$BFFF, odd)
        case 0xA001u:
            OnChangePRGProtecting(data);
            break;

        // IRQ latch ($C000-$DFFE, even)
        case 0xC000u:
            m_uIRQLatch = data;
            break;

        // IRQ reload ($C001-$DFFF, odd)
        case 0xC001u:
            // 1. Writing any value to this register reloads the MMC3 IRQ counter at the NEXT rising edge of the PPU address,
            // presumably at PPU cycle 260 of the current scanline.

            // 2. Writing to $C001 will cause the counter to be cleared, and set reload flag to true.
            // It will be reloaded on the NEXT rising edge of filtered A12.
            m_uIRQCounter = 0u;
            break;

        // IRQ disable ($E000-$FFFE, even)
        case 0xE000u:
            DisableIRQ();
            break;

        // IRQ enable ($E001-$FFFF, odd)
        case 0xE001u:
            EnableIRQ();
            break;
    }

    // 写保护直接忽略调用基类函数
    if(m_bPRGWriteProtected) return false;
    return CBaseMapper::WritePRG(address, data);
}


void CMapper004::DataSignal(const uint8_t &signal, const Address &data)
{
    // IRQ计数
    if(signal == DATA_SIGNAL_SCANLINE_COUNTER && data == 260)
    {
        // 在下一次260周期的上升沿时, 重新加载IRQCounter
        uint8_t count = m_uIRQCounter;

        if(count == 0)
        {
            m_uIRQCounter = m_uIRQLatch;
        }
        else
        {
            --m_uIRQCounter;
        }

        if (count && m_uIRQCounter == 0 && m_bIRQEnabled)
            m_pRefMainBoard->CPUSignal(SIGNAL_REQUEST_IRQ);
    }
}


void CMapper004::CHR_Bank(uint8_t chr0,
                          uint8_t chr1,
                          uint8_t chr2,
                          uint8_t chr3,
                          uint8_t chr4,
                          uint8_t chr5,
                          uint8_t chr6,
                          uint8_t chr7)
{
    SelectCHRBankIn1K(0, chr0);
    SelectCHRBankIn1K(1, chr1);
    SelectCHRBankIn1K(2, chr2);
    SelectCHRBankIn1K(3, chr3);
    SelectCHRBankIn1K(4, chr4);
    SelectCHRBankIn1K(5, chr5);
    SelectCHRBankIn1K(6, chr6);
    SelectCHRBankIn1K(7, chr7);
}

void CMapper004::PRG_Bank(uint8_t prg0, uint8_t prg1, uint8_t prg2, uint8_t prg3)
{
    SelectPRGBankIn8K(0, prg0);
    SelectPRGBankIn8K(1, prg1);
    SelectPRGBankIn8K(2, prg2);
    SelectPRGBankIn8K(3, prg3);
}

void CMapper004::UpdateBanks()
{
    // R0和R1必须是偶数
    m_uRegs[0] &= 0xFEu;
    m_uRegs[1] &= 0xFEu;


    // CHR
    if(m_bCHRInversion)
    {
        CHR_Bank(
            m_uRegs[2],
            m_uRegs[3],
            m_uRegs[4],
            m_uRegs[5],
            m_uRegs[0],
            m_uRegs[0] | 1u,
            m_uRegs[1],
            m_uRegs[1] | 1u
        );
    }
    else
    {
        CHR_Bank(
            m_uRegs[0],
            m_uRegs[0] | 1u,
            m_uRegs[1],
            m_uRegs[1] | 1u,
            m_uRegs[2],
            m_uRegs[3],
            m_uRegs[4],
            m_uRegs[5]
        );
    }

    // PRG
    if(m_uPRGBankMode)
    {
        PRG_Bank(
            BanksOfPRGIn8K()-2,
            m_uRegs[7],
            m_uRegs[6],
            BanksOfPRGIn8K()-1
        );
    }
    else
    {
        PRG_Bank(
            m_uRegs[6],
            m_uRegs[7],
            BanksOfPRGIn8K()-2,
            BanksOfPRGIn8K()-1
        );
    }
}

void CMapper004::OnBankSelect(const uint8_t &data)
{
    // CHR是否倒置
    m_bCHRInversion = (data & 0x80u) != 0u;

    // PRG模式
    m_uPRGBankMode = (data & 0x40u) != 0u;

    // 选择哪一个寄存器
    m_uRegSelector = data & 0x7u;
}

void CMapper004::OnBankData(const uint8_t& data)
{
    m_uRegs[m_uRegSelector] = data;
    UpdateBanks();
}

void CMapper004::OnChangeMirroring(const uint8_t &data)
{
    // This bit has no effect on cartridges with hardwired 4-screen VRAM
    if(m_pRefNesHeader->HasFourScreenVRAM())
    {
        return;
    }

    // 0: vertical; 1: horizontal
    m_uCustomMirrorMode = data & 1u;
    if(data & 1u)
    {
        m_pfNameTableMirroring = &CBaseMapper::NT_H_Mirroring;
    }
    else
    {
        m_pfNameTableMirroring = &CBaseMapper::NT_V_Mirroring;
    }
}

void CMapper004::OnChangePRGProtecting(const uint8_t &data)
{
    // permanently disable WRAM when 4-screen
    if(m_pRefNesHeader->HasFourScreenVRAM()) return;

    SetPRGRAMEnabled((data & 0x80u) != 0u);
    m_bPRGWriteProtected = ((data & 0x40u) != 0u);
}

void CMapper004::DisableIRQ()
{
    m_bIRQEnabled = false;
    m_pRefMainBoard->CPUSignal(SIGNAL_CANCEL_IRQ);
}

void CMapper004::EnableIRQ()
{
    m_bIRQEnabled = true;
}
