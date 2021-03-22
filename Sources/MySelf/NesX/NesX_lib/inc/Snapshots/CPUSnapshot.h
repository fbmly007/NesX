
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_CPUSNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_CPUSNAPSHOT_H_

#include "Common.h"

#pragma pack(push)
#pragma pack(1)
struct CPUSnapshot
{
    int cyclesToWait;    // 等待下个周期的次数
    uint8_t ram[0x800u]; // 内存

    // 内部寄存器
    uint16_t PC;          // PC(16bit)
    uint8_t A;            // 算数寄存器(bit)
    uint8_t X;            // X寄存器(8bit)
    uint8_t Y;            // Y寄存器(8bit)
    uint8_t SR;           // 状态寄存器(8bit)
    uint8_t SP;           // 栈指针 (8bit)
    bool nmi;             // 是否正在发生非屏蔽中断
    bool irq;             // 是否正在发生中断请求
    bool cycleOdd;        // 当前是否为奇数周期
    bool cross;           // 最后执行的指令是否跨页
    uint8_t countOfWrite; // 同一个指令下连续写的个数
};
#pragma pack(pop)

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_CPUSNAPSHOT_H_
