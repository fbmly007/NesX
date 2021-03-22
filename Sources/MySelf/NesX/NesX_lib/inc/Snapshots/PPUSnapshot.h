
#ifndef NESX_NESX_LIB_INC_SNAPSHOTS_PPUSNAPSHOT_H_
#define NESX_NESX_LIB_INC_SNAPSHOTS_PPUSNAPSHOT_H_

#include "Common.h"
#include "Units/PPUDef.h"

#pragma pack(push)
#pragma pack(1)
struct PPUSnapshot
{
    // BUS数据总线上最后一次写入的值
    uint8_t openBus;

    // 0-7端口寄存器
    uint8_t reg0;
    uint8_t reg1;
    uint8_t reg2;
    uint8_t oamAddr; // 2003/2004
    uint16_t reg6;

    // === 滚动 ===
    bool scrollLatch;
    uint16_t tmpReg6;
    uint8_t scrollOffsetX;   // 仅使用3位! (x:Fine X scroll)

    // === 周期 ===
    bool frameOdd;
    int  scanline;
    int  clocks;

    // === 精灵渲染 ===
    uint8_t primaryOAM[4*64];
    SpriteInfo nextOAMData[PPU_MAX_VISIBLE_SPRITE + 1];     // 存放下一行的OAM数据
    SpriteInfo activedOAMData[PPU_MAX_VISIBLE_SPRITE];  // 当前要绘制的OAM数据

    bool clearNextOAMSignal;
    // n and m (仅用再Sprite Evaluation阶段)

    uint8_t tempOAMValue;       // 分周期获取值的临时值(以前是static, 不过为了保存状态, 这里放到成员里来!)
    uint8_t tempOAMIndex;       // n(0-63)
    uint8_t tempOAMByteOffset;  // m(0-3)
    uint8_t numberOfSpriteFound;// 找到的精灵个数
    bool evaluationOverflows;   // 溢出

    // >>===== 背景渲染 =====<<

    // 当前图块的8个像素
    uint8_t bgActivedPatternDataL;
    uint8_t bgActivedPatternDataH;

    // 当前图块的8个像素属性
    uint8_t bgActivedAttributeDataL;
    uint8_t bgActivedAttributeDataH;

    // 当前所使用的属性锁存器(每个是1 bit)
    bool bgNextAttributeLatchL;
    bool bgNextAttributeLatchH;

    // 下一个图块的8个像素
    uint8_t bgNextPatternDataL;
    uint8_t bgNextPatternDataH;


    // 8个获取周期时的临时锁存器
    bool enableBGShifter;  // 是否将临时锁存器的内容加载到 Next 中

    uint8_t tmpNTByteLatch;         // 获取Name Table字节(Tile #)
    uint8_t tmpATByteLatch;         // 获取属性(仅使用2位)
    uint8_t tmpBGPatternLatchL;     // 获取Pattern低位字节(+0)
    uint8_t tmpBGPatternLatchH;     // 获取Pattern高位字节(+8)

    Address tmpBGAddress;           // 用于存放上个周期的背景图块或属性地址(以前也是放在static中的)


    // >>===== Internal RAM =====<<
    // Name Table
    uint8_t nameTables[0x1000]; // 额外定义的2KB 是为了方便在Cartridge中直接控制4-Screen.(其他镜像模式不使用后面的2KB)
    // Palette
    uint8_t palette[0x20];
};
#pragma pack(pop)

#endif //NESX_NESX_LIB_INC_SNAPSHOTS_PPUSNAPSHOT_H_
