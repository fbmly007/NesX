
#include "Units/Cartridge.h"
#include "Core/NesHeader.h"
#include "Core/BaseMapper.h"
#include "Core/MapperFactory.h"
#include <cassert>

CCartridge::CCartridge()
  : m_pHeader(nullptr),
    m_pMapper(nullptr),
    m_bLoaded(false)
{
}

CCartridge::~CCartridge()
{
  Quit();
}

void CCartridge::SaveSnapshot(CartridgeSnapshot& state)
{
    state.prgHash = 0;
    state.chrHash = 0;
    this->m_pMapper->SaveSnapshot(state.mapper);
}

bool CCartridge::LoadSnapshot(const CartridgeSnapshot& state)
{
    // TODO 需要检查hash值是否符合!
    this->m_pMapper->LoadSnapshot(state.mapper);
    return true;
}

bool CCartridge::Init(CMainBoard *pRefMainBoard)
{
  if(!CUnit::Init(pRefMainBoard)) return false;
  m_pHeader = new CNesHeader;
  return true;
}

void CCartridge::Quit()
{
  Unload();
  SAFE_RELEASE(m_pHeader)
  CUnit::Quit();
}

void CCartridge::PowerUp()
{
    if(m_pMapper)
        m_pMapper->PowerUp();
}

void CCartridge::Reset()
{
    if(m_pMapper)
        m_pMapper->Reset();
}

void CCartridge::PowerDown()
{
    if(m_pMapper)
        m_pMapper->PowerDown();
}

bool CCartridge::Load(const char *szFileName)
{
  assert(m_pHeader);

  // 卸载游戏数据
  Unload();

  // 加载新数据
  ifstream ifs(szFileName, std::ios::binary);
  if (!ifs) return false;

  // 加载头部信息
  if(!m_pHeader->Load(ifs))
    return false;

  // 根据Nes头创建Mapper
  m_pMapper = CreateMapper(m_pHeader, ifs);
  if(!m_pMapper)
    return false;

  // 加载完毕
  m_bLoaded = true;
  return true;
}

void CCartridge::Unload()
{
  SAFE_RELEASE(m_pMapper)
  if(m_pHeader)
    m_pHeader->Unload();
  m_bLoaded = false;
}

bool CCartridge::IsLoaded() const
{
  return m_bLoaded;
}

CNesHeader *CCartridge::GetHeader() const
{
  return m_pHeader;
}

Address CCartridge::NameTableMirroring(const Address &address)
{
  return m_pMapper->NameTableMirroring(address);
}

uint8_t CCartridge::ReadPRG(const Address &address)
{
  return m_pMapper->ReadPRG(address);
}

bool CCartridge::WritePRG(const Address &address, const uint8_t &data)
{
  return m_pMapper->WritePRG(address, data);
}

uint8_t CCartridge::ReadCHR(const Address &address)
{
  return m_pMapper->ReadCHR(address);
}

bool CCartridge::WriteCHR(const Address &address, const uint8_t &data)
{
  return m_pMapper->WriteCHR(address, data);
}


void CCartridge::DataSignal(const uint8_t &signal, const Address &data)
{
    m_pMapper->DataSignal(signal, data);
}

CBaseMapper *CCartridge::CreateMapper(CNesHeader *pHeader, ifstream& ifs)
{
  CBaseMapper *pMapper = CMapperFactory::Get(pHeader->Mapper());
  if(pMapper)
  {
    if(!pMapper->Init(m_pRefMainBoard, pHeader, ifs))
    {
      delete pMapper;
      pMapper = nullptr;
    }
  }
  return pMapper;
}

