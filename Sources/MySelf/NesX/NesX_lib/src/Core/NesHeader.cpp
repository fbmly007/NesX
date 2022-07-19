
#include <Core/NesHeader.h>

CNesHeader::CNesHeader()
        : m_uNumberOfPRG(0u),
          m_uNumberOfCHR(0u),
          m_uFlags(0u),
          m_uMapper(0u),
          m_uSubMapper(0u),
          m_uSizeOfPRGRAM(0u),
          m_uSizeOfPRGNVRAM(0u),
          m_uSizeOfCHRRAM(0u),
          m_uSizeOfCHRNVRAM(0u),

          m_uPPUType(0u),
          m_uHardwareType(0u),

          m_uExtendedConsoleType(0u),
          m_uMiscellaneousROMs(0u),

          m_uDefaultExpansionDevice(0u),

          m_ConsoleType(ConsoleType::Console_NES),
          m_Variant(NesVariant::Variant_Invalid),
          m_TVSystem(TVSystem::TV_NTSC)
{
}

CNesHeader::~CNesHeader()
= default;

bool CNesHeader::Load(istream &stream)
{
    Unload();

    // 读取0-15
    uint8_t header[16];
    stream.read((char *) header, 16);

    // 判断版本(会检查NES头)
    m_Variant = CheckVariant(header);
    if (NesVariant::Variant_Invalid == m_Variant)
        return false;

    // 4: 读取PRG的个数(每个16KB)
    m_uNumberOfPRG = header[4];

    // 5: 读取CHR的个数(每个8KB)
    m_uNumberOfCHR = header[5];

    // 6: * 读取属性和Mapper的低半字节
    m_uFlags = header[6] & 0xFu;
    m_uMapper = header[6] >> 4u;

    // 是否有Trainer  (需要将这个数据映射到内存的$7000-$71FF, 也就是SRAM区域)
    if (HasTrainer())
    {
        m_TrainerData.resize(0x200u);
        stream.read((char *) m_TrainerData.data(), 0x200u);
    }

    // 老版本(兼容)的NES 不使用7-15
    if (m_Variant == NesVariant::Variant_ArchaiciNES)
        return true;

    // 读取Mapper号的高半字节
    m_uMapper |= header[7] & 0xF0u;

    // 解析Console Type
    m_ConsoleType = (ConsoleType) (header[7] & 0x3u);

    // 如果是iNES
    if (m_Variant == NesVariant::Variant_iNES)
    {
        // iNES的flag8是PRG内存大小
        m_uSizeOfPRGRAM = header[8] << 13u;

        // iNES的flag9的第0位是TV系统类型
        m_TVSystem = (TVSystem) (header[9] & 0x1u);

        return true;
    }

    // 之后的解析都是2.0的

    // 8 * 解析mapper(2.0变成12位)
    m_uMapper |= (uint16_t) ((header[8] & 0xFu) << 8u);

    // 8 * 解析子mapper号
    m_uSubMapper = header[8] >> 4u;

    // 9 * 解析PRG-ROM的高字节
    m_uNumberOfPRG |= (uint16_t) ((header[9] & 0xFu) << 8u);

    // 9 * 解析CHR-ROM的高字节
    m_uNumberOfCHR |= (uint16_t) ((uint16_t) (header[9] >> 4u) << 8u);

    // 10 * 解析PRG-RAM
    m_uSizeOfPRGRAM = header[10] & 0xFu;
    m_uSizeOfPRGRAM = m_uSizeOfPRGRAM > 0u ? 64u << m_uSizeOfPRGRAM : 0u;

    // 10 * 解析PRG-NVRAM
    m_uSizeOfPRGNVRAM = header[10] >> 4u;
    m_uSizeOfPRGNVRAM = m_uSizeOfPRGNVRAM > 0u ? 64u << m_uSizeOfPRGNVRAM : 0u;

    // 11 * 解析CHR-RAM
    m_uSizeOfCHRRAM = header[11] & 0xFu;
    m_uSizeOfCHRRAM = m_uSizeOfCHRRAM > 0u ? 64u << m_uSizeOfCHRRAM : 0u;

    // 11 * 解析PRG-NVRAM
    m_uSizeOfCHRNVRAM = header[11] >> 4u;
    m_uSizeOfCHRNVRAM = m_uSizeOfCHRNVRAM > 0u ? 64u << m_uSizeOfCHRNVRAM : 0u;

    // 12 * 解析TVSystem(TimingMode)
    m_TVSystem = (TVSystem) (header[12] & 0x3u);

    // 13 * 按ConsoleType去解析不同的值
    if (m_ConsoleType == ConsoleType::Console_VSSystem)
    {
        m_uPPUType = header[13] & 0xFu;
        m_uHardwareType = header[13] >> 4u;
    } else if (m_ConsoleType == ConsoleType::Console_Extended)
    {
        m_uExtendedConsoleType = header[13] & 0xFu;
    }
    // 14 * 解析Miscellaneous ROMs
    m_uMiscellaneousROMs = header[14] & 0x3u;

    // 15 * 解析Default Expansion Device
    m_uDefaultExpansionDevice = header[15] & 0x3Fu;

    // 解析完毕
    return true;
}

