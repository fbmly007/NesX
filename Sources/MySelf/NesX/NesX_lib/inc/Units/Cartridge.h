
#ifndef NESX_NESX_LIB_INC_UNITS_CARTRIDGE_H_
#define NESX_NESX_LIB_INC_UNITS_CARTRIDGE_H_

#include "Units/Unit.h"
#include "Snapshots/CartridgeSnapshot.h"
#include <fstream>

using std::ifstream;

class CNesHeader;

class CBaseMapper;

class CCartridge : public CUnit
{
public:
    CCartridge();
    ~CCartridge();

    void SaveSnapshot(CartridgeSnapshot& state);
    bool LoadSnapshot(const CartridgeSnapshot& state);

    bool Init(CMainBoard *pRefMainBoard) override;
    void Quit() override;

    void PowerUp() override;
    void Reset() override;
    void PowerDown() override;

    void Tick() override
    {};
    int Divider() const override
    { return 0; };

public:
    bool Load(const char *szFileName);

    void Unload();

    bool IsLoaded() const;

    CNesHeader *GetHeader() const;

    Address NameTableMirroring(const Address &address);

    uint8_t ReadPRG(const Address &address);

    bool WritePRG(const Address &address, const uint8_t &data);

    uint8_t ReadCHR(const Address &address);

    bool WriteCHR(const Address &address, const uint8_t &data);

    void DataSignal(const uint8_t& signal, const Address& data);

private:
    CBaseMapper *CreateMapper(CNesHeader *pHeader, ifstream &ifs);

private:
    CNesHeader *m_pHeader;
    CBaseMapper *m_pMapper;
    bool m_bLoaded;
};

#endif //NESX_NESX_LIB_INC_UNITS_CARTRIDGE_H_
