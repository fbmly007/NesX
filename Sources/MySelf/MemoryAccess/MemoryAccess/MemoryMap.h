#pragma once

#include "MemoryPageInformation.h"

#include <unordered_set>
using std::unordered_set;

class CMemoryMap
{
public:
	CMemoryMap(size_t uSize, uint8_t uShiftCount);

public:
	// 注册内存页属性
	void SetPageInfo(SegmentIndex index, const MemoryPageInformation& info);
	void RegisterPort(Address addr);
	void RegisterPorts(Address addr, uint16_t size);
	void RegisterPageChangedHook(SegmentIndex index);

	// 读
	uint8_t ReadByte(Address addr) const;
	uint16_t ReadWord(Address addr) const;
	void RawReadBytes(uint8_t *buffer, uint16_t bufferSize, Address addr, uint16_t sizeToCopy);

	// 写
	void WriteByte(Address addr, uint8_t value);
	void WriteWord(Address addr, uint16_t value);
	void RawWriteBytes(Address addr, uint8_t *buffer, uint16_t size);

	// 原始
	uint8_t *UNSAFE_GetRawData(Address addr = 0u);
	Address GetBaseAddress(SegmentIndex index);

private:
	uint8_t PortRead(Address port) const;
	void PortWrite(Address port, uint8_t value);
	void NotifyChange(SegmentIndex index, Address addr);

private:
	uint8_t *m_pMemory;								// 原始内存
	size_t m_uSizeOfMemory;							// 内存大小
	MemoryPageInformation *m_pPages;				// 页信息数组
	uint32_t m_uNumberOfPages;						// 页数
	uint8_t  m_uShiftCount;							// log2(m_uSizePerPage), 用于移位运算
	Address  m_uSizePerPage;						// 用于求余

	unordered_set<Address> m_Ports;					// 用于端口映射
	unordered_set<SegmentIndex> m_ChangedHooks;		// 用于监视当页内容发生改变时, 发送Hook
};

