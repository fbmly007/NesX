
#include "Units/CPU.h"
#include "Units/MainBoard.h"
#include "Common.h"
#include <cassert>

// 定义在此处的变量只是临时变量, 不是状态变量
static DataReg s_TempReg;

CCPU::CCPU()
        : m_InternalRAM{},
          m_nCyclesToWait(0),

          A(0u),
          X(0u),
          Y(0u),
          SP(0u),
          m_bNMI(false),
          m_bIRQ(false),
          m_bCross(false),
          m_uCountOfWrite(0),
          m_bCycleOdd(true)
{
	SR.Value = 0u;
	PC.D = 0u;
}

CCPU::~CCPU()
{
    Quit();
}

void CCPU::SaveSnapshot(CPUSnapshot& state)
{
    state.cyclesToWait = m_nCyclesToWait;
    memcpy(state.ram, m_InternalRAM, sizeof(m_InternalRAM));
    state.PC = PC.W;
    state.A = A;
    state.X = X;
    state.Y = Y;
    state.SR = SR.Value;
    state.SP = SP;
    state.nmi = m_bNMI;
    state.irq = m_bIRQ;
    state.cycleOdd = m_bCycleOdd;
    state.cross = m_bCross;
    state.countOfWrite = m_uCountOfWrite;
}

void CCPU::LoadSnapshot(const CPUSnapshot& state)
{
    this->m_nCyclesToWait = state.cyclesToWait;
    memcpy(this->m_InternalRAM, state.ram, sizeof(this->m_InternalRAM));
    this->PC.W = state.PC;
    this->A = state.A;
    this->X = state.X;
    this->Y = state.Y;
    this->SR.Value = state.SR;
    this->SP = state.SP;
    this->m_bNMI = state.nmi;
    this->m_bIRQ = state.irq;
    this->m_bCycleOdd = state.cycleOdd;
    this->m_bCross = state.cross;
    this->m_uCountOfWrite = state.countOfWrite;
}

bool CCPU::Init(CMainBoard *pRefMainBoard)
{
    return CUnit::Init(pRefMainBoard);
}

void CCPU::Quit()
{
    CUnit::Quit();
}

void CCPU::PowerUp()
{
    memset(m_InternalRAM, -1, 0x800u);

    // 初始化寄存器
    SR.I = SR.B = SR.X = 1u;
    A = X = Y = 0u;
    SP = 0xFDu;

    // 设置入口点
    PC.D = 0u;
    PC.WL = m_pRefMainBoard->CartridgeReadPRG(RESET_INT_HANDLER);
    PC.WH = m_pRefMainBoard->CartridgeReadPRG(RESET_INT_HANDLER + 1u);

    // FIXME ?
    m_bNMI = false;
    m_bIRQ = false;

    m_bCycleOdd = true;

    m_nCyclesToWait = 7;
}

void CCPU::Reset()
{
    SR.X = 0u;
    SP -= 3;
    SR.I = 1u;

    // 设置入口点
    PC.D = 0u;
    PC.WL = m_pRefMainBoard->CartridgeReadPRG(RESET_INT_HANDLER);
    PC.WH = m_pRefMainBoard->CartridgeReadPRG(RESET_INT_HANDLER + 1u);

    // FIXME ?
    m_bNMI = false;
    m_bIRQ = false;

    m_bCycleOdd = true;

    //The internal memory was unchanged
    //APU mode in $4017 was unchanged
    //APU was silenced ($4015 = 0)
    //APU triangle phase is reset to 0 (i.e. outputs a value of 15, the first step of its waveform)
    //APU DPCM output ANDed with 1 (upper 6 bits cleared)
    //2A03G: APU Frame Counter reset. (but 2A03letterless: APU frame counter retains old value) [6]

    m_nCyclesToWait = 7;
}

void CCPU::PowerDown()
{
}

void CCPU::Signal(const uint8_t &signal)
{
    switch(signal)
    {
        case SIGNAL_REQUEST_NMI:
            m_bNMI = true;
            break;

        case SIGNAL_CANCEL_NMI:
            m_bNMI = false;
            break;

        case SIGNAL_REQUEST_IRQ:
            m_bIRQ = true;
            break;

        case SIGNAL_CANCEL_IRQ:
            m_bIRQ = false;
            break;

        default: break;
    }
}

uint8_t CCPU::CountOfWrite() const
{
    return m_uCountOfWrite;
}

uint8_t CCPU::DMC_Read(const Address& address)
{
    return Read(address);
}

void CCPU::Tick()
{
    m_bCycleOdd = !m_bCycleOdd;
    if (m_nCyclesToWait > 0 && --m_nCyclesToWait)
        return;
    Step();
}

int CCPU::Divider() const
{
    return 3;
}

void CCPU::Step()
{
    ExecuteOpCode();
    if (m_bNMI)
    {
        HandleNMI();
    }
    else if (m_bIRQ && !SR.I)
    {
        HandleIRQ();
    }
}

