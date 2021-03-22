
#include "Units/MainBoard.h"
#include "Units/Cartridge.h"
#include "Units/Joystick.h"
#include "Units/CPU.h"
#include "Units/PPU.h"
#include "Units/APU.h"

constexpr Uint64 kClockTable[] = {89488, 89489, 89488, 89489, 89489};

CMainBoard::CMainBoard()
        : m_pRefCartridge(nullptr),
          m_pRefJoystick(nullptr),
          m_pCPU(nullptr),
          m_pPPU(nullptr),
          m_uCurrentClockIndex(0u),
          m_uCurrentClockValue(0u),
          m_kCountersPerFrame(SDL_GetPerformanceFrequency() / 60)
{
    SetClockIndex(m_uCurrentClockIndex);
}

CMainBoard::~CMainBoard()
{
    Quit();
}

void CMainBoard::SaveSnapshot(MainBoardSnapshot& state)
{
    state.clockIndex = m_uCurrentClockIndex;
    state.clockValue = m_uCurrentClockValue;
    this->m_pCPU->SaveSnapshot(state.cpu);
    this->m_pPPU->SaveSnapshot(state.ppu);
    this->m_pAPU->SaveSnapshot(state.apu);
}

void CMainBoard::LoadSnapshot(const MainBoardSnapshot& state)
{
    m_uCurrentClockIndex = state.clockIndex;
    m_uCurrentClockValue = state.clockValue;
    this->m_pCPU->LoadSnapshot(state.cpu);
    this->m_pPPU->LoadSnapshot(state.ppu);
    this->m_pAPU->LoadSnapshot(state.apu);
}

bool CMainBoard::Init(CMonitor *pRefMonitor)
{
    // CPU
    m_pCPU = new CCPU;
    if (!m_pCPU->Init(this))
        return false;

    // PPU
    m_pPPU = new CPPU;
    if(!m_pPPU->Init(this))
        return false;
    m_pPPU->ConnectMonitor(pRefMonitor);

    // APU
    m_pAPU = new CAPU;
    if(!m_pAPU->Init(this))
        return false;

    // 初始化CPU
    return true;
}

void CMainBoard::Quit()
{
    m_pRefCartridge = nullptr;
    m_pRefJoystick = nullptr;
    SAFE_RELEASE(m_pCPU)
    SAFE_RELEASE(m_pPPU)
}



void CMainBoard::Run()
{
    Uint64 start = SDL_GetPerformanceCounter();

    for (Uint64 index = 0; index < m_uCurrentClockValue; ++index)
    {
        // ppu
        m_pPPU->Clock();

        // apu
        m_pAPU->Clock();

        // cpu
        m_pCPU->Clock();

    }

    while (SDL_GetPerformanceCounter() - start < m_kCountersPerFrame);
    SetClockIndex(m_uCurrentClockIndex + 1);
}

//void CMainBoard::Run()
//{
//    static Uint64 globalstart = SDL_GetPerformanceCounter();
//    static Uint64 totalClocks = 0;
//
//    Uint64 start = SDL_GetPerformanceCounter();
//
//    for(Uint64 index = 0; index < m_uCurrentClockValue; ++index)
//    {
//        // m_pCPU->Clock();
//    }
//
//    totalClocks += m_uCurrentClockValue;
//
//    while(SDL_GetPerformanceCounter()-start < m_kCountersPerFrame);
//    m_uCurrentClockIndex = (m_uCurrentClockIndex + 1) % 5;
//    m_uCurrentClockValue = kClockTable[m_uCurrentClockIndex];
//
//    if(SDL_GetPerformanceCounter()-globalstart >= SDL_GetPerformanceFrequency())
//    {
//        printf("clocks = %llu\n", totalClocks);
//        globalstart = SDL_GetPerformanceCounter();
//        totalClocks = 0;
//    }
//}

void CMainBoard::PowerUp()
{
    SetClockIndex(0);
    if (m_pRefCartridge->IsLoaded())
        m_pRefCartridge->PowerUp();
    m_pCPU->PowerUp();
    m_pPPU->PowerUp();
}

void CMainBoard::Reset()
{
    if (m_pRefCartridge->IsLoaded())
        m_pRefCartridge->Reset();
    m_pCPU->Reset();
    m_pPPU->Reset();
}

void CMainBoard::PowerDown()
{
    if (m_pRefCartridge->IsLoaded())
        m_pRefCartridge->PowerDown();
    m_pCPU->PowerDown();
    m_pPPU->PowerDown();
}

void CMainBoard::InsertCartridge(CCartridge *pRefCartridge)
{
    m_pRefCartridge = pRefCartridge;
}

void CMainBoard::ConnectJoystick(CJoystick *pRefJoystick)
{
    m_pRefJoystick = pRefJoystick;
}

uint8_t CMainBoard::PPURead(const Address &address)
{
    return m_pPPU->ReadPort(address);
}

bool CMainBoard::PPUWrite(const Address &address, const uint8_t &data)
{
    return m_pPPU->WritePort(address, data);
}

uint8_t CMainBoard::APURead(const Address &address)
{
    return m_pAPU->ReadPort(address);
}

bool CMainBoard::APUWrite(const Address &address, const uint8_t &data)
{
    return m_pAPU->WritePort(address, data);
}

Address CMainBoard::CartridgeNameTableMirroring(const Address &address)
{
    return m_pRefCartridge->NameTableMirroring(address);
}

uint8_t CMainBoard::CartridgeReadPRG(const Address &address)
{
    return m_pRefCartridge->ReadPRG(address);
}

bool CMainBoard::CartridgeWritePRG(const Address &address, const uint8_t &data)
{
    return m_pRefCartridge->WritePRG(address, data);
}

uint8_t CMainBoard::CartridgeReadCHR(const Address &address)
{
    return m_pRefCartridge->ReadCHR(address);
}

bool CMainBoard::CartridgeWriteCHR(const Address &address, const uint8_t &data)
{
    return m_pRefCartridge->WriteCHR(address, data);
}

void CMainBoard::CartridgeDataSignal(const uint8_t& signal, const Address& data)
{
    m_pRefCartridge->DataSignal(signal, data);
}

void CMainBoard::CPUSignal(const uint8_t &signal)
{
    m_pCPU->Signal(signal);
}

uint8_t CMainBoard::CPUCountOfWrite() const
{
    return m_pCPU->CountOfWrite();
}

void CMainBoard::CPUDMA(uint8_t **pOAM, uint8_t& uOAMAddr)
{
    m_pPPU->DMA(pOAM, uOAMAddr);
}

uint8_t CMainBoard::CPURead(const Address &address)
{
    return m_pCPU->DMC_Read(address);
}

void CMainBoard::SetClockIndex(const Uint64 &index)
{
    m_uCurrentClockIndex = index % 5;
    m_uCurrentClockValue = kClockTable[m_uCurrentClockIndex];
}

void CMainBoard::JoystickStrobe(const uint8_t &data)
{
    m_pRefJoystick->Write(data & 0x1u);
}

uint8_t CMainBoard::JoystickRead(int n)
{
    return m_pRefJoystick->Read(n);
}



