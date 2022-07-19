
#ifndef NESX_NESX_LIB_INC_UNITS_COMMON_H_
#define NESX_NESX_LIB_INC_UNITS_COMMON_H_

#include <cstdint>
#include <cstring>
#include "Config.h"

typedef uint32_t Address;

// 屏幕宽与高
#define WINDOW_WIDTH 256
#define WINDOW_HEIGHT 240

// NES的宽与高
#define NES_SCREEN_WIDTH 256
#define NES_SCREEN_HEIGHT 240

#define SAFE_RELEASE(p) if (p) { delete (p); (p) = nullptr; }
#define SAFE_ARRAY_RELEASE(p) if(p) { delete[] (p); (p) = nullptr; }

// CPU中断向量
#define NMI_INT_HANDLER     0xFFFAu
#define RESET_INT_HANDLER   0xFFFCu
#define IRQ_INT_HANDLER     0xFFFEu
#define BRK_INT_HANDLER     IRQ_INT_HANDLER

// CPU信号
#define SIGNAL_REQUEST_NMI  0x1
#define SIGNAL_CANCEL_NMI   0x2
#define SIGNAL_REQUEST_IRQ  0x3
#define SIGNAL_CANCEL_IRQ   0x4

// Mapper数据线信号
#define DATA_SIGNAL_SCANLINE_COUNTER      0x10

// 默认一行不能显示超过8个精灵, 这样设置是为了可以让它超过此限制(未测试, 不要轻易修改!)
#define PPU_MAX_VISIBLE_SPRITE 8u

// 当大于等于8的时候, 需要将overflow设置为true
#define PPU_OVERFLOW_SPRITES 8u

#endif //NESX_NESX_LIB_INC_UNITS_COMMON_H_
