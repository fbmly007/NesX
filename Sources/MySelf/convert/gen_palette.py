#!/usr/bin/python
# -*- coding: UTF-8 -*-
nCol = 0
with open("RP2C03.pal", "rb") as f:
    with open("palette.c", "w") as pf:
        pf.write("constexpr SDL_Color kVideoColorTable[] = {")

        while True:
            b = f.read(3)
            if not b:
                break
            if not nCol:
                pf.write("\n    ")
                nCol = 4
            pf.write("{%d, %d, %d, 255}, " % (b[0], b[1], b[2]))
            nCol = nCol - 1

        pf.write("\n};")
