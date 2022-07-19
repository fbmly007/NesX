
#ifndef NESX_NESX_LIB_INC_UNITS_CPU_H_
#define NESX_NESX_LIB_INC_UNITS_CPU_H_

#include "Units/Unit.h"
#include "Snapshots/CPUSnapshot.h"
#include "Config.h"

class CCPU : public CUnit
{
    typedef uint16_t (CCPU::*AddressModeFunc)();

public:
    CCPU();

    ~CCPU();

    void SaveSnapshot(CPUSnapshot& state);
    void LoadSnapshot(const CPUSnapshot& state);

    bool Init(CMainBoard *pRefMainBoard) override;

    void Quit() override;

    void PowerUp() override;

    void Reset() override;

    void PowerDown() override;

    void Signal(const uint8_t& signal);

    uint8_t CountOfWrite() const;

    uint8_t DMC_Read(const Address& address);

    // Virtual
protected:
    void Tick() override;

    int Divider() const override;

    // Self
private:
    void Step();

    void ExecuteOpCode();

    void HandleNMI();

    void HandleIRQ();

    void ExecuteDMA(const uint8_t& data);

private:
    uint8_t Read(const Address &address);

    bool Write(const Address &address, const uint8_t &data);

    void Push(uint8_t value);

    void PushSR();

    uint8_t Pop();

    // 标志寄存器操作
    void UpdateC(uint16_t r);

    void UpdateV(uint8_t x, uint8_t y, uint16_t r);

    void UpdateN(uint8_t x);

    void UpdateZ(uint8_t x);

    // 寻址模式
    uint16_t IMM();

    uint16_t ZPG();

    uint16_t ZPGX();

    uint16_t ZPGY();

    uint16_t ABS();

    uint16_t ABSX();

    uint16_t ABSY();

    uint16_t IND();

    uint16_t XIND();

    uint16_t INDY();

    // 指令
    void BRK();

    void ADC(AddressModeFunc pFunc);

    void AND(AddressModeFunc pFunc);

    void ASLA();

    void ASL(AddressModeFunc pFunc);

    void BR(bool bCond);

    void BIT(AddressModeFunc pFunc);

    void CLC();

    void CLD();

    void CLI();

    void CLV();

    void CMP(AddressModeFunc pFunc);

    void CPX(AddressModeFunc pFunc);

    void CPY(AddressModeFunc pFunc);

    void DEC(AddressModeFunc pFunc);

    void DEX();

    void DEY();

    void EOR(AddressModeFunc pFunc);

    void INC(AddressModeFunc pFunc);

    void INX();

    void INY();

    void JMP(AddressModeFunc pFunc);

    void JSR();

    void LDA(AddressModeFunc pFunc);

    void LDX(AddressModeFunc pFunc);

    void LDY(AddressModeFunc pFunc);

    void LSRA();

    void LSR(AddressModeFunc pFunc);

    void NOP();

    void ORA(AddressModeFunc pFunc);

    void PHA();

    void PHP();

    void PLA();

    void PLP();

    void ROLA();

    void ROL(AddressModeFunc pFunc);

    void RORA();

    void ROR(AddressModeFunc pFunc);

    void RTI();

    void RTS();

    void SBC(AddressModeFunc pFunc);

    void SEC();

    void SED();

    void SEI();

    void STA(AddressModeFunc pFunc);

    void STX(AddressModeFunc pFunc);

    void STY(AddressModeFunc pFunc);

    void TAX();

    void TAY();

    void TSX();

    void TXA();

    void TXS();

    void TYA();

    // 非官方指令(新出现的)
    void STP();
    void SLO(AddressModeFunc pFunc);
    void NOP(AddressModeFunc pFunc);
    void ANC(AddressModeFunc pFunc);
    void RLA(AddressModeFunc pFunc);
    void SRE(AddressModeFunc pFunc);
    void ASR(AddressModeFunc pFunc);
    void RRA(AddressModeFunc pFunc);
    void ARR(AddressModeFunc pFunc);
    void SAX(AddressModeFunc pFunc);
    void XAA();
    void SHA(AddressModeFunc pFunc);
    void XAS();
    void SYA();
    void SXA();
    void LAX(AddressModeFunc pFunc);
    void LAS();
    void DCP(AddressModeFunc pFunc);
    void AXS(AddressModeFunc pFunc);
    void ISC(AddressModeFunc pFunc);

    // 部分指令的单独算法部分
    uint8_t DO_ASL(const Address& addr, uint8_t m, AddressModeFunc pFunc);
    void DO_ORA(const Address& addr, const uint8_t& m);
    uint8_t DO_ROL(const Address& addr, uint8_t m, AddressModeFunc pFunc);
    void DO_AND(const Address& addr, const uint8_t& m);
    uint8_t DO_LSR(const Address& addr, uint8_t m, AddressModeFunc pFunc);
    void DO_EOR(const Address& addr, const uint8_t& m);
    uint8_t DO_ROR(const Address& addr, uint8_t m, AddressModeFunc pFunc);
    void DO_ADC(const Address& addr, const uint8_t& m);
    void DO_LDA(const Address& addr, const uint8_t& m);
    void DO_LDX(const Address& addr, const uint8_t& m);
    uint8_t DO_DEC(const Address& addr, uint8_t m, AddressModeFunc pFunc);
    void DO_CPXX(const uint8_t& reg, const uint8_t& m);
    uint8_t DO_INC(const Address& addr, uint8_t m, AddressModeFunc pFunc);
    void DO_SBC(const Address& addr, uint8_t m);

private:
    int m_nCyclesToWait;

    // 内存
    uint8_t m_InternalRAM[0x800u];

    // 内部寄存器
    DataReg PC;  // Program counter (16bit)
    uint8_t A;   // Accumulator      (8bit)
    uint8_t X;   // X register       (8bit)
    uint8_t Y;   // Y register       (8bit)
    Flags SR;    // Status Register  (8bit)
    uint8_t SP;  // Stack Pointer    (8bit)
    bool m_bNMI;
    bool m_bIRQ;
    bool m_bCycleOdd;

    bool m_bCross;
    uint8_t m_uCountOfWrite; // 同一个指令下连续写的个数
};

#endif //NESX_NESX_LIB_INC_UNITS_CPU_H_
