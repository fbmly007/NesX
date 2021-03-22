
#ifndef NESX_NESX_LIB_INC_UNITS_PPUDEF_H_
#define NESX_NESX_LIB_INC_UNITS_PPUDEF_H_

// 0-239    Visible line
// 240      Post-render line
// 241-260  VBlank line
// 261      Pre-render line (-1)
enum ScanLineType
{
  Visible,
  PostRender,
  VBlank,
  PreRender
};

// 属性信息
union AttributeInfo
{
  uint8_t Value;
  struct
  {
    uint8_t Palette         : 2;
    uint8_t Unused          : 3;
    bool BehindBackground   : 1;
    bool FlipHorizontal     : 1;
    bool FlipVertical       : 1;
  };
};

// 精灵结构
#pragma pack(push)
#pragma pack(1)
struct SpriteInfo
{
  uint8_t id;
  uint8_t PositionY;
  uint8_t TileIndex;
  uint8_t Attribute;
  uint8_t PositionX;

  uint8_t PatternDataL;  // Tile data (low).
  uint8_t PatternDataH;  // Tile data (high).
};
#pragma pack(pop)

#define NTH_BIT(x, n) (((x) >> (n)) & 1)
struct Sprite2
{
  uint8_t id;     // Index in OAM.
  uint8_t x;      // X position.
  uint8_t y;      // Y position.
  uint8_t tile;   // Tile index.
  uint8_t attr;   // Attributes.
  uint8_t dataL;  // Tile data (low).
  uint8_t dataH;  // Tile data (high).
};

// 寄存器2000的结构(PPUCTRL)
union Reg2000
{
  uint8_t Value;
  struct
  {
    // Base Nametable address
    // 0 = $2000 (VRAM)
    // 1 = $2400 (VRAM)
    // 2 = $2800 (VRAM)
    // 3 = $2C00 (VRAM)
    uint8_t NameTableIndex          : 2;

    // 0 = 每次递增1
    // 1 = 每次递增32 (0x20)
    uint8_t Increment               : 1;

    // 0 = $0000 (VRAM)
    // 1 = $1000 (VRAM)
    // (ignored in 8x16 mode???)
    uint8_t SpritePatternAddress    : 1;

    // 0 = $0000 (VRAM)
    // 1 = $1000 (VRAM)
    uint8_t ScreenPatternAddress    : 1;

    // 0 = 8x8
    // 1 = 8x16
    uint8_t SpriteSize              : 1;

    // 0 = Disabled
    // 1 = Enabled
    // PPU master/slave select???
    // (0: read backdrop from EXT pins; 1: output color on EXT pins)???
    uint8_t NMIOnSpriteHit          : 1;

    // 0 = Disabled
    // 1 = Enabled
    uint8_t NMIOnVBlank             : 1;
  };
};

// 寄存器2001的结构(PPUMASK)
union Reg2001
{
  uint8_t Value;
  struct
  {
    // 0: normal color
    // 1: produce a greyscale display
    bool Greyscale   : 1;

    // 0 = Don't show the left 8 pixels of screen
    // 1 = Show the left 8 pixels
    bool ImageClip          : 1;

    // 0 = Don't show sprites in the left 8-pixel column
    // 1 = Show sprites everywhere
    bool SpriteClip         : 1;

    // 0 = Off
    // 1 = On
    bool ScreenDisplay      : 1;

    // 0 = Hide sprites
    // 1 = Show sprites
    bool SpriteDisplay      : 1;

    uint8_t EmphasizeRed    : 1;
    uint8_t EmphasizeGreen  : 1;
    uint8_t EmphasizeBlue   : 1;
  };
};

// 寄存器2002的结构(PPUSTATUS)
union Reg2002
{
  uint8_t Value;
  struct
  {
    uint8_t Unused              : 5;
    bool LostSprites            : 1;
    bool Hit                    : 1;
    bool InVBlank               : 1;
  };
};

// 寄存器2006的结构(PPUADDR)
// OOO.NN.YYYYY.XXXXX
// X 表示为 X scroll counter.
// Y 表示为 Y scroll counter.
// N 表示为 nametable select bits.
// O 表示为 Y scroll offset
union Reg2006
{
  struct
  {
    uint8_t CoarseX     : 5;
    uint8_t CoarseY     : 5;
    uint8_t NameTable   : 2;
    uint8_t FineY       : 3;
  };

  // * 很感谢LaiNES 让我学到了这种技巧! (现在有些不想感谢了!)
  struct
  {
    uint8_t L : 8;
    uint8_t H : 7;
  };
  uint16_t PC : 14;    //PC占14位 0-0x3FFF
  uint16_t Value : 15;
};

#endif //NESX_NESX_LIB_INC_UNITS_PPUDEF_H_
