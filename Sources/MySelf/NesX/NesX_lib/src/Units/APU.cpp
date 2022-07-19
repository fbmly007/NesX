
#include "Units/APU.h"
#include "Units/MainBoard.h"

constexpr int OUT_SIZE = 4096;
blip_sample_t outBuf[OUT_SIZE];

CAPU *CAPU::s_Instance = nullptr;

CAPU::CAPU()
    : soundQueue(nullptr)
{
}

CAPU::~CAPU()
{
}

void CAPU::SaveSnapshot(apu_snapshot_t& state)
{
    apu.save_snapshot(&state);
}

void CAPU::LoadSnapshot(const apu_snapshot_t& state)
{
    apu.load_snapshot(state);
}

bool CAPU::Init(CMainBoard *pRefMainBoard)
{
    if(!CUnit::Init(pRefMainBoard)) return false;

    // 初始化
    soundQueue = new Sound_Queue;
    if ( soundQueue->init( 44100 ) )
        return false;

    blargg_err_t error = buf.sample_rate( 44100 );
    if ( error )
        return false;

    buf.clock_rate( 1789772 );
    apu.output(&buf);

    apu.dmc_reader(&CAPU::STATIC_dmc_read);
    // apu.irq_notifier(irq_changed);

    s_Instance = this;
    return true;
}


void CAPU::Quit()
{
    CUnit::Quit();
    SAFE_RELEASE(soundQueue)
    s_Instance = nullptr;
}

void CAPU::PowerUp()
{

}

void CAPU::Reset()
{
    apu.reset();
    buf.clear();
}

void CAPU::PowerDown()
{

}

uint8_t CAPU::ReadPort(const Address &address)
{
    if(address == apu.start_addr)
        return apu.read_status(1);
    return 0u;
}

bool CAPU::WritePort(const Address &address, const uint8_t &data)
{
    apu.write_register(1, address, data);
    return true;
}

void CAPU::Tick()
{
    apu.end_frame( 1 );
    buf.end_frame( 1 );

    // Read some samples out of Blip_Buffer if there are enough to
    // fill our output buffer
    if ( buf.samples_avail() >= OUT_SIZE )
    {
        size_t count = buf.read_samples( outBuf, OUT_SIZE );
        soundQueue->write(outBuf, count);
    }
}

int CAPU::Divider() const
{
    return 3;
}

int CAPU::STATIC_dmc_read(void *, cpu_addr_t addr)
{
    return s_Instance->m_pRefMainBoard->CPURead((Address)addr);
}
