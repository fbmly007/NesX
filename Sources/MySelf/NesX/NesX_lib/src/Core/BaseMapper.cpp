
#include "Core/BaseMapper.h"
#include "Core/NesHeader.h"
#include <cassert>
#include <cstring>

CBaseMapper::CBaseMapper()
    : m_pRefMainBoard(nullptr),
      m_pRefNesHeader(nullptr),

      m_pPRGBanks{},
      m_cachedPrgBanks{},
      m_pCHRBanks{},
      m_cachedChrBanks{},

      m_pRawPRG(nullptr),
      m_uBanksOfPRG(0u),
      m_pRawCHR(nullptr),
      m_uBanksOfCHR(0u),
      m_SRAM{},

      m_pfNameTableMirroring(nullptr),

      m_bCHRRamEnabled(false),
      m_bSRAMEnabled(false)
{
}

void CBaseMapper::SaveSnapshot(MapperSnapshot& state)
{
    memcpy(state.prgBanks, this->m_cachedPrgBanks, sizeof(this->m_cachedPrgBanks));
    memcpy(state.chrBanks, this->m_cachedChrBanks, sizeof(this->m_cachedChrBanks));
    memcpy(state.sram, this->m_SRAM, sizeof(this->m_SRAM));
    state.customMirrorMode = 0;
    state.chrRamEnabled = this->m_bCHRRamEnabled;
    state.sramEnabled = this->m_bSRAMEnabled;

    if(state.chrRamEnabled)
    {
        // 如果是CHR-RAM的话, 需要保存起来
        memcpy(state.chrBankData, this->m_pRawCHR, sizeof(state.chrBankData));
    }
}

void CBaseMapper::LoadSnapshot(const MapperSnapshot& state)
{
    // prg
    SelectPRGBankIn8K(0, state.prgBanks[0]);
    SelectPRGBankIn8K(1, state.prgBanks[1]);
    SelectPRGBankIn8K(2, state.prgBanks[2]);
    SelectPRGBankIn8K(3, state.prgBanks[3]);

    // chr
    SelectCHRBankIn1K(0, state.chrBanks[0]);
    SelectCHRBankIn1K(1, state.chrBanks[1]);
    SelectCHRBankIn1K(2, state.chrBanks[2]);
    SelectCHRBankIn1K(3, state.chrBanks[3]);
    SelectCHRBankIn1K(4, state.chrBanks[4]);
    SelectCHRBankIn1K(5, state.chrBanks[5]);
    SelectCHRBankIn1K(6, state.chrBanks[6]);
    SelectCHRBankIn1K(7, state.chrBanks[7]);

    memcpy(this->m_SRAM, state.sram, sizeof(this->m_SRAM));
    this->m_bCHRRamEnabled = state.chrRamEnabled;
    this->m_bSRAMEnabled = state.sramEnabled;

    if(state.chrRamEnabled)
    {
        // 如果是CHR-RAM的话, 需要保存起来
        memcpy(this->m_pRawCHR, state.chrBankData, sizeof(state.chrBankData));
    }
}

bool CBaseMapper::Init(CMainBoard *pRefMainBoard, CNesHeader *pRefNesHeader, istream &ifs)
{
    m_pRefMainBoard = pRefMainBoard;
    m_pRefNesHeader = pRefNesHeader;

    InitRawPRG(ifs);
    InitRawCHR(ifs);
    InitSRAM();
    InitDefaultBanks();
    InitCommonNTMirroring();

    OnInit();
    return true;
}

// FIXME: 必须在子类的析构函数自行调用 Quit 函数
void CBaseMapper::Quit()
{
    m_pRefMainBoard = nullptr;
    m_pRefNesHeader = nullptr;

    m_pfNameTableMirroring = nullptr;

    SAFE_ARRAY_RELEASE(m_pRawPRG)
    m_uBanksOfPRG = 0u;
    SAFE_ARRAY_RELEASE(m_pRawCHR)
    m_uBanksOfCHR = 0u;
    m_bCHRRamEnabled = false;
    m_bSRAMEnabled = false;
}

void CBaseMapper::PowerUp()
{
    // 多数情况下 PowerUp会调用Reset就足够了
    Reset();
}

void CBaseMapper::Reset()
{
}

void CBaseMapper::PowerDown()
{
}

Address CBaseMapper::NameTableMirroring(const Address &address)
{
    return (*m_pfNameTableMirroring)(address);
}

uint8_t CBaseMapper::ReadPRG(const Address &address)
{
    switch(address)
    {
        // TODO: Registers for APU and Expanded ROM
        case 0x4018u ... 0x5FFFu: break;

        // SRAM
        case 0x6000u ... 0x7FFFu:
            if(m_bSRAMEnabled)
            {
                return m_SRAM[address & 0x1FFFu];
            }
            break;

        // PRG-ROM [0x8000, 0xFFFF]
        case 0x8000u ... 0xFFFFu:
            return m_pPRGBanks[(address >> 13u) & 3u][address & 0x1FFFu];

        default: break;
    }

    // cpu open bus
    return (address >> 8u) & 0xFFu;
}

