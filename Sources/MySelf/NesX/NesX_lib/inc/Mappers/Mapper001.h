
#ifndef NESX_NESX_LIB_INC_MAPPERS_MAPPER001_H_
#define NESX_NESX_LIB_INC_MAPPERS_MAPPER001_H_

#include "Core/BaseMapper.h"

// https://wiki.nesdev.com/w/index.php/MMC1
// http://kevtris.org/mappers/mmc1/index.html
// http://nesdev.com/mmc1.txt

class CMapper001 : public CBaseMapper
{
public:
    CMapper001();
    ~CMapper001() override;

    void SaveSnapshot(MapperSnapshot& state) override;
    void LoadSnapshot(const MapperSnapshot& state) override;


    void Quit() override;

    bool WritePRG(const Address& address, const uint8_t &data) override;

private:
    void Reset() override;

    // 外部寄存器 ==> (内部寄存器)
    void OnHandleLoadRegister(const Address& address, const uint8_t& value);

    // 内部寄存器
    void UpdateInternalRegisters(const Address& address, const uint8_t& value);

    // 设置函数(放置重复设置)
    void SetControlRegister(const uint8_t& value);
    void SetCHR0Register(const uint8_t& value);
    void SetCHR1Register(const uint8_t& value);
    void SetPRGRegister(const uint8_t& value);

    // 上面设置函数的更新实现
    void _UpdateMirror();
    void _UpdateCHR0();
    void _UpdateCHR1();
    void _UpdatePRGRegister();

    // 交换函数的单独实现
    void SwitchingPRGBanks();
    void SwitchingCHR0();
    void SwitchingCHR1();

    // 是否为8KB模式
    bool Is8KBCHRBankMode() const;

    // 移位寄存器(临时)
    bool IsSRFull() const;
    void ShiftIntoSR(const uint8_t& value);
    void ClearSR();

    // 强制更新所有
    void ForceUpdate();

    uint8_t m_uTmpSR; //shift register

    // internal registers
    uint8_t m_uIRControl;
    uint8_t m_uIRCHR0;
    uint8_t m_uIRCHR1;
    uint8_t m_uIRPRG;

    // 选择PRG行 (是在0x00-0x0F 还是 0x10-0x1F)
    uint8_t m_uPRGRowSelect;

    // 变种判断
    bool m_bSUROM;
};

#endif //NESX_NESX_LIB_INC_MAPPERS_MAPPER001_H_
