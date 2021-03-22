
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 下面是生成的数据代码
typedef unsigned char Byte;
typedef unsigned short Word;

static Byte *s_Mnemonic[] =
{
	"ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI",
	"BNE","BPL","BRK","BVC","BVS","CLC","CLD","CLI",
	"CLV","CMP","CPX","CPY","DEC","DEX","DEY","EOR",
	"INC","INX","INY","JMP","JSR","LDA","LDX","LDY",
	"LSR","NOP","ORA","PHA","PHP","PLA","PLP","ROL",
	"ROR","RTI","RTS","SBC","SEC","SED","SEI","STA",
	"STX","STY","TAX","TAY","TSX","TXA","TXS","TYA",
};


enum addressingMode
{
	invalid = 0,
	immidiate = 1,
	zeropage = 2,
	zeropageX = 3,
	absolute = 4,
	absoluteX = 5,
	absoluteY = 6,
	indirectX = 7,
	indirectY = 8,
	accumulator = 9,
	relative = 10,
	implied = 11,
	indirect = 12,
	zeropageY = 13,
};

struct _opcode
{
	int code;
	int mnId;
	int addressingId;
	int reserved;
};
typedef struct _opcode opcode;
typedef struct _opcode*popcode;

static opcode s_Opcodes[256] =
{
	{0x00,11,11, 0},	{0x01,35, 7, 0},	{0x02, 0, 0, 0},	{0x03, 0, 0, 0},	{0x04, 0, 0, 0},	{0x05,35, 2, 0},	{0x06, 3, 2, 0},	{0x07, 0, 0, 0},
	{0x08,37,11, 0},	{0x09,35, 1, 0},	{0x0A, 3, 9, 0},	{0x0B, 0, 0, 0},	{0x0C, 0, 0, 0},	{0x0D,35, 4, 0},	{0x0E, 3, 4, 0},	{0x0F, 0, 0, 0},
	{0x10,10,10, 0},	{0x11,35, 8, 0},	{0x12, 0, 0, 0},	{0x13, 0, 0, 0},	{0x14, 0, 0, 0},	{0x15,35, 3, 0},	{0x16, 3, 3, 0},	{0x17, 0, 0, 0},
	{0x18,14,11, 0},	{0x19,35, 6, 0},	{0x1A, 0, 0, 0},	{0x1B, 0, 0, 0},	{0x1C, 0, 0, 0},	{0x1D,35, 5, 0},	{0x1E, 3, 5, 0},	{0x1F, 0, 0, 0},
	{0x20,29, 4, 0},	{0x21, 2, 7, 0},	{0x22, 0, 0, 0},	{0x23, 0, 0, 0},	{0x24, 7, 2, 0},	{0x25, 2, 2, 0},	{0x26,40, 2, 0},	{0x27, 0, 0, 0},
	{0x28,39,11, 0},	{0x29, 2, 1, 0},	{0x2A,40, 9, 0},	{0x2B, 0, 0, 0},	{0x2C, 7, 4, 0},	{0x2D, 2, 4, 0},	{0x2E,40, 4, 0},	{0x2F, 0, 0, 0},
	{0x30, 8,10, 0},	{0x31, 2, 8, 0},	{0x32, 0, 0, 0},	{0x33, 0, 0, 0},	{0x34, 0, 0, 0},	{0x35, 2, 3, 0},	{0x36,40, 3, 0},	{0x37, 0, 0, 0},
	{0x38,45,11, 0},	{0x39, 2, 6, 0},	{0x3A, 0, 0, 0},	{0x3B, 0, 0, 0},	{0x3C, 0, 0, 0},	{0x3D, 2, 5, 0},	{0x3E,40, 5, 0},	{0x3F, 0, 0, 0},
	{0x40,42,11, 0},	{0x41,24, 7, 0},	{0x42, 0, 0, 0},	{0x43, 0, 0, 0},	{0x44, 0, 0, 0},	{0x45,24, 2, 0},	{0x46,33, 2, 0},	{0x47, 0, 0, 0},
	{0x48,36,11, 0},	{0x49,24, 1, 0},	{0x4A,33, 9, 0},	{0x4B, 0, 0, 0},	{0x4C,28, 4, 0},	{0x4D,24, 4, 0},	{0x4E,33, 4, 0},	{0x4F, 0, 0, 0},
	{0x50,12,10, 0},	{0x51,24, 8, 0},	{0x52, 0, 0, 0},	{0x53, 0, 0, 0},	{0x54, 0, 0, 0},	{0x55,24, 3, 0},	{0x56,33, 3, 0},	{0x57, 0, 0, 0},
	{0x58,16,11, 0},	{0x59,24, 6, 0},	{0x5A, 0, 0, 0},	{0x5B, 0, 0, 0},	{0x5C, 0, 0, 0},	{0x5D,24, 5, 0},	{0x5E,33, 5, 0},	{0x5F, 0, 0, 0},
	{0x60,43,11, 0},	{0x61, 1, 7, 0},	{0x62, 0, 0, 0},	{0x63, 0, 0, 0},	{0x64, 0, 0, 0},	{0x65, 1, 2, 0},	{0x66,41, 2, 0},	{0x67, 0, 0, 0},
	{0x68,38,11, 0},	{0x69, 1, 1, 0},	{0x6A,41, 9, 0},	{0x6B, 0, 0, 0},	{0x6C,28,12, 0},	{0x6D, 1, 4, 0},	{0x6E,41, 4, 0},	{0x6F, 0, 0, 0},
	{0x70,13,10, 0},	{0x71, 1, 8, 0},	{0x72, 0, 0, 0},	{0x73, 0, 0, 0},	{0x74, 0, 0, 0},	{0x75, 1, 3, 0},	{0x76,41, 3, 0},	{0x77, 0, 0, 0},
	{0x78,47,11, 0},	{0x79, 1, 6, 0},	{0x7A, 0, 0, 0},	{0x7B, 0, 0, 0},	{0x7C, 0, 0, 0},	{0x7D, 1, 5, 0},	{0x7E,41, 5, 0},	{0x7F, 0, 0, 0},
	{0x80, 0, 0, 0},	{0x81,48, 7, 0},	{0x82, 0, 0, 0},	{0x83, 0, 0, 0},	{0x84,50, 2, 0},	{0x85,48, 2, 0},	{0x86,49, 2, 0},	{0x87, 0, 0, 0},
	{0x88,23,11, 0},	{0x89, 0, 0, 0},	{0x8A,54,11, 0},	{0x8B, 0, 0, 0},	{0x8C,50, 4, 0},	{0x8D,48, 4, 0},	{0x8E,49, 4, 0},	{0x8F, 0, 0, 0},
	{0x90, 4,10, 0},	{0x91,48, 8, 0},	{0x92, 0, 0, 0},	{0x93, 0, 0, 0},	{0x94,50, 3, 0},	{0x95,48, 3, 0},	{0x96,49,13, 0},	{0x97, 0, 0, 0},
	{0x98,56,11, 0},	{0x99,48, 6, 0},	{0x9A,55,11, 0},	{0x9B, 0, 0, 0},	{0x9C, 0, 0, 0},	{0x9D,48, 5, 0},	{0x9E, 0, 0, 0},	{0x9F, 0, 0, 0},
	{0xA0,32, 1, 0},	{0xA1,30, 7, 0},	{0xA2,31, 1, 0},	{0xA3, 0, 0, 0},	{0xA4,32, 2, 0},	{0xA5,30, 2, 0},	{0xA6,31, 2, 0},	{0xA7, 0, 0, 0},
	{0xA8,52,11, 0},	{0xA9,30, 1, 0},	{0xAA,51,11, 0},	{0xAB, 0, 0, 0},	{0xAC,32, 4, 0},	{0xAD,30, 4, 0},	{0xAE,31, 4, 0},	{0xAF, 0, 0, 0},
	{0xB0, 5,10, 0},	{0xB1,30, 8, 0},	{0xB2, 0, 0, 0},	{0xB3, 0, 0, 0},	{0xB4,32, 3, 0},	{0xB5,30, 3, 0},	{0xB6,31,13, 0},	{0xB7, 0, 0, 0},
	{0xB8,17,11, 0},	{0xB9,30, 6, 0},	{0xBA,53,11, 0},	{0xBB, 0, 0, 0},	{0xBC,32, 5, 0},	{0xBD,30, 5, 0},	{0xBE,31, 6, 0},	{0xBF, 0, 0, 0},
	{0xC0,20, 1, 0},	{0xC1,18, 7, 0},	{0xC2, 0, 0, 0},	{0xC3, 0, 0, 0},	{0xC4,20, 2, 0},	{0xC5,18, 2, 0},	{0xC6,21, 2, 0},	{0xC7, 0, 0, 0},
	{0xC8,27,11, 0},	{0xC9,18, 1, 0},	{0xCA,22,11, 0},	{0xCB, 0, 0, 0},	{0xCC,20, 4, 0},	{0xCD,18, 4, 0},	{0xCE,21, 4, 0},	{0xCF, 0, 0, 0},
	{0xD0, 9,10, 0},	{0xD1,18, 8, 0},	{0xD2, 0, 0, 0},	{0xD3, 0, 0, 0},	{0xD4, 0, 0, 0},	{0xD5,18, 3, 0},	{0xD6,21, 3, 0},	{0xD7, 0, 0, 0},
	{0xD8,15,11, 0},	{0xD9,18, 6, 0},	{0xDA, 0, 0, 0},	{0xDB, 0, 0, 0},	{0xDC, 0, 0, 0},	{0xDD,18, 5, 0},	{0xDE,21, 5, 0},	{0xDF, 0, 0, 0},
	{0xE0,19, 1, 0},	{0xE1,44, 7, 0},	{0xE2, 0, 0, 0},	{0xE3, 0, 0, 0},	{0xE4,19, 2, 0},	{0xE5,44, 2, 0},	{0xE6,25, 2, 0},	{0xE7, 0, 0, 0},
	{0xE8,26,11, 0},	{0xE9,44, 1, 0},	{0xEA,34,11, 0},	{0xEB, 0, 0, 0},	{0xEC,19, 4, 0},	{0xED,44, 4, 0},	{0xEE,25, 4, 0},	{0xEF, 0, 0, 0},
	{0xF0, 6,10, 0},	{0xF1,44, 8, 0},	{0xF2, 0, 0, 0},	{0xF3, 0, 0, 0},	{0xF4, 0, 0, 0},	{0xF5,44, 3, 0},	{0xF6,25, 3, 0},	{0xF7, 0, 0, 0},
	{0xF8,46,11, 0},	{0xF9,44, 6, 0},	{0xFA, 0, 0, 0},	{0xFB, 0, 0, 0},	{0xFC, 0, 0, 0},	{0xFD,44, 5, 0},	{0xFE,25, 5, 0},	{0xFF, 0, 0, 0},
};

