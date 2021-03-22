
#ifndef NESX_CONFIG_H
#define NESX_CONFIG_H

// 设置宿主机的大小端 (如果目标系统没有定义这个宏, 可以自行设置)
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif

// 数据类型
union DataReg
{
    // 半字节不受影响
    struct { uint8_t  BL:4;  uint8_t  BH:4; };

#if LITTLE_ENDIAN
    struct { uint8_t  WL;    uint8_t  WH;   };
    struct { uint16_t DL;    uint16_t DH;   };
#else
    struct { uint8_t  WH;    uint8_t  WL;   };
    struct { uint16_t DH;    uint16_t DL;   };
#endif
    uint8_t  Raw[4];  // raw
    uint8_t       B;  // byte
    uint16_t      W;  // word
    uint32_t      D;  // dword
};

// 标志寄存器
union Flags
{
    // 从高位 -> 低位
    // N	....	Negative
    // V	....	Overflow
    // -	....	ignored
    // B	....	Break
    // D	....	Decimal (use BCD for arithmetics)
    // I	....	Interrupt (IRQ disable)
    // Z	....	Zero
    // C	....	Carry

    uint8_t Value;
    struct
    {
        uint8_t C : 1;  /*进位*/
        uint8_t Z : 1;  /*0位*/
        uint8_t I : 1;  /*禁止中断位*/
        uint8_t D : 1;  /*十进制模式(NES不支持)*/
        uint8_t B : 1;  /*BRK*/
        uint8_t X : 1;  /*未使用*/
        uint8_t V : 1;  /*溢出*/
        uint8_t N : 1;  /*负数*/
    };
};

#endif //NESX_CONFIG_H