bool CBaseMapper::WritePRG(const Address &address, const uint8_t &data)
{
    if(address >= 0x6000u && address <= 0x7FFFu)
    {
        // SRAM
        if (m_bSRAMEnabled)
        {
            m_SRAM[ address & 0x1FFFu ] = data;
            return true;
        }
    }
    // ROM (read-only default)
    return false;
}

uint8_t CBaseMapper::ReadCHR(const Address &address)
{
    if(address >= 0x0000 && address <= 0x1FFFu)
    {
        return m_pCHRBanks[address >> 10u][address & 0x3FFu];
    }
    return 0u;
}

bool CBaseMapper::WriteCHR(const Address &address, const uint8_t &data)
{
    if(!m_bCHRRamEnabled) return false;

    if(address >= 0x0000 && address <= 0x1FFFu)
    {
        m_pCHRBanks[address >> 10u][address & 0x3FFu] = data;
        return true;
    }

    return false;
}

void CBaseMapper::DataSignal(const uint8_t& signal, const Address& data)
{
}

bool CBaseMapper::OnInit()
{
    return true;
}

void CBaseMapper::InitRawPRG(istream& ifs)
{
    assert(m_pRefNesHeader);
    assert(!m_pRawPRG);

    // 计算Banks的个数
    m_uBanksOfPRG = m_pRefNesHeader->NumberOfPRG() << 1u;

    size_t size = (size_t)m_pRefNesHeader->NumberOfPRG() << 14u;

    m_pRawPRG = new uint8_t[size];
    memset(m_pRawPRG, 0, size);

    // 将所有PRG内容读进来
    ifs.read((char *) m_pRawPRG, size);
}

void CBaseMapper::InitRawCHR(istream &ifs)
{
    assert(m_pRefNesHeader);
    assert(!m_pRawCHR);

    // 使用CHR-RAM, 默认为8KB
    m_bCHRRamEnabled = m_pRefNesHeader->NumberOfCHR() == 0u;

    // 计算size
    size_t size;
    if(m_bCHRRamEnabled)
    {
        size = 0x2000u;
        m_uBanksOfCHR = 8;
    }
    else
    {
        size = (size_t)m_pRefNesHeader->NumberOfCHR() << 13u;
        m_uBanksOfCHR = m_pRefNesHeader->NumberOfCHR() << 3u;
    }

    // 开辟CHR内存
    m_pRawCHR = new uint8_t[size];
    memset(m_pRawCHR, 0, size);

    if(!m_bCHRRamEnabled)
    {
        // 将所有CHR内容读进来
        ifs.read((char *)m_pRawCHR, size);
    }
}

void CBaseMapper::InitSRAM()
{
    m_bSRAMEnabled = m_pRefNesHeader->HasBettery();

    if(m_pRefNesHeader->HasTrainer())
    {
        // 将trainer数据拷贝到[0x7000,0x71FF]的位置
        string trainData;
        m_pRefNesHeader->GetTrainer(trainData);
        memcpy(m_SRAM+0x1000u, trainData.data(), trainData.size());
    }
}

void CBaseMapper::InitDefaultBanks()
{
    // 初始化PRG Bank指针
    SelectPRGBankIn16K(0, 0);
    SelectPRGBankIn16K(1, BanksOfPRGIn16K()-1u);

    // 初始化CHR Bank指针
    SelectCHRBankIn8K(0);
}

void CBaseMapper::InitCommonNTMirroring()
{
    if(m_pRefNesHeader->HasFourScreenVRAM())
    {
        m_pfNameTableMirroring = &NT_NO_Mirroring;
    }
    else if(m_pRefNesHeader->HasVerticalMirroring())
    {
        m_pfNameTableMirroring = &NT_V_Mirroring;
    }
    else
    {
        m_pfNameTableMirroring = &NT_H_Mirroring;
    }
}

size_t CBaseMapper::BanksOfPRGIn8K() const
{
    return m_uBanksOfPRG;
}

size_t CBaseMapper::BanksOfPRGIn16K() const
{
    return m_uBanksOfPRG >> 1u;
}

size_t CBaseMapper::BanksOfPRGIn32K() const
{
    return m_uBanksOfPRG >> 2u;
}

size_t CBaseMapper::BanksOfCHRIn1K() const
{
    return m_uBanksOfCHR;
}

size_t CBaseMapper::BanksOfCHRIn2K() const
{
    return m_uBanksOfCHR >> 1u;
}

