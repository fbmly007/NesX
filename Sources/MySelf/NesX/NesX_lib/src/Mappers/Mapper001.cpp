
#include "Mappers/Mapper001.h"
#include "Core/NesHeader.h"
#include "Units/MainBoard.h"

CMapper001::CMapper001()
        : m_uTmpSR(0),
          m_uIRControl(0),
          m_uIRCHR0(0),
          m_uIRCHR1(0),
          m_uIRPRG(0),
          m_bSUROM(false),
          m_uPRGRowSelect(0u)
{
    ClearSR();
}

CMapper001::~CMapper001()
{
    Quit();
    ClearSR();
}

void CMapper001::SaveSnapshot(MapperSnapshot &state)
{
    CBaseMapper::SaveSnapshot(state);

    state.extraData[0] = m_uTmpSR;
    state.extraData[1] = m_uIRControl;
    state.extraData[2] = m_uIRCHR0;
    state.extraData[3] = m_uIRCHR1;
    state.extraData[4] = m_uIRPRG;
    state.extraData[5] = m_uPRGRowSelect;
    state.extraData[6] = m_bSUROM;
}

void CMapper001::LoadSnapshot(const MapperSnapshot &state)
{
    CBaseMapper::LoadSnapshot(state);

    m_uTmpSR = state.extraData[0];
    m_uIRControl = state.extraData[1];
    m_uIRCHR0 = state.extraData[2];
    m_uIRCHR1 = state.extraData[3];
    m_uIRPRG = state.extraData[4];
    m_uPRGRowSelect = state.extraData[5];
    m_bSUROM = state.extraData[6];

    // ForceUpdate();
    _UpdateMirror();
}

void CMapper001::Quit()
{
    CBaseMapper::Quit();
}

void CMapper001::Reset()
{
    ClearSR();
    // 判断是否为SUROM
    m_bSUROM = BanksOfPRGIn16K() == 0x20u;
    SetControlRegister(0xCu);
    SetPRGRAMEnabled(true);
}

bool CMapper001::WritePRG(const Address &address, const uint8_t &data)
{
    if(address >= 0x8000u && address <= 0xFFFFu)
    {
        OnHandleLoadRegister(address, data);
    }
    return CBaseMapper::WritePRG(address, data);
}

void CMapper001::OnHandleLoadRegister(const Address& address, const uint8_t& value)
{
    // RMW忽略连续写周期
    if(m_pRefMainBoard->CPUCountOfWrite() > 1)
        return;

    //  Load register
    //  7  bit  0
    //  ---- ----
    //  Rxxx xxxD
    //  |       |
    //  |       +- Data bit to be shifted into shift register, LSB first
    //  +--------- 1: Reset shift register and write Control with (Control OR $0C),
    //  locking PRG ROM at $C000-$FFFF to the last bank.
    if(value & 0x80u)
    {
        // 重置移位寄存器
        ClearSR();

        // 将Control内部状态位或上0xC
        SetControlRegister(m_uIRControl | 0xCu);
        return;
    }

    // SR满了
    if(IsSRFull())
    {
        // 再移动一次组成一个5位的最终值
        ShiftIntoSR(value);

        // 获取到最终的值 并且 重置SR
        uint8_t registerValue = m_uTmpSR;
        ClearSR();

        // 将寄存器值传给对应的寄存器
        UpdateInternalRegisters(address, registerValue);
        return;
    }

    // 将最低位移入SR移位寄存器中
    ShiftIntoSR(value);
}

void CMapper001::UpdateInternalRegisters(const Address &address, const uint8_t &value)
{
    // 根据地址二进制位13和14来选择寄存器
    uint8_t regType = (address >> 13u) & 0x3u;
    switch(regType)
    {
        case 0: return SetControlRegister(value);

        case 1: return SetCHR0Register(value);

        case 2: return SetCHR1Register(value);

        case 3: return SetPRGRegister(value);

        default: return;
    }
}

void CMapper001::SetControlRegister(const uint8_t &value)
{
    if(value == m_uIRControl) return;
    m_uIRControl = value;

    // 设置Mirroring指针
    _UpdateMirror();
}

void CMapper001::SetCHR0Register(const uint8_t &value)
{
    if(value == m_uIRCHR0) return;
    m_uIRCHR0 = value;
    _UpdateCHR0();
}

void CMapper001::SetCHR1Register(const uint8_t &value)
{
    if(value == m_uIRCHR1) return;
    m_uIRCHR1 = value;
    _UpdateCHR1();
}