void CCPU::ExecuteOpCode()
{
    m_uCountOfWrite = 0u;

    uint8_t uOpCode = Read(PC.W++);

    // https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes

    switch(uOpCode)
    {
        case 0x00: return BRK();                    case 0x01: return ORA(&CCPU::XIND);
        case 0x02: return STP();                   case 0x03: return SLO(&CCPU::XIND);
        case 0x04: return NOP(&CCPU::ZPG);          case 0x05: return ORA(&CCPU::ZPG);
        case 0x06: return ASL(&CCPU::ZPG);          case 0x07: return SLO(&CCPU::ZPG);
        case 0x08: return PHP();                    case 0x09: return ORA(&CCPU::IMM);
        case 0x0A: return ASLA();                   case 0x0B: return ANC(&CCPU::IMM);
        case 0x0C: return NOP(&CCPU::ABS);          case 0x0D: return ORA(&CCPU::ABS);
        case 0x0E: return ASL(&CCPU::ABS);          case 0x0F: return SLO(&CCPU::ABS);
        case 0x10: return BR(!SR.N);         case 0x11: return ORA(&CCPU::INDY);
        case 0x12: return STP();                    case 0x13: return SLO(&CCPU::INDY);
        case 0x14: return NOP(&CCPU::ZPGX);         case 0x15: return ORA(&CCPU::ZPGX);
        case 0x16: return ASL(&CCPU::ZPGX);         case 0x17: return SLO(&CCPU::ZPGX);
        case 0x18: return CLC();                    case 0x19: return ORA(&CCPU::ABSY);
        case 0x1A: return NOP();                   case 0x1B: return SLO(&CCPU::ABSY);
        case 0x1C: return NOP(&CCPU::ABSX);         case 0x1D: return ORA(&CCPU::ABSX);
        case 0x1E: return ASL(&CCPU::ABSX);         case 0x1F: return SLO(&CCPU::ABSX);
        case 0x20: return JSR();                    case 0x21: return AND(&CCPU::XIND);
        case 0x22: return STP();                    case 0x23: return RLA(&CCPU::XIND);
        case 0x24: return BIT(&CCPU::ZPG);          case 0x25: return AND(&CCPU::ZPG);
        case 0x26: return ROL(&CCPU::ZPG);          case 0x27: return RLA(&CCPU::ZPG);
        case 0x28: return PLP();                    case 0x29: return AND(&CCPU::IMM);
        case 0x2A: return ROLA();                   case 0x2B: return ANC(&CCPU::IMM);
        case 0x2C: return BIT(&CCPU::ABS);          case 0x2D: return AND(&CCPU::ABS);
        case 0x2E: return ROL(&CCPU::ABS);          case 0x2F: return RLA(&CCPU::ABS);
        case 0x30: return BR(SR.N);           case 0x31: return AND(&CCPU::INDY);
        case 0x32: return STP();                      case 0x33: return RLA(&CCPU::INDY);
        case 0x34: return NOP(&CCPU::ZPGX);          case 0x35: return AND(&CCPU::ZPGX);
        case 0x36: return ROL(&CCPU::ZPGX);         case 0x37: return RLA(&CCPU::ZPGX);
        case 0x38: return SEC();                    case 0x39: return AND(&CCPU::ABSY);
        case 0x3A: return NOP();                   case 0x3B: return RLA(&CCPU::ABSY);
        case 0x3C: return NOP(&CCPU::ABSX);         case 0x3D: return AND(&CCPU::ABSX);
        case 0x3E: return ROL(&CCPU::ABSX);         case 0x3F: return RLA(&CCPU::ABSX);
        case 0x40: return RTI();                    case 0x41: return EOR(&CCPU::XIND);
        case 0x42: return STP();                     case 0x43: return SRE(&CCPU::XIND);
        case 0x44: return NOP(&CCPU::ZPG);           case 0x45: return EOR(&CCPU::ZPG);
        case 0x46: return LSR(&CCPU::ZPG);          case 0x47: return SRE(&CCPU::ZPG);
        case 0x48: return PHA();                    case 0x49: return EOR(&CCPU::IMM);
        case 0x4A: return LSRA();                   case 0x4B: return ASR(&CCPU::IMM);
        case 0x4C: return JMP(&CCPU::ABS);          case 0x4D: return EOR(&CCPU::ABS);
        case 0x4E: return LSR(&CCPU::ABS);          case 0x4F: return SRE(&CCPU::ABS);
        case 0x50: return BR(!SR.V);          case 0x51: return EOR(&CCPU::INDY);
        case 0x52: return STP();                    case 0x53: return SRE(&CCPU::INDY);
        case 0x54: return NOP(&CCPU::ZPGX);         case 0x55: return EOR(&CCPU::ZPGX);
        case 0x56: return LSR(&CCPU::ZPGX);         case 0x57: return SRE(&CCPU::ZPGX);
        case 0x58: return CLI();                    case 0x59: return EOR(&CCPU::ABSY);
        case 0x5A: return NOP();                   case 0x5B: return SRE(&CCPU::ABSY);
        case 0x5C: return NOP(&CCPU::ABSX);         case 0x5D: return EOR(&CCPU::ABSX);
        case 0x5E: return LSR(&CCPU::ABSX);         case 0x5F: return SRE(&CCPU::ABSX);
        case 0x60: return RTS();                    case 0x61: return ADC(&CCPU::XIND);
        case 0x62: return STP();                    case 0x63: return RRA(&CCPU::XIND);
        case 0x64: return NOP(&CCPU::ZPG);          case 0x65: return ADC(&CCPU::ZPG);
        case 0x66: return ROR(&CCPU::ZPG);          case 0x67: return RRA(&CCPU::ZPG);
        case 0x68: return PLA();                    case 0x69: return ADC(&CCPU::IMM);
        case 0x6A: return RORA();                   case 0x6B: return ARR(&CCPU::IMM);
        case 0x6C: return JMP(&CCPU::IND);          case 0x6D: return ADC(&CCPU::ABS);
        case 0x6E: return ROR(&CCPU::ABS);          case 0x6F: return RRA(&CCPU::ABS);
        case 0x70: return BR(SR.V);          case 0x71: return ADC(&CCPU::INDY);
        case 0x72: return STP();                    case 0x73: return RRA(&CCPU::INDY);
        case 0x74: return NOP(&CCPU::ZPGX);         case 0x75: return ADC(&CCPU::ZPGX);
        case 0x76: return ROR(&CCPU::ZPGX);         case 0x77: return RRA(&CCPU::ZPGX);
        case 0x78: return SEI();                    case 0x79: return ADC(&CCPU::ABSY);
        case 0x7A: return NOP();                   case 0x7B: return RRA(&CCPU::ABSY);
        case 0x7C: return NOP(&CCPU::ABSX);         case 0x7D: return ADC(&CCPU::ABSX);
        case 0x7E: return ROR(&CCPU::ABSX);         case 0x7F: return RRA(&CCPU::ABSX);
        case 0x80: return NOP(&CCPU::IMM);          case 0x81: return STA(&CCPU::XIND);
        case 0x82: return NOP(&CCPU::IMM);          case 0x83: return SAX(&CCPU::XIND);
        case 0x84: return STY(&CCPU::ZPG);          case 0x85: return STA(&CCPU::ZPG);
        case 0x86: return STX(&CCPU::ZPG);          case 0x87: return SAX(&CCPU::ZPG);
        case 0x88: return DEY();                    case 0x89: return NOP(&CCPU::IMM);
        case 0x8A: return TXA();                    case 0x8B: return XAA();
        case 0x8C: return STY(&CCPU::ABS);          case 0x8D: return STA(&CCPU::ABS);
        case 0x8E: return STX(&CCPU::ABS);          case 0x8F: return SAX(&CCPU::ABS);
        case 0x90: return BR(!SR.C);         case 0x91: return STA(&CCPU::INDY);
        case 0x92: return STP();                    case 0x93: return SHA(&CCPU::INDY);
        case 0x94: return STY(&CCPU::ZPGX);         case 0x95: return STA(&CCPU::ZPGX);
        case 0x96: return STX(&CCPU::ZPGY);         case 0x97: return SAX(&CCPU::ZPGY);
        case 0x98: return TYA();                    case 0x99: return STA(&CCPU::ABSY);
        case 0x9A: return TXS();                    case 0x9B: return XAS();
        case 0x9C: return SYA();                    case 0x9D: return STA(&CCPU::ABSX);
        case 0x9E: return SXA();                    case 0x9F: return SHA(&CCPU::ABSY);
        case 0xA0: return LDY(&CCPU::IMM);          case 0xA1: return LDA(&CCPU::XIND);
        case 0xA2: return LDX(&CCPU::IMM);          case 0xA3: return LAX(&CCPU::XIND);
        case 0xA4: return LDY(&CCPU::ZPG);          case 0xA5: return LDA(&CCPU::ZPG);
        case 0xA6: return LDX(&CCPU::ZPG);          case 0xA7: return LAX(&CCPU::ZPG);
        case 0xA8: return TAY();                    case 0xA9: return LDA(&CCPU::IMM);
        case 0xAA: return TAX();                    case 0xAB: return LAX(&CCPU::IMM);
        case 0xAC: return LDY(&CCPU::ABS);          case 0xAD: return LDA(&CCPU::ABS);
        case 0xAE: return LDX(&CCPU::ABS);          case 0xAF: return LAX(&CCPU::ABS);
        case 0xB0: return BR(SR.C);          case 0xB1: return LDA(&CCPU::INDY);
        case 0xB2: return STP();                    case 0xB3: return LAX(&CCPU::INDY);
        case 0xB4: return LDY(&CCPU::ZPGX);         case 0xB5: return LDA(&CCPU::ZPGX);
        case 0xB6: return LDX(&CCPU::ZPGY);         case 0xB7: return LAX(&CCPU::ZPGY);
        case 0xB8: return CLV();                    case 0xB9: return LDA(&CCPU::ABSY);
        case 0xBA: return TSX();                    case 0xBB: return LAS();
        case 0xBC: return LDY(&CCPU::ABSX);         case 0xBD: return LDA(&CCPU::ABSX);
        case 0xBE: return LDX(&CCPU::ABSY);         case 0xBF: return LAX(&CCPU::ABSY);
        case 0xC0: return CPY(&CCPU::IMM);          case 0xC1: return CMP(&CCPU::XIND);
        case 0xC2: return NOP(&CCPU::IMM);          case 0xC3: return DCP(&CCPU::XIND);
        case 0xC4: return CPY(&CCPU::ZPG);          case 0xC5: return CMP(&CCPU::ZPG);
        case 0xC6: return DEC(&CCPU::ZPG);          case 0xC7: return DCP(&CCPU::ZPG);
        case 0xC8: return INY();                    case 0xC9: return CMP(&CCPU::IMM);
        case 0xCA: return DEX();                    case 0xCB: return AXS(&CCPU::IMM);
        case 0xCC: return CPY(&CCPU::ABS);          case 0xCD: return CMP(&CCPU::ABS);
        case 0xCE: return DEC(&CCPU::ABS);          case 0xCF: return DCP(&CCPU::ABS);
        case 0xD0: return BR(!SR.Z);         case 0xD1: return CMP(&CCPU::INDY);
        case 0xD2: return STP();                    case 0xD3: return DCP(&CCPU::INDY);
        case 0xD4: return NOP(&CCPU::ZPGX);         case 0xD5: return CMP(&CCPU::ZPGX);
        case 0xD6: return DEC(&CCPU::ZPGX);         case 0xD7: return DCP(&CCPU::ZPGX);
        case 0xD8: return CLD();                    case 0xD9: return CMP(&CCPU::ABSY);
        case 0xDA: return NOP();                   case 0xDB: return DCP(&CCPU::ABSY);
        case 0xDC: return NOP(&CCPU::ABSX);         case 0xDD: return CMP(&CCPU::ABSX);
        case 0xDE: return DEC(&CCPU::ABSX);         case 0xDF: return DCP(&CCPU::ABSX);
        case 0xE0: return CPX(&CCPU::IMM);          case 0xE1: return SBC(&CCPU::XIND);
        case 0xE2: return NOP(&CCPU::IMM);          case 0xE3: return ISC(&CCPU::XIND);
        case 0xE4: return CPX(&CCPU::ZPG);          case 0xE5: return SBC(&CCPU::ZPG);
        case 0xE6: return INC(&CCPU::ZPG);          case 0xE7: return ISC(&CCPU::ZPG);
        case 0xE8: return INX();                    case 0xE9: return SBC(&CCPU::IMM);
        case 0xEA: return NOP();                    case 0xEB: return SBC(&CCPU::IMM);
        case 0xEC: return CPX(&CCPU::ABS);          case 0xED: return SBC(&CCPU::ABS);
        case 0xEE: return INC(&CCPU::ABS);          case 0xEF: return ISC(&CCPU::ABS);
        case 0xF0: return BR(SR.Z);          case 0xF1: return SBC(&CCPU::INDY);
        case 0xF2: return STP();                    case 0xF3: return ISC(&CCPU::INDY);
        case 0xF4: return NOP(&CCPU::ZPGX);         case 0xF5: return SBC(&CCPU::ZPGX);
        case 0xF6: return INC(&CCPU::ZPGX);         case 0xF7: return ISC(&CCPU::ZPGX);
        case 0xF8: return SED();                    case 0xF9: return SBC(&CCPU::ABSY);
        case 0xFA: return NOP();                   case 0xFB: return ISC(&CCPU::ABSY);
        case 0xFC: return NOP(&CCPU::ABSX);         case 0xFD: return SBC(&CCPU::ABSX);
        case 0xFE: return INC(&CCPU::ABSX);         case 0xFF: return ISC(&CCPU::ABSX);
    }
}