size_t CBaseMapper::BanksOfCHRIn4K() const
{
    return m_uBanksOfCHR >> 2u;
}

size_t CBaseMapper::BanksOfCHRIn8K() const
{
    return m_uBanksOfCHR >> 3u;
}

void CBaseMapper::SetPRGRAMEnabled(bool bEnabled)
{
    m_bSRAMEnabled = bEnabled;
}

void CBaseMapper::SelectCHRBankIn8K(const size_t &uBank)
{
    // slot 0 = [0000, 1FFF]
    SelectCHRBankIn4K(0, uBank + uBank);
    SelectCHRBankIn4K(1, uBank + uBank + 1u);
}

void CBaseMapper::SelectCHRBankIn4K(const int& slot, const size_t &uBank)
{
    // slot 0 = [0000, 0FFF]
    // slot 1 = [1000, 1FFF]
    SelectCHRBankIn2K(slot + slot, uBank + uBank);
    SelectCHRBankIn2K(slot + slot + 1, uBank + uBank + 1u);
}

void CBaseMapper::SelectCHRBankIn2K(const int& slot, const size_t &uBank)
{
    // slot 0 = [0000, 07FF]
    // slot 1 = [0800, 0FFF]
    // slot 2 = [1000, 17FF]
    // slot 3 = [1800, 1FFF]
    SelectCHRBankIn1K(slot + slot, uBank + uBank);
    SelectCHRBankIn1K(slot + slot + 1, uBank + uBank + 1u);
}

void CBaseMapper::SelectCHRBankIn1K(const int& slot, const size_t &uBank)
{
    // slot 0 = [0000, 03FF]
    // slot 1 = [0400, 07FF]
    // slot 2 = [0800, 0BFF]
    // slot 3 = [0C00, 0FFF]
    // slot 4 = [1000, 13FF]
    // slot 5 = [1400, 17FF]
    // slot 6 = [1800, 1BFF]
    // slot 7 = [1C00, 1FFF]
    assert(slot >= 0 && slot < 8);
    m_pCHRBanks[slot] = m_pRawCHR + ((uBank % BanksOfCHRIn1K()) << 10u);
    m_cachedChrBanks[slot] = (uint8_t)uBank;
    // printf("slot %d = %lu\n", slot, uBank);
}

void CBaseMapper::SelectPRGBankIn32K(const size_t &uBank)
{
    // slot 0 = [8000, FFFF]
    SelectPRGBankIn16K(0, uBank + uBank);
    SelectPRGBankIn16K(1, uBank + uBank + 1u);
}

void CBaseMapper::SelectPRGBankIn16K(const int &slot, const size_t &uBank)
{
    // slot 0 = [8000, BFFF]
    // slot 1 = [C000, FFFF]
    SelectPRGBankIn8K(slot + slot, uBank + uBank);
    SelectPRGBankIn8K(slot + slot + 1, uBank + uBank + 1u);
}

void CBaseMapper::SelectPRGBankIn8K(const int &slot, const size_t &uBank)
{
    // slot 0 = [8000, 9FFF]
    // slot 1 = [A000, BFFF]
    // slot 2 = [C000, DFFF]
    // slot 3 = [E000, FFFF]
    assert(slot >= 0 && slot < 4);
    m_pPRGBanks[slot] = m_pRawPRG + ((uBank % BanksOfPRGIn8K()) << 13u);
    m_cachedPrgBanks[slot] = (uint8_t)uBank;
}


Address CBaseMapper::NT_H_Mirroring(const Address &address)
{
    // Horizontal mirroring: $2000 equals $2400 and $2800 equals $2C00 (e.g. Kid Icarus)
    // PPU内部的物理内存只有2KB(额外的2KB是卡带提供的),所以
    // $2000,$2400对应前面的1KB (+0x000)
    // $2800,$2C00对应后面的1KB (+0x400)
    // 因此还是按照物理连续的方式映射好(对现有代码没有影响, 因为PPU中额外定义的2KB是给4-screen模式准备的)
    // TODO: 为什么必须要映射到 +0x400 这个位置 才能显示正确, 还未研究清楚!
    return ((address >> 1u) & 0x400u) | (address & 0x3FFu);
}

Address CBaseMapper::NT_V_Mirroring(const Address &address)
{
    // Vertical mirroring: $2000 equals $2800 and $2400 equals $2C00 (e.g. Super Mario Bros.)
    return address & 0x7FFu;
}

// one-screen, lower bank
Address CBaseMapper::NT_1L_Mirroring(const Address &address)
{
    return address & 0x3FFu;
}

// one-screen, upper bank
Address CBaseMapper::NT_1U_Mirroring(const Address &address)
{
    return (address & 0x3FFu) | 0x400u;
}

Address CBaseMapper::NT_NO_Mirroring(const Address &address)
{
    // CIRAM is disabled
    return address;
}






