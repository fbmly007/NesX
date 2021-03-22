#include "iNesHeader.h"

iNesHeader::iNesHeader(istream& stream)
	: m_uNumberOfPRG(0),
	  m_uNumberOfCHR(0),
	  m_uFlags(0),
	  m_uMapper(0),
	  m_uSubMapper(0),
	  m_uSizeOfPRGRAM(0),
	  m_uSizeOfPRGNVRAM(0),
	  m_uSizeOfCHRRAM(0),
	  m_uSizeOfCHRNVRAM(0),
	  m_uPPUType(0),
	  m_uHardwareType(0),
	  m_uExtendedConsoleType(0),
      m_uMiscellaneousROMs(0),
	  m_uDefaultExpansionDevice(0),
	  m_consoleType(ConsoleType::Console_NES),
	  m_nesVariant(NesVariant::Variant_Invalid),
	  m_tvSystem(TVSystem::TV_NTSC)
{
	// 读取0-15
	uint8_t header[16];
	stream.read((char *)header, 16);
		
	// 判断版本(会检查NES头)
	m_nesVariant = CheckVariant(header);

	// 4: 读取PRG的个数(每个16KB)
	m_uNumberOfPRG = header[4];

	// 5: 读取CHR的个数(每个8KB)
	m_uNumberOfCHR = header[5];

	// 6: * 读取属性和Mapper的低半字节
	m_uFlags = header[6] & 0xF;
	m_uMapper = header[6] >> 4;

	// 是否有Trainer  (需要将这个数据映射到内存的$7000-$71FF, 也就是SRAM区域）
	if (HasTrainer())
	{
		m_trainerData.resize(0x200);
		stream.read((char *)m_trainerData.data(), m_trainerData.size());
	}

	// 老版本(兼容)的NES 不使用7-15
	if (m_nesVariant == NesVariant::Variant_ArchaiciNES)
		return;

	// 读取Mapper号的高半字节
	m_uMapper |= header[7] & 0xF0;

	// 解析Console Type
	m_consoleType = (ConsoleType)(header[7] & 0x3);

	// 如果是iNES
	if (m_nesVariant == NesVariant::Variant_iNES)
	{
		// iNES的flag8是PRG内存大小
		m_uSizeOfPRGRAM = (uint32_t)header[8];

		// iNES的flag9的第0位是TV系统类型
		m_tvSystem = (TVSystem)(header[9] & 0x1);
		return;
	}

	// 之后的解析都是2.0的

	// 8 * 解析mapper(2.0变成12位)
	m_uMapper |= ((uint16_t)(header[8] & 0xF)) << 8;

	// 8 * 解析子mapper号
	m_uSubMapper = header[8] >> 4;

	// 9 * 解析PRG-ROM的高字节
	m_uNumberOfPRG |= ((uint16_t)(header[9] & 0xF)) << 8;

	// 9 * 解析CHR-ROM的高字节
	m_uNumberOfCHR |= ((uint16_t)(header[9] >> 4)) << 8;

	// 10 * 解析PRG-RAM
	m_uSizeOfPRGRAM = header[10] & 0xF;
	m_uSizeOfPRGRAM = m_uSizeOfPRGRAM > 0u ? 64u << m_uSizeOfPRGRAM : 0u;

	// 10 * 解析PRG-NVRAM
	m_uSizeOfPRGNVRAM = header[10] >> 4;
	m_uSizeOfPRGNVRAM = m_uSizeOfPRGNVRAM > 0u ? 64u << m_uSizeOfPRGNVRAM : 0u;

	// 11 * 解析CHR-RAM
	m_uSizeOfCHRRAM = header[11] & 0xF;
	m_uSizeOfCHRRAM = m_uSizeOfCHRRAM > 0u ? 64u << m_uSizeOfCHRRAM : 0u;

	// 11 * 解析PRG-NVRAM
	m_uSizeOfCHRNVRAM = header[11] >> 4;
	m_uSizeOfCHRNVRAM = m_uSizeOfCHRNVRAM > 0u ? 64u << m_uSizeOfCHRNVRAM : 0u;

	// 12 * 解析TVSystem(TimingMode)
	m_tvSystem = (TVSystem)(header[12] & 0x3);

	// 13 * 按ConsoleType去解析不同的值
	if (m_consoleType == ConsoleType::Console_VSSystem)
	{
		m_uPPUType = header[13] & 0xF;
		m_uHardwareType = header[13] >> 4;
	}
	else if (m_consoleType == ConsoleType::Console_Extended)
	{
		m_uExtendedConsoleType = header[13] & 0xF;
	}

	// 14 * 解析Miscellaneous ROMs
	m_uMiscellaneousROMs = header[14] & 0x3;

	// 15 * 解析Default Expansion Device
	m_uDefaultExpansionDevice = header[15] & 0x3F;
}

iNesHeader::~iNesHeader()
{
}

bool iNesHeader::HasBettery() const
{
	return (m_uFlags & NesHeaderFlag::Flag_Battery) != 0;
}

bool iNesHeader::HasVerticalMirroring() const
{
	return (m_uFlags & NesHeaderFlag::Flag_VerticalMirroring) != 0;
}

bool iNesHeader::HasTrainer() const
{
	return (m_uFlags & NesHeaderFlag::Flag_Trainer) != 0;
}

bool iNesHeader::HasFourScreenRam() const
{
	return (m_uFlags & NesHeaderFlag::Flag_FourScreenVRAM) != 0;
}

uint16_t iNesHeader::NumberOfPRG() const
{
	return m_uNumberOfPRG;
}

uint16_t iNesHeader::NumberOfCHR() const
{
	return m_uNumberOfCHR;
}

uint16_t iNesHeader::Mapper() const
{
	return m_uMapper;
}

void iNesHeader::GetTrainer(string & data) const
{
	data.append(m_trainerData.data(), m_trainerData.size());
}

/*
 * <TODO> Recommended detection procedure:
 * If byte 7 AND $0C = $08, and the size taking into account byte 9 does not exceed the actual size of the ROM image, then NES 2.0.
 * If byte 7 AND $0C = $00, and bytes 12 - 15 are all 0, then iNES.
 * Otherwise, archaic iNES.
 * 现在先实现 ArchaiciNES , 其他的先解析放在那里, 所以这个类型暂时不是很重要
*/
iNesHeader::NesVariant iNesHeader::CheckVariant(uint8_t header[16])
{
	if (!(header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A))
		return NesVariant::Variant_Invalid;

#define HEADER_CHECK(i, c) (!header[i] || header[i] == c)
	// 全0 或 `DiskDude!`
	if (HEADER_CHECK( 7, 'D') &&
		HEADER_CHECK( 8, 'i') &&
		HEADER_CHECK( 9, 's') &&
		HEADER_CHECK(10, 'k') &&
		HEADER_CHECK(11, 'D') &&
		HEADER_CHECK(12, 'u') &&
		HEADER_CHECK(13, 'd') &&
		HEADER_CHECK(14, 'e') &&
		HEADER_CHECK(15, '!')
	) {
		return NesVariant::Variant_ArchaiciNES;
	}
#undef HEADER_CHECK

	// 判断是不是扩展2.0版本
	if ((header[7] & 0x0C) == 0x08)
	{
		return NesVariant::Variant_iNES_20;
	}

	// 2.0版本以下
	return NesVariant::Variant_iNES;
}