#define CLK Read(PC.W)

void CCPU::HandleNMI()
{
    CLK;CLK;
    SR.B = 0u;
    Push(PC.WH);
    Push(PC.WL);
    PushSR();
    SR.I = 1u;
    PC.WL = Read(NMI_INT_HANDLER);
    PC.WH = Read(NMI_INT_HANDLER + 1u);
    m_bNMI = false;
}

void CCPU::HandleIRQ()
{
    CLK;CLK;
    SR.B = 0u;
    Push(PC.WH);
    Push(PC.WL);
    PushSR();
    SR.I = 1u;
    PC.WL = Read(IRQ_INT_HANDLER);
    PC.WH = Read(IRQ_INT_HANDLER + 1u);
}

void CCPU::ExecuteDMA(const uint8_t& data)
{
    // 和PPU的OAM建立连接
    uint8_t *pOAM;
    uint8_t uOAMAddr;
    m_pRefMainBoard->CPUDMA(&pOAM, uOAMAddr);

    Address addressStart = data << 8u;
    uint8_t value;

    // 513个周期
    for(Address index = 0u; index < 0x100u; ++index)
    {
        Address addr = addressStart | index;
        value = Read(addr);
        Read(addr);
        pOAM[(uOAMAddr + index) & 0xFFu] = value;
    }
    Read(PC.W);
    if(!m_bCycleOdd) Read(PC.W);
}

