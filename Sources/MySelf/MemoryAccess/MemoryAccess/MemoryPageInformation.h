#pragma once
#include <stdint.h>

// 类型定义
typedef uint16_t Address;
typedef uint32_t SegmentIndex;
typedef void(*RedirectFunc)(Address&);

// 当前保护属性 
enum MemoryProtect : uint8_t
{
	Read,
	Write
};

// 内存页信息
struct MemoryPageInformation
{
	SegmentIndex MirrorPage; // [1-n]
	MemoryProtect Protect;   // 保护属性
	RedirectFunc Func;       // 映射地址
};
