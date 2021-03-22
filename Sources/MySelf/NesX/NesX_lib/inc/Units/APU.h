
#ifndef NESX_NESX_LIB_INC_UNITS_APU_H_
#define NESX_NESX_LIB_INC_UNITS_APU_H_

#include "Units/Unit.h"
#include "Nes_Apu.h"
#include "Sound_Queue.h"

class CAPU : public CUnit
{
public:
    CAPU();
    ~CAPU();

    void SaveSnapshot(apu_snapshot_t& state);
    void LoadSnapshot(const apu_snapshot_t& state);

    bool Init(CMainBoard *pRefMainBoard) override;
    void Quit() override;
    void PowerUp() override;
    void Reset() override;
    void PowerDown() override;

    uint8_t ReadPort(const Address &address);
    bool WritePort(const Address &address, const uint8_t &data);

    // Virtual
protected:
    void Tick() override;
    int Divider() const override;


private:
    static int STATIC_dmc_read( void*, cpu_addr_t addr );


private:
    Blip_Buffer buf;
    Nes_Apu apu;
    Sound_Queue * soundQueue;

    static CAPU *s_Instance;
};

#endif //NESX_NESX_LIB_INC_UNITS_APU_H_