uint8_t CCPU::Read(const Address &address)
{
    ++m_nCyclesToWait;

	// 自身2KB内存
	if (address >= 0x0000u && address <= 0x1FFFu)
	{
		return m_InternalRAM[address & 0x7FFu];
	}

	// PPU 端口
	if (address >= 0x2000u && address <= 0x3FFFu)
	{
		return m_pRefMainBoard->PPURead(address & 0x7u);
	}

	// DMA
	if (address == 0x4014u)
	{
		// cpu open bus
		return (address >> 8u) & 0xFFu;
	}

	// Joystick 1
	if (address == 0x4016u)
	{
		return m_pRefMainBoard->JoystickRead(1);
	}

	// Joystick 2
	if (address == 0x4017u)
	{
		return m_pRefMainBoard->JoystickRead(2);
	}

	// APU (不算0x4014)
	if (address >= 0x4000u && address <= 0x4015u)
	{
		return m_pRefMainBoard->APURead(address);
	}
	
	// Mapper (0x00004018 ~ ........)
	return m_pRefMainBoard->CartridgeReadPRG(address);
}

bool CCPU::Write(const Address &address, const uint8_t &data)
{
    ++m_nCyclesToWait;
    ++m_uCountOfWrite;

	// 自身2KB内存
	if (address >= 0x0000u && address <= 0x1FFFu)
	{
		m_InternalRAM[address & 0x7FFu] = data;
		return true;
	}

	// PPU 端口
	if (address >= 0x2000u && address <= 0x3FFFu)
	{
		return m_pRefMainBoard->PPUWrite(address & 0x7u, data);
	}

	// DMA
	if (address == 0x4014u)
	{
		ExecuteDMA(data);
		return true;
	}

	// Joystick strobe
	if (address == 0x4016u)
	{
		m_pRefMainBoard->JoystickStrobe(data);
		return true;
	}
	
	// APU (已不会匹配 0x4014u和0x4016u)
	if (address >= 0x4000u && address <= 0x4017u)
	{
		return m_pRefMainBoard->APUWrite(address, data);
	}

	// Mapper (4018~FFFF)
	return m_pRefMainBoard->CartridgeWritePRG(address, data);
}