void CMapper001::SetPRGRegister(const uint8_t &value)
{
    if(value == m_uIRPRG) return;
    m_uIRPRG = value;
    _UpdatePRGRegister();
}

void CMapper001::_UpdateMirror()
{
    switch(m_uIRControl & 0x3u)
    {
        case 0: // one-screen, lower bank
            m_pfNameTableMirroring = &CBaseMapper::NT_1L_Mirroring;
            break;

        case 1: // one-screen, upper bank
            m_pfNameTableMirroring = &CBaseMapper::NT_1U_Mirroring;
            break;

        case 2: // vertical
            m_pfNameTableMirroring = &CBaseMapper::NT_V_Mirroring;
            break;

        case 3: // horizontal
            m_pfNameTableMirroring = &CBaseMapper::NT_H_Mirroring;
            break;

        default: break;
    }
}

void CMapper001::_UpdateCHR0()
{
    SwitchingCHR0();
}

void CMapper001::_UpdateCHR1()
{
    SwitchingCHR1();
}

void CMapper001::_UpdatePRGRegister()
{
    SetPRGRAMEnabled((m_uIRPRG & 0x10u) == 0u);
    SwitchingPRGBanks();
}

void CMapper001::SwitchingPRGBanks()
{
    // PRG切换模式
    uint8_t uPRGBankMode = (m_uIRControl & 0xCu) >> 2u;

    // 获取行选择索引(0x00-0x0F或者0x10-0x1F), 目前除了SUROM 其余板子任何时候都是0
    uint8_t prgRowSelect = m_uPRGRowSelect;

    // 组成bankNumber
    uint8_t bankNumber = (m_uIRPRG & 0xFu) | prgRowSelect;

    switch(uPRGBankMode)
    {
        // 0, 1: switch 32 KB at $8000, ignoring low bit of bank number
        case 0: case 1:
            bankNumber &= ~1u;          // 保证最低位0是0, 也就是偶数
            SelectPRGBankIn16K(0u, bankNumber);
            SelectPRGBankIn16K(1u, bankNumber | 0x1u);
            break;

            // 2: fix first bank at $8000 and switch 16 KB bank at $C000
        case 2:
            SelectPRGBankIn16K(0, prgRowSelect); // prgRowSelect + 0
            SelectPRGBankIn16K(1, bankNumber);
            break;

            // 3: fix last bank at $C000 and switch 16 KB bank at $8000
        case 3:
            SelectPRGBankIn16K(0, bankNumber);
            SelectPRGBankIn16K(1, prgRowSelect | 0xFu); // 保证是当前偏移行的最后一列(页)
            break;

        default: break;
    }
}

void CMapper001::SwitchingCHR0()
{
    if(m_bSUROM)
    {
        // 从CHR0处偷取一位(其实是共用)
        m_uPRGRowSelect = m_uIRCHR0 & 0x10u;
        SwitchingPRGBanks();
    }

    if(Is8KBCHRBankMode())
    {
        // switch 8 KB at a time
        uint8_t bankNumber = m_uIRCHR0 & 0x1Eu;
        SelectCHRBankIn8K(bankNumber);
    }
    else
    {
        // switch two separate 4 KB banks
        SelectCHRBankIn4K(0, m_uIRCHR0 & 0x1Fu);
    }
}

void CMapper001::SwitchingCHR1()
{
    // ignored in 8 KB mode
    if(Is8KBCHRBankMode()) return;

    if(m_bSUROM)
    {
        // 从CHR1处偷取一位(其实是共用)
        m_uPRGRowSelect = m_uIRCHR1 & 0x10u;
        SwitchingPRGBanks();
    }

    // Select 4 KB CHR bank at PPU $1000
    SelectCHRBankIn4K(1, m_uIRCHR1 & 0x1Fu);
}

bool CMapper001::Is8KBCHRBankMode() const
{
    return (m_uIRControl & 0x10u) == 0u;
}

bool CMapper001::IsSRFull() const
{
    return (m_uTmpSR & 0x1u) != 0;
}

void CMapper001::ShiftIntoSR(const uint8_t &value)
{
    m_uTmpSR >>= 1u;
    if(value & 1u)
    {
        m_uTmpSR |= 0x10u;
    }
}

void CMapper001::ClearSR()
{
    m_uTmpSR = 0x10u;
}


// 强制更新
void CMapper001::ForceUpdate()
{
    _UpdateMirror();
    _UpdateCHR0();
    _UpdateCHR1();
    _UpdatePRGRegister();
}


