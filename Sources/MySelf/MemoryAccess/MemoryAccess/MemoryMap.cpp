#include "MemoryMap.h"
#include "NotificationCenter.h"
#include "MessageDef.h"
#include <assert.h>
#include <stdio.h>

// 常用的转换
#define MIRROR_ADDR(addr, index) (index << m_uShiftCount) + (addr & (m_uSizePerPage - 1))
#define PAGE_INDEX(addr) ((addr) >> m_uShiftCount)

// 将重复代码提取到这个地方 (不使用函数调用)
#define CALC_PAGE_AND_ADDR(addr) \
SegmentIndex index = PAGE_INDEX(addr);\
assert(index < m_uNumberOfPages);\
if (m_pPages[index].MirrorPage > 0)\
{\
	assert(m_pPages[index].MirrorPage <= m_uNumberOfPages);\
	index = m_pPages[index].MirrorPage - 1;\
	addr = MIRROR_ADDR(addr, index);\
}\
const MemoryPageInformation& info = m_pPages[index];\
if (info.Func) {info.Func(addr);}\
assert(addr < m_uSizeOfMemory);

CMemoryMap::CMemoryMap(size_t uSize, uint8_t uShiftCount)
{
	m_pMemory = new uint8_t[uSize]();
	m_uSizeOfMemory = uSize;
	m_uShiftCount = uShiftCount;
	m_uNumberOfPages = uSize >> uShiftCount;
	m_uSizePerPage = 1 << uShiftCount;
	m_pPages = new MemoryPageInformation[m_uNumberOfPages]();
}

void CMemoryMap::SetPageInfo(SegmentIndex index, const MemoryPageInformation & info)
{
	assert(index < m_uNumberOfPages);
	assert(info.MirrorPage == 0 || (info.MirrorPage <= m_uNumberOfPages && m_pPages[info.MirrorPage - 1].MirrorPage == 0));
	m_pPages[index] = info;
}

void CMemoryMap::RegisterPort(Address addr)
{
	assert(addr < m_uSizeOfMemory);
	m_Ports.insert(addr);
}

void CMemoryMap::RegisterPorts(Address addr, uint16_t size)
{
	assert(addr < m_uSizeOfMemory);
	assert(size > 0);
	assert((size_t)(addr + size) <= m_uSizeOfMemory);
	for (uint16_t index = 0; index < size; ++index)
		RegisterPort(addr + index);
}

void CMemoryMap::RegisterPageChangedHook(SegmentIndex index)
{
	assert(index < m_uNumberOfPages);
	m_ChangedHooks.insert(index);
}

uint8_t CMemoryMap::ReadByte(Address addr) const
{
	// 将常用的代码提取为宏定义
	CALC_PAGE_AND_ADDR(addr)

	// 判断是否为端口
	unordered_set<Address>::const_iterator Iter = m_Ports.find(addr);
	if (Iter != m_Ports.cend())
	{
		return PortRead(addr);
	}

	return m_pMemory[addr];
}

uint16_t CMemoryMap::ReadWord(Address addr) const
{
	// Little-Endian
	return ReadByte(addr) | ((uint16_t)ReadByte(addr + 1u)) << 8u;
}

void CMemoryMap::RawReadBytes(uint8_t * buffer, uint16_t bufferSize, Address addr, uint16_t sizeToCopy)
{
	assert(buffer);
	assert(bufferSize > 0);
	assert((size_t)(addr + sizeToCopy) <= m_uSizeOfMemory);
	assert(bufferSize >= sizeToCopy);
	memcpy(buffer, m_pMemory + addr, sizeToCopy);
}

void CMemoryMap::WriteByte(Address addr, uint8_t value)
{
	CALC_PAGE_AND_ADDR(addr)

	// 检查是否为端口
	unordered_set<Address>::const_iterator Iter = m_Ports.find(addr);
	if (Iter != m_Ports.cend())
	{
		PortWrite(addr, value);
		return;
	}

	// 没有权限写入!
	if (info.Protect != MemoryProtect::Write)
		return;

	uint8_t rawValue = m_pMemory[addr];
	m_pMemory[addr] = value;
	if (rawValue != value)
	{
		// 发生改变, 尝试通知
		unordered_set<SegmentIndex>::const_iterator IterChanged = m_ChangedHooks.find(index);
		if (IterChanged != m_ChangedHooks.cend())
		{
			NotifyChange(index, addr);
		}
	}
}

void CMemoryMap::WriteWord(Address addr, uint16_t value)
{
	// Little-Endian
	WriteByte(addr, value & 0xFFu);
	WriteByte(addr + 1, value >> 8u);
}

void CMemoryMap::RawWriteBytes(Address addr, uint8_t * buffer, uint16_t size)
{
	assert(buffer);
	assert(size > 0);
	assert((size_t)(addr + size) <= m_uSizeOfMemory);
	memcpy(m_pMemory + addr, buffer, size);
}

uint8_t *CMemoryMap::UNSAFE_GetRawData(Address addr)
{
	assert(addr < m_uSizeOfMemory);
	return m_pMemory + addr;
}

Address CMemoryMap::GetBaseAddress(SegmentIndex index)
{
	return index << m_uShiftCount;
}

uint8_t CMemoryMap::PortRead(Address port) const
{
	return (uint8_t)(size_t)CNotificationCenter::GetInst()->Notify(PORT_READ_SIGNAL, (void *)port, nullptr, this);
}

void CMemoryMap::PortWrite(Address port, uint8_t value)
{
	CNotificationCenter::GetInst()->Notify(PORT_WRITE_SIGNAL, (void *)port, (void *)value, this);
}

void CMemoryMap::NotifyChange(SegmentIndex index, Address addr)
{
	CNotificationCenter::GetInst()->Notify(PAGE_CHANGED_ON_WRITE, (void *)index, (void *)addr, this);
}