void CCPU::Push(uint8_t value)
{
    Write(0x100u + SP, value);
    --SP;
}

void CCPU::PushSR()
{
    Flags flags = SR;
    flags.X = 1u;
    flags.B = 1u;
    Push(flags.Value);
}

uint8_t CCPU::Pop()
{
    ++SP;
    return Read(0x100u + SP);
}


void CCPU::UpdateC(uint16_t r)
{
    SR.C = r > 0xFFu;
}

void CCPU::UpdateV(uint8_t x, uint8_t y, uint16_t r)
{
    // 多种实现方式
    // (r ^ x) & (r ^ y) & 0x80
    // ~(x ^ y) & (x ^ r) & 0x80
    uint32_t v = (uint32_t)(r ^ x) & (uint32_t)(r ^ y);
    v &= 0x80u;
    SR.V = (v != 0u);
}

void CCPU::UpdateN(uint8_t x)
{
    SR.N = x >> 7u;
}

void CCPU::UpdateZ(uint8_t x)
{
    SR.Z = x == 0u;
}

uint16_t CCPU::IMM()
{
    return PC.W++;
}

uint16_t CCPU::ZPG()
{
    return Read(IMM());
}

uint16_t CCPU::ZPGX()
{
    uint16_t addr = Read(PC.W++);
    CLK;
    return (uint8_t)(addr + X);
}

uint16_t CCPU::ZPGY()
{
    uint16_t addr = Read(PC.W++);
    CLK;
    return (uint8_t)(addr + Y);
}

uint16_t CCPU::ABS()
{
    s_TempReg.WL = Read(PC.W++);
    s_TempReg.WH = Read(PC.W++);
    return s_TempReg.W;
}

#define CHECK_CROSS_PAGE(a, w) (m_bCross = ((a & 0x100u) ^ (w & 0x100u)))

uint16_t CCPU::ABSX()
{
    s_TempReg.WL = Read(PC.W++);
    s_TempReg.WH = Read(PC.W++);
    uint16_t addr = s_TempReg.W + X;
    if (CHECK_CROSS_PAGE(addr, s_TempReg.W))
        CLK;
    return addr;
}

uint16_t CCPU::ABSY()
{
    s_TempReg.WL = Read(PC.W++);
    s_TempReg.WH = Read(PC.W++);
    uint16_t addr = s_TempReg.W + Y;
    if (CHECK_CROSS_PAGE(addr, s_TempReg.W))
        CLK;
    return addr;
}

uint16_t CCPU::IND()
{
    s_TempReg.WL = Read(PC.W++);
    s_TempReg.WH = Read(PC.W++);
    uint16_t addrLow = s_TempReg.W;
    // 仅能在页内移动(FC中的特殊情况)
    s_TempReg.WL++;
    uint16_t addrHigh = s_TempReg.W;
    s_TempReg.WL = Read(addrLow);
    s_TempReg.WH = Read(addrHigh);
    return s_TempReg.W;
}

uint16_t CCPU::XIND()
{
    Address addr = Read(PC.W++);
    CLK;
    s_TempReg.WL = Read((uint8_t)(addr + X));
    s_TempReg.WH = Read((uint8_t)(addr + X + 1u));
    return s_TempReg.W;
}

uint16_t CCPU::INDY()
{
    Address addr = Read(PC.W++);
    s_TempReg.WL = Read(addr);
    s_TempReg.WH = Read((uint8_t)(addr + 1u));
    addr = s_TempReg.W + Y;
    if (CHECK_CROSS_PAGE(addr, s_TempReg.W))
        CLK;
    return addr;
}

#define GA assert(pFunc); uint16_t addr = (this->*pFunc)()
#define GM GA; uint8_t m = Read(addr)

void CCPU::BRK()
{
    Read(PC.W++);
    Push(PC.WH);
    Push(PC.WL);

    SR.B = 1u;
    PushSR();
    SR.I = 1u;

    // 这里是 nes_emu.txt 中的实现
    if (m_bNMI)
    {
        m_bNMI = false;
        PC.WL = Read(NMI_INT_HANDLER);
        PC.WH = Read(NMI_INT_HANDLER + 1u);
    }
    else
    {
        PC.WL = Read(BRK_INT_HANDLER);
        PC.WH = Read(BRK_INT_HANDLER + 1u);
    }
}

void CCPU::ADC(AddressModeFunc pFunc)
{
    // ADC (N Z C V)
    // A + M + C -> A, C
    GM;
    DO_ADC(addr, m);
}

void CCPU::AND(AddressModeFunc pFunc)
{
    // A AND M -> A  (N Z)
    GM;
    DO_AND(addr, m);
}

void CCPU::ASLA()
{
    // C <- [76543210] <- 0   (N Z C)
    SR.C = (A & 0x80u) != 0;
    A <<= 1u;
    UpdateN(A);
    UpdateZ(A);
    CLK;
}

void CCPU::ASL(AddressModeFunc pFunc)
{
    // C <- [76543210] <- 0    (N Z C)
    GM;
    DO_ASL(addr, m, pFunc);
}

void CCPU::BR(bool bCond)
{
    int8_t m = Read(IMM());
    if (bCond)
    {
        uint8_t old_bank = PC.WH;
        PC.W = PC.W + m;
        CLK;
        if ((m_bCross = (PC.WH != old_bank))) CLK;
    }
}

void CCPU::BIT(AddressModeFunc pFunc)
{
    // A AND M, M7 -> N, M6 -> V (N Z V)
    GM;
	Flags flags;
	flags.Value = m;
    SR.V = flags.V;
    SR.N = flags.N;
    UpdateZ(m & A);
}