static void newLine(FILE *fp)
{
	fprintf(fp, "\n");
}

static char s_FileName[256];

// 初始化文件名
static int initFileName(const char *szFullName)
{
	char *p = strrchr(szFullName, '/');
	if (!p) p = strrchr(szFullName, '\\');
	if (!p) return 0;
	++p;
	strcpy(s_FileName, p);
	return 1;
}

static size_t disassembler(const Byte *pBuf, size_t pc, FILE *fout)
{
	const Byte *p = pBuf;

	popcode pcode = s_Opcodes + (*p++);

	fprintf(fout, "$%04X\t", pc);

	switch (pcode->addressingId)
	{
	case immidiate: 
		fprintf(fout, "%s #$%02X", s_Mnemonic[pcode->mnId-1], *p++);
		break;

	case zeropage: 
		fprintf(fout, "%s $%02X", s_Mnemonic[pcode->mnId - 1], *p++);
		break;

	case zeropageX: 
		fprintf(fout, "%s $%02X, X", s_Mnemonic[pcode->mnId - 1], *p++);
		break;

	case absolute:
		fprintf(fout, "%s $%02X%02X", s_Mnemonic[pcode->mnId - 1], p[1], p[0]);
		p += 2;
		break;

	case absoluteX:
		fprintf(fout, "%s $%02X%02X, X", s_Mnemonic[pcode->mnId - 1], p[1], p[0]);
		p += 2;
		break;

	case absoluteY: 
		fprintf(fout, "%s $%02X%02X, Y", s_Mnemonic[pcode->mnId - 1], p[1], p[0]);
		p += 2;
		break;

	case indirectX: 
		fprintf(fout, "%s ($%02X, X)", s_Mnemonic[pcode->mnId - 1], *p++);
		break;

	case indirectY: 
		fprintf(fout, "%s ($%02X), Y", s_Mnemonic[pcode->mnId - 1], *p++);
		break;

	case accumulator: 
		fprintf(fout, "%s A", s_Mnemonic[pcode->mnId - 1]);
		break;

	case relative: 
		pc += 2;
		pc += (int)(char)*p++;
		fprintf(fout, "%s $%04X (rel)", s_Mnemonic[pcode->mnId - 1], pc);
		break;

	case implied: 
		fprintf(fout, "%s", s_Mnemonic[pcode->mnId - 1]);
		break;

	case indirect: 
		fprintf(fout, "%s ($%02X%02X)", s_Mnemonic[pcode->mnId - 1], p[1], p[0]);
		p += 2;
		break;

	case zeropageY: 
		fprintf(fout, "%s $%02X, Y", s_Mnemonic[pcode->mnId - 1], *p++);
		break;

	default: 
		fprintf(fout, "unresolved opcode: <%02X>", pcode->code);
		break;
	}

	return p - pBuf;
}

