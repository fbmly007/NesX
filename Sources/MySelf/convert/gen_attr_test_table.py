
nCounter1 = 0
nCounter2 = 0
nIndex = 0
nStart = 0

for address in range(0x0000, 0x0400):
    print('0x%02X,' % nIndex, end="")
    if address & 0x1F == 0x1F:
        print("\n", end="")
    nCounter1 = nCounter1 + 1
    if nCounter1 >= 4:
        nIndex = nIndex + 1
        nCounter1 = 0
        if nIndex-nStart > 7:
            nCounter2 = nCounter2 + 1
            if nCounter2 >= 4:
                nStart = nStart + 8
                nCounter2 = 0
            nIndex = nStart






