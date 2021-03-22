
#include <stdio.h>
#include <fstream>
#include <string>

#include "iNesHeader.h"
using namespace std;

int main(void)
{
	ifstream ifs("Test.nes", ios::binary);
	iNesHeader header(ifs);

	printf("nes文件头信息>>>>\n");
	printf("PRG页个数: %u (每页16KB)\n", header.NumberOfPRG());
	printf("CHR页个数: %u (每页8KB)\n", header.NumberOfCHR());
	printf("Mapper号: %02X\n", header.Mapper());
	printf("属性: ");

	// 镜像属性
	if (header.HasVerticalMirroring())
	{
		printf("VerticalMirroring ");
	}
	else
	{
		printf("HorizontalMirroring ");
	}

	// 后背电池
	if (header.HasBettery())
	{
		printf("BetteryON  ");
	}

	// trainer
	if (header.HasTrainer())
	{
		printf("TrainerON ");
	}

	// 4屏幕显存
	if (header.HasFourScreenRam())
	{
		printf("4ScreenVRAM");
	}

	printf("\n");

	system("pause");
	return 0;
}