void CCPU::CLC()
{
    // 0 -> C
    SR.C = 0u;
    CLK;
}

void CCPU::CLD()
{
    // 0 -> D
    SR.D = 0u;
    CLK;
}

void CCPU::CLI()
{
    // 0 -> I
    SR.I = 0u;
    CLK;
}

void CCPU::CLV()
{
    // 0 -> V
    SR.V = 0u;
    CLK;
}

// 根据 http://www.obelisk.me.uk/6502/reference.html#CPY 文档说明
// 这里的C的规则不一样!
#define CPXX(v) GM; DO_CPXX(v, m)

void CCPU::CMP(AddressModeFunc pFunc)
{
    // A - M  (N Z C)
    CPXX(A);
}

void CCPU::CPX(AddressModeFunc pFunc)
{
    // X - M (N Z C)
    CPXX(X);
}

void CCPU::CPY(AddressModeFunc pFunc)
{
    // Y - M (N Z C)
    CPXX(Y);
}

#undef CPXX

void CCPU::DEC(AddressModeFunc pFunc)
{
    // M - 1 -> M  (N Z)
    GM;
    DO_DEC(addr, m, pFunc);
}

void CCPU::DEX()
{
    // X - 1 -> X (N Z)
    --X;
    CLK;
    UpdateN(X); UpdateZ(X);
}

void CCPU::DEY()
{
    // Y - 1 -> Y (N Z)
    --Y;
    CLK;
    UpdateN(Y); UpdateZ(Y);
}

void CCPU::EOR(CCPU::AddressModeFunc pFunc)
{
    // A EOR M -> A (N Z)
    GM;
    DO_EOR(addr, m);
}

void CCPU::INC(CCPU::AddressModeFunc pFunc)
{
    // M + 1 -> M (N Z)
    GM;
    DO_INC(addr, m, pFunc);
}

void CCPU::INX()
{
    // X + 1 -> X (N Z)
    ++X;
    CLK;
    UpdateN(X); UpdateZ(X);
}

void CCPU::INY()
{
    // Y + 1 -> Y (N Z)
    ++Y;
    CLK;
    UpdateN(Y); UpdateZ(Y);
}

void CCPU::JMP(AddressModeFunc pFunc)
{
    GA;
    PC.W = addr;
}

void CCPU::JSR()
{
    CLK;
	DataReg r;
	r.D = 0u;
    // 将JSR第二个操作数所在的地址 Push到栈中(先push高字节)
    r.W = PC.W + 1u;
    Push(r.WH);
    Push(r.WL);
    PC.W = ABS();
}

void CCPU::LDA(AddressModeFunc pFunc)
{
    // M -> A (N Z)
    GM;
    DO_LDA(addr, m);
}

void CCPU::LDX(AddressModeFunc pFunc)
{
    // M -> X (N Z)
    GM;
    DO_LDX(addr, m);
}

void CCPU::LDY(AddressModeFunc pFunc)
{
    // M -> Y (N Z)
    GM;
    Y = m;
    UpdateN(Y); UpdateZ(Y);
}

void CCPU::LSRA()
{
    // 0 -> [76543210] -> C  (N Z C)
    SR.C = A & 0x1u;
    A >>= 1u;
    UpdateN(A);
    UpdateZ(A);
    CLK;
}

void CCPU::LSR(AddressModeFunc pFunc)
{
    // 0 -> [76543210] -> C  (Z C)
    GM;
    DO_LSR(addr, m, pFunc);
}

void CCPU::NOP()
{
    CLK;
}

void CCPU::ORA(AddressModeFunc pFunc)
{
    // A OR M -> A (N Z)
    GM;
    DO_ORA(addr, m);
}

void CCPU::PHA()
{
    Push(A);
    CLK;
}

void CCPU::PHP()
{
    // from nintendulator
    PushSR();
    CLK;
}

void CCPU::PLA()
{
    // N Z
    A = Pop();
    CLK;
    UpdateN(A); UpdateZ(A);
    CLK;
}

void CCPU::PLP()
{
    SR.Value = Pop();
    CLK;
    CLK;
}

void CCPU::ROLA()
{
    // C <- [76543210] <- C (N Z C)
    uint8_t fc = A >> 7u;
    A <<= 1u;
    A |= SR.C;
    SR.C = fc;
    UpdateN(A); UpdateZ(A);
    CLK;
}

void CCPU::ROL(AddressModeFunc pFunc)
{
    // C <- [76543210] <- C (N Z C)
    GM;
    DO_ROL(addr, m, pFunc);
}

void CCPU::RORA()
{
    // C -> [76543210] -> C (N Z C)
    uint8_t fc = A & 0x1u;
    A >>= 1u;
    A |= (uint8_t)(SR.C << 0x7u);
    SR.C = fc;
    UpdateN(A); UpdateZ(A);
    CLK;
}

void CCPU::ROR(CCPU::AddressModeFunc pFunc)
{
    // C -> [76543210] -> C (N Z C)
    GM;
    DO_ROR(addr, m, pFunc);
}

void CCPU::RTI()
{
    PLP();
    PC.WL = Pop();
    PC.WH = Pop();
}

void CCPU::RTS()
{
    CLK; CLK;
    PC.WL = Pop();
    PC.WH = Pop();
    ++PC.W;
    CLK;
}

void CCPU::SBC(AddressModeFunc pFunc)
{
    // G ^ 0xFF; s16 r = A + p + P[C]; upd_cv(A, p, r); upd_nz(A = r);

    // A - M - !C -> A (N Z C V)
    GM;
    DO_SBC(addr, m);
}