void CNesHeader::Unload()
{
    m_uNumberOfPRG = 0u;
    m_uNumberOfCHR = 0u;
    m_uFlags = 0u;
    m_uMapper = 0u;
    m_uSubMapper = 0u;
    m_uSizeOfPRGRAM = 0u,
    m_uSizeOfPRGNVRAM = 0u,
    m_uSizeOfCHRRAM = 0u,
    m_uSizeOfCHRNVRAM = 0u,

    m_uPPUType = 0u,
    m_uHardwareType = 0u,

    m_uExtendedConsoleType = 0u,
    m_uMiscellaneousROMs = 0u,

    m_uDefaultExpansionDevice = 0u,

    m_ConsoleType = ConsoleType::Console_NES;
    m_Variant = NesVariant::Variant_Invalid;
    m_TVSystem = TVSystem::TV_NTSC;
    m_TrainerData.clear();
}

bool CNesHeader::HasTrainer() const
{
    return (m_uFlags & NesHeaderFlag::Flag_Trainer) != 0;
}

bool CNesHeader::HasBettery() const
{
    return (m_uFlags & NesHeaderFlag::Flag_Battery) != 0;
}

uint16_t CNesHeader::Mapper() const
{
    return m_uMapper;
}

uint8_t CNesHeader::SubMapper() const
{
    return m_uSubMapper;
}

uint16_t CNesHeader::NumberOfPRG() const
{
    return m_uNumberOfPRG;
}

uint16_t CNesHeader::NumberOfCHR() const
{
    return m_uNumberOfCHR;
}

bool CNesHeader::HasVerticalMirroring() const
{
    return (m_uFlags & NesHeaderFlag::Flag_VerticalMirroring) != 0;
}

bool CNesHeader::HasFourScreenVRAM() const
{
    return (m_uFlags & NesHeaderFlag::Flag_FourScreenVRAM) != 0;
}

void CNesHeader::GetTrainer(string &data) const
{
    data.append(m_TrainerData.data(), m_TrainerData.size());
}

/*
 * <TODO> Recommended detection procedure:
 * If byte 7 AND $0C = $08, and the size taking into account byte 9 does not exceed the actual size of the ROM image, then NES 2.0.
 * If byte 7 AND $0C = $00, and bytes 12 - 15 are all 0, then iNES.
 * Otherwise, archaic iNES.
 * 现在先实现 ArchaiciNES , 目前这个判断的实现细节不是很重要!
*/
CNesHeader::NesVariant CNesHeader::CheckVariant(const uint8_t header[16])
{
    if (!(header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A))
        return NesVariant::Variant_Invalid;

#define HEADER_CHECK(i, c) (!header[i] || header[i] == c)
    // 全0 或 `DiskDude!`
    if (HEADER_CHECK(7, 'D') &&
        HEADER_CHECK(8, 'i') &&
        HEADER_CHECK(9, 's') &&
        HEADER_CHECK(10, 'k') &&
        HEADER_CHECK(11, 'D') &&
        HEADER_CHECK(12, 'u') &&
        HEADER_CHECK(13, 'd') &&
        HEADER_CHECK(14, 'e') &&
        HEADER_CHECK(15, '!')
            )
    {
        return NesVariant::Variant_ArchaiciNES;
    }
#undef HEADER_CHECK

    // 判断是不是扩展2.0版本
    if ((header[7] & 0x0Cu) == 0x08u)
    {
        return NesVariant::Variant_iNES_20;
    }

    // 2.0版本以下
    return NesVariant::Variant_iNES;
}
