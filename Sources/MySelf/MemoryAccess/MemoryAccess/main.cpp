#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "MemoryMap.h"

int main(void)
{
	CMemoryMap map(0x10000u, 13u);
	/*1*/map.SetPageInfo(0u, { 0u,  MemoryProtect::Write, nullptr });  /*0000-1FFF*/  // RAM 2KB and Mirroring*
	/*2*/map.SetPageInfo(1u, { 0u,  MemoryProtect::Read , nullptr });  /*2000-3FFF*/  // Registers for PPU and Mirroring*
	/*3*/map.SetPageInfo(2u, { 0u,  MemoryProtect::Read , nullptr });  /*4000-5FFF*/  // Registers for APU and Expanded ROM
	/*4*/map.SetPageInfo(3u, { 0u,  MemoryProtect::Write, nullptr });  /*6000-7FFF*/  // SRAM (8KB), 根据不同属性, 这里会加Write属性来允许使用这块空间
	/*5*/map.SetPageInfo(4u, { 0u,  MemoryProtect::Read , nullptr });  /*8000-9FFF*/  // PRG-ROM 1
	/*6*/map.SetPageInfo(5u, { 0u,  MemoryProtect::Read , nullptr });  /*A000-BFFF*/  // ---------
	/*7*/map.SetPageInfo(6u, { 5u,  MemoryProtect::Read , nullptr });  /*C000-DFFF*/  // PRG-ROM 2 (*)
	/*8*/map.SetPageInfo(7u, { 6u,  MemoryProtect::Read , nullptr });  /*E000-FFFF*/  // ---------

	// 如果存在SRAM, 需要在此注册一下, 以便外部可以接收到改变 并保存新的.sav文件 
	map.RegisterPageChangedHook(3u);

	// registers for PPU
	map.RegisterPorts(0x2000u, 0x8u);

	// registers for APU and CPU Test mode
	map.RegisterPorts(0x4000u, 0x20u);
	
	map.ReadByte(0x4000u);

	map.WriteByte(0xE012u, 10u);
	map.WriteByte(0x1FFFu, 0x8u);

	printf("%d\n", sizeof MemoryPageInformation);
	printf("%u\n", map.ReadByte(0x1FFFu));
	system("pause");
	return 0;
}