static int loadFile(const char *szFileName, FILE *fout)
{
#define MAXBUF_LENGTH 16
#define MAXBUF_HALF_LENGTH 8

	static Byte tmpBuf[MAXBUF_LENGTH];

	FILE *fp = fopen(szFileName, "rb");
	if (!fp) return errno;

	// header
	fprintf(
		fout,
		"=================== [asm of %s] ===================",
		szFileName
	);
	newLine(fout);

	size_t nRead = (size_t)0;
	size_t byteCount = (size_t)0;
	int index;
	size_t pc = 0x8000U;

	while (nRead += fread(tmpBuf + nRead, 1, MAXBUF_HALF_LENGTH - nRead, fp))
	{
		// memset(tmpBuf + MAXBUF_HALF_LENGTH, 0, MAXBUF_HALF_LENGTH);

		byteCount = disassembler(tmpBuf, pc, fout);
		newLine(fout);
		pc += byteCount;

		for (index = byteCount; index < nRead; ++index)
			tmpBuf[index - byteCount] = tmpBuf[index];
		
		nRead = nRead > byteCount ? nRead - byteCount : 0;
	}

	fclose(fp);
	return 0;
#undef MAXBUF_HALF_LENGTH
#undef MAXBUF_LENGTH
}

int main(int argc, char *argv[])
{
	// if (!initFileName(argv[0])) { fprintf(stderr, "init fileName failed"); return 1; }

	int index;
	int errCode;
	for (index = 1; index < argc; ++index)
	{
		errCode = loadFile(argv[index], stdout);
		if (errCode)
		{
			fprintf(
				stderr,
				"ERROR: load <%s> failed, error code <%d>",
				argv[index],
				errCode
			);
			return errCode;
		}
	}
	system("pause");
	return 0;
}