void CCPU::SEC()
{
    // 1 -> C
    SR.C = 1u;
    CLK;
}

void CCPU::SED()
{
    // 1 -> D
    SR.D = 1u;
    CLK;
}

void CCPU::SEI()
{
    // 1 -> I
    SR.I = 1u;
    CLK;
}

void CCPU::STA(AddressModeFunc pFunc)
{
    // A -> M
    GA;
    Write(addr, A);
    if (!m_bCross)
    {
        if (pFunc == &CCPU::INDY || pFunc == &CCPU::ABSX || pFunc == &CCPU::ABSY)
        {
            CLK;
        }
    }
}

void CCPU::STX(AddressModeFunc pFunc)
{
    // X -> M
    GA;
    Write(addr, X);
}

void CCPU::STY(AddressModeFunc pFunc)
{
    // Y -> M
    GA;
    Write(addr, Y);
}

void CCPU::TAX()
{
    // A -> X (N Z)
    X = A;
    UpdateN(X); UpdateZ(X);
    CLK;
}

void CCPU::TAY()
{
    // A -> Y (N Z)
    Y = A;
    UpdateN(Y); UpdateZ(Y);
    CLK;
}

void CCPU::TSX()
{
    // SP -> X (N Z)
    X = SP;
    UpdateN(X); UpdateZ(X);
    CLK;
}

void CCPU::TXA()
{
    // X -> A (N Z)
    A = X;
    UpdateN(X); UpdateZ(X);
    CLK;
}

void CCPU::TXS()
{
    // X -> SP
    SP = X;
    CLK;
}

void CCPU::TYA()
{
    // Y -> A  (N Z)
    A = Y;
    UpdateN(A); UpdateZ(A);
    CLK;
}

void CCPU::STP()
{
    // 停机指令
    Read(PC.W);
    assert(false);
}

void CCPU::SLO(CCPU::AddressModeFunc pFunc)
{
    // ASL value then ORA value
    GM;
    DO_ORA(addr, DO_ASL(addr, m, pFunc));
}

void CCPU::NOP(CCPU::AddressModeFunc pFunc)
{
    GA;
    Read(addr);
}

void CCPU::ANC(CCPU::AddressModeFunc pFunc)
{
    GM;
    //A &= value;
    //c = n = A & highbit;
    DO_AND(addr, m);
    SR.C = (A & 0x80u) != 0;
}

void CCPU::RLA(CCPU::AddressModeFunc pFunc)
{
    // ROL value then AND value
    GM;
    DO_AND(addr, DO_ROL(addr, m, pFunc));
}

void CCPU::SRE(CCPU::AddressModeFunc pFunc)
{
    // LSR value then EOR value
    GM;
    DO_EOR(addr, DO_LSR(addr, m, pFunc));
}

// ASR = ALR
void CCPU::ASR(CCPU::AddressModeFunc pFunc)
{
    // AND #{imm} + LSR
    // A:=(A&#{imm})/2
    GM;
    DO_AND(addr, m);
    LSRA();
}

void CCPU::RRA(CCPU::AddressModeFunc pFunc)
{
    // ROR value then ADC value
    GM;
    DO_ADC(addr, DO_ROR(addr, m, pFunc));
}

void CCPU::ARR(CCPU::AddressModeFunc pFunc)
{
    // AND value then ROR value
    GM;
    DO_AND(addr, m);

    uint32_t v = ((uint32_t)A ^ ((uint32_t)A >> 1u));
    SR.V = (v & 0x40u) != 0;
    uint8_t fc = A >> 7u;
    A >>= 1u;
    A |= (uint8_t)(SR.C << 7u);
    SR.C = fc;
    UpdateN(A);
    UpdateZ(A);
    CLK;
}

void CCPU::SAX(CCPU::AddressModeFunc pFunc)
{
    GA;
    Write(addr, A & X);
}

void CCPU::XAA()
{
    CCPU::AddressModeFunc pFunc = &CCPU::IMM;
    GM;
    uint8_t result = (uint8_t)(A & X) & m;
    UpdateN(result);
    UpdateZ(result);
    A &= X & (m | 0xEFu);
}

void CCPU::SHA(CCPU::AddressModeFunc pFunc)
{
    GA;
    // A AND X AND ([high byte of address] + 1)
    uint32_t result = (uint32_t)A & X & ((addr >> 8u) + 1u);
    Write(addr, result);
}

// XAS = TAS
void CCPU::XAS()
{
    // 	S:=A&X {adr}:=S&H
    CCPU::AddressModeFunc pFunc = &CCPU::ABSY;
    GA;
    SP = A & X;
    uint32_t result = SP & ((addr >> 8u) + 1u);
    Write(addr, result);
}

// SYA = SHY
void CCPU::SYA()
{
    CCPU::AddressModeFunc pFunc = &CCPU::ABSX;
    GA;
    // Y & a <= min(Y, a)
    // 让Y控制页数(高位), 但必须限制在 [0, addrH]
    // 所以这里取得absx的地址的高位 是为了限制Y的范围的?
    // 也可能是为了让Y充当掩码的功能, 对地址做处理(比如高位的第1位永远是0)?

    //From here: http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
    //Unsure if this is accurate or not
    //"the target address for e.g. SYA becomes ((y & (addr_high + 1)) << 8) | addr_low instead of the normal ((addr_high + 1) << 8) | addr_low"
    uint8_t result = Y & ((addr >> 8u) + 1u);
    addr = (uint16_t)(result << 8u) | (addr & 0xFFu);
    Write(addr, result);
}

