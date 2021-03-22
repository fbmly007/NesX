#include <iostream>
#include <cassert>

// case 1
constexpr int numprg = 0x8;
constexpr int numchr = 0x10;
constexpr int prgSize = numprg*0x4000;
constexpr int chrSize = numchr*0x2000;

/* PRG mapping functions */
template <int pageKBs> void map_prg(int slot, int bank)
{
    if (bank < 0)
        bank = (prgSize / (0x400*pageKBs)) + bank;

    printf("============================\n");
    //printf("pageKBs = %d, slot = %d, bank = %d\n",
    //    pageKBs, slot, bank);

    for (int i = 0; i < (pageKBs/8); i++)
    {
        printf("prgMap[%d] = %X\n",
               (pageKBs/8) * slot + i,
               (pageKBs*0x400*bank + 0x2000*i) % prgSize);
    }

    printf("============================\n");
}
template void map_prg<32>(int, int);
template void map_prg<16>(int, int);
template void map_prg<8> (int, int);

/* CHR mapping functions */
// 1KB
template <int pageKBs> void map_chr(int slot, int bank)
{
    printf("============================\n");
    //printf("pageKBs = %d, slot = %d, bank = %d\n",
    //       pageKBs, slot, bank);

    for (int i = 0; i < pageKBs; i++)
    {
        printf("chrMap[%d] = %X\n",
               pageKBs*slot + i,
               (pageKBs*0x400*bank + 0x400*i) % chrSize
        );
    }
    printf("============================\n");

}
template void map_chr<8>(int, int);
template void map_chr<4>(int, int);
template void map_chr<2>(int, int);
template void map_chr<1>(int, int);

size_t BanksOfPRGIn8K()
{
    return numprg*2;
}

size_t BanksOfCHRIn1K()
{
    return numchr*8;
}


void SelectCHRBankIn1K(const int& slot, const size_t &uBank)
{
    assert(slot >= 0 && slot < 8);
    printf("chrMap[%d] = %X\n", slot, ((uBank % BanksOfCHRIn1K()) << 10u));
}


void SelectCHRBankIn2K(const int& slot, const size_t &uBank)
{
    SelectCHRBankIn1K(slot + slot, uBank + uBank);
    SelectCHRBankIn1K(slot + slot + 1, uBank + uBank + 1u);
}

void SelectCHRBankIn4K(const int& slot, const size_t &uBank)
{
    SelectCHRBankIn2K(slot + slot, uBank + uBank);
    SelectCHRBankIn2K(slot + slot + 1, uBank + uBank + 1u);
}

void SelectCHRBankIn8K(const size_t &uBank)
{
    SelectCHRBankIn4K(0, uBank + uBank);
    SelectCHRBankIn4K(1, uBank + uBank + 1u);
}





void SelectPRGBankIn8K(const int &slot, const size_t &uBank)
{
    assert(slot >= 0 && slot < 4);
    printf("prgMap[%d] = %X\n", slot, ((uBank % BanksOfPRGIn8K()) << 13u));
}

void SelectPRGBankIn16K(const int &slot, const size_t &uBank)
{
    SelectPRGBankIn8K(slot + slot, uBank + uBank);
    SelectPRGBankIn8K(slot + slot + 1, uBank + uBank + 1u);
}

void SelectPRGBankIn32K(const size_t &uBank)
{
    SelectPRGBankIn16K(0, uBank + uBank);
    SelectPRGBankIn16K(1, uBank + uBank + 1u);
}




// case 2
int main()
{
    uint8_t regs[8] = {1,2,3,4,5,6,7,8};

    for (int i = 0; i < 4; i++)
    {
        map_chr<1>(i, regs[2 + i]);
        SelectCHRBankIn1K(i, regs[2 + i]);
    }

    return 0;
}