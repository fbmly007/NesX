#!/usr/bin/python
# -*- coding: UTF-8 -*-
basePaletteList = []


def clamp_color(c):
    if c < 0:
        return 0
    elif c > 255:
        return 255
    return c


# ntsc强调系数
ntsc_emphasis_factors = (
    (1.00, 1.00, 1.00),
    (1.00, 0.80, 0.81),
    (0.78, 0.94, 0.66),
    (0.79, 0.77, 0.63),
    (0.82, 0.83, 1.12),  # 这块原先是1.12(怎么会有1.12呢?)
    (0.81, 0.71, 0.87),
    (0.68, 0.79, 0.79),
    (0.70, 0.70, 0.70)
)

finalPaletteList = []

# 读出底数
with open("FCEUX.pal", "rb") as f:
    while True:
        b = f.read(3)
        if not b:
            break
        basePaletteList.append((int(b[0]), int(b[1]), int(b[2])))


# 构造完整的列表
for idx in range(0, 8):
    f_r, f_g, f_b = ntsc_emphasis_factors[idx]
    for r, g, b in basePaletteList:
        finalPaletteList.append(
            (
                clamp_color(round(r * f_r)),
                clamp_color(round(g * f_g)),
                clamp_color(round(b * f_b))
            )
        )

# 生成c语言代码
nCol = 0
with open("palette.c", "w") as pf:
    pf.write("constexpr SDL_Color kVideoColorTable[] = {")
    for r, g, b in finalPaletteList:
        if not nCol:
            pf.write("\n    ")
            nCol = 4
        pf.write("{%d, %d, %d, 255}, " % (r, g, b))
        nCol = nCol - 1

    pf.write("\n};")