// SXA = SHX
void CCPU::SXA()
{
    CCPU::AddressModeFunc pFunc = &CCPU::ABSY;
    GA;
    uint8_t result = X & ((addr >> 8u) + 1u);
    addr = (uint16_t)(result << 8u) | (addr & 0xFFu);
    Write(addr, result);
}

void CCPU::LAX(CCPU::AddressModeFunc pFunc)
{
    // LDA {adr} + LDX {adr}
    GM;
    DO_LDA(addr, m);
    DO_LDX(addr, m);
}

void CCPU::LAS()
{
    CCPU::AddressModeFunc pFunc = &CCPU::ABSY;
    GM;
    // A = X = S &= value;
    //    n = A & highbit;
    //    z = !A;
    SP &= m;
    A = X = SP;
    UpdateN(A);
    UpdateZ(A);
}

void CCPU::DCP(CCPU::AddressModeFunc pFunc)
{
    // DEC value then CMP value
    GM;
    DO_CPXX(A, DO_DEC(addr, m, pFunc));
}

void CCPU::AXS(CCPU::AddressModeFunc pFunc)
{
    GM;
    uint32_t t = (A & X) - m;
    UpdateN(t);
    UpdateZ(t);
    UpdateC(t);
    SR.C ^= 1u;
    X = t;
}

void CCPU::ISC(CCPU::AddressModeFunc pFunc)
{
    // ISC {adr} = INC {adr} + SBC {adr} (ISC就是ISB指令)
    GM;
    DO_SBC(addr, DO_INC(addr, m, pFunc));
}

uint8_t CCPU::DO_ASL(const Address& addr, uint8_t m, AddressModeFunc pFunc)
{
    SR.C = (m & 0x80u) != 0;
    Write(addr, m);
    m <<= 1u;
    UpdateN(m); UpdateZ(m);
    Write(addr, m);
    if (!m_bCross && pFunc == &CCPU::ABSX) CLK;
    return m;
}

void CCPU::DO_ORA(const Address& addr, const uint8_t& m)
{
    A |= m;
    UpdateN(A); UpdateZ(A);
}

uint8_t CCPU::DO_ROL(const Address& addr, uint8_t m, AddressModeFunc pFunc)
{
    Write(addr, m);
    uint8_t fc = m >> 0x7u;
    m <<= 1u;
    m |= SR.C;
    SR.C = fc;
    UpdateN(m); UpdateZ(m);
    Write(addr, m);
    if (!m_bCross && pFunc == &CCPU::ABSX) CLK;
    return m;
}

void CCPU::DO_AND(const Address& addr, const uint8_t& m)
{
    A &= m;
    UpdateN(A); UpdateZ(A);
}

uint8_t CCPU::DO_LSR(const Address& addr, uint8_t m, AddressModeFunc pFunc)
{
    Write(addr, m);
    SR.C = m & 0x1u;
    m >>= 1u;
    UpdateN(m);
    UpdateZ(m);
    Write(addr, m);
    if (!m_bCross && pFunc == &CCPU::ABSX) CLK;
    return m;
}

void CCPU::DO_EOR(const Address& addr, const uint8_t& m)
{
    A ^= m;
    UpdateN(A); UpdateZ(A);
}

uint8_t CCPU::DO_ROR(const Address &addr, uint8_t m, CCPU::AddressModeFunc pFunc)
{
    Write(addr, m);
    uint8_t fc = m & 0x1u;
    m >>= 1u;
    m |= (uint8_t)(SR.C << 0x7u);
    SR.C = fc;
    UpdateN(m); UpdateZ(m);
    Write(addr, m);
    if (!m_bCross && pFunc == &CCPU::ABSX) CLK;
    return m;
}

void CCPU::DO_ADC(const Address &addr, const uint8_t &m)
{
    uint16_t r = A + m + SR.C;
    UpdateN(r); UpdateZ(r); UpdateC(r); UpdateV(A, m, r);
    A = r;
}

void CCPU::DO_LDA(const Address& addr, const uint8_t& m)
{
    A = m;
    UpdateN(A); UpdateZ(A);
}

void CCPU::DO_LDX(const Address& addr, const uint8_t& m)
{
    X = m;
    UpdateN(X); UpdateZ(X);
}

uint8_t CCPU::DO_DEC(const Address& addr, uint8_t m, AddressModeFunc pFunc)
{
    Write(addr, m);
    --m;
    UpdateN(m); UpdateZ(m);
    Write(addr, m); // 写回新值
    if (!m_bCross && pFunc == &CCPU::ABSX) CLK;
    return m;
}

void CCPU::DO_CPXX(const uint8_t& reg, const uint8_t& m)
{
    uint16_t r = reg - m;
    UpdateN(r);
    UpdateZ(r);
    SR.C = (reg >= m);
}

uint8_t CCPU::DO_INC(const Address& addr, uint8_t m, AddressModeFunc pFunc)
{
    Write(addr, m);
    ++m;
    UpdateN(m); UpdateZ(m);
    Write(addr, m);
    if (!m_bCross && pFunc == &CCPU::ABSX) CLK;
    return m;
}

void CCPU::DO_SBC(const Address& addr, uint8_t m)
{
    m ^= 0xFFu;
    // * r = A - M - !C  (from http://nesdev.com/6502.txt)
    // * uint32 l=_A-x-((_P&1)^1);   (from fceux)
    // * 一个化简后的方法: r = A + (x & 0xFF) + C      (from LaiNES)
    uint16_t r = A + m + SR.C;
    UpdateC(r);
    UpdateV(A, m, r);
    UpdateN(r);
    UpdateZ(r);
    A = r;
}

#undef GM
#undef GA
#undef CLK
