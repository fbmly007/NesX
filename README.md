# NesX

my nes project

### 环境

> 可以使用Clion或VS来进行编译和调试(可参考 https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index.php 进行不同平台SDL的配置)

#### Windows (VS)

1. 安装cmake
2. 下载SDL2 VC++开发库 (http://www.libsdl.org/download-2.0.php)
3. 将SDL2开发库解压缩到你想要放置的目录, 然后设置环境变量 `SDL2DIR` 为SDL2开发库的根目录
   > %SDL2DIR%目录下应该包含docs, include, lib等子目录, 请确保指定的目录是正确的

#### Windows (MinGW)
1. 安装 msys2 (https://www.msys2.org/) 并按照网站的 `Installation` 下的指引完成环境的初始化
2. 打开 MSYS2 MinGW 64-bit 安装SDL2库
```bash
pacman -S mingw-w64-x86_64-SDL2 --noconfirm
```
3. 安装cmake (使用MSYS Makefiles)
```bash
pacman -S mingw-w64-x86_64-cmake --noconfirm
```

> 使用Clion需要将MinGW配置到Toolchains中去(还需要gdb), 还需要将Working Directory配置为 `$PROJECT_DIR$/Resources`, 否则无法加载游戏

#### 其他平台(Linux/Unix/Mac)

> ubuntu平台可以使用如下命令安装
>
> ```shell
> sudo apt-get install libsdl2-2.0
> sudo apt-get install libsdl2-dev
> sudo apt-get install cmake
> ```

其他环境可参考 
https://lazyfoo.net/tutorials/SDL/01_hello_SDL/ 来安装SDL2

https://cmake.org/documentation/ 来安装cmake(也可以通过不同平台的包管理来安装)

### 编译

> 定位到 Sources/MySelf/NesX 目录

#### Windows (VS)

1. 使用cmake生成VS的sln工程

> 32位
```bash
cmake -S . -G "Visual Studio 15 2017" -B build -A Win32
```

> 64位
```bash
cmake -S . -G "Visual Studio 15 2017" -B build -A x64
```

2. 进入build目录打开NesX.sln直接编译即可


> 可以用`cmake --help`命令来查看可以使用哪个VS版本来替换上面 `-G` 后面的内容, 比如2019就是`Visual Studio 16 2019`, 2022就是`Visual Studio 17 2022`

> 根据32和64位来生成sln文件, 会自动指定对应的sdl库


##### 已知问题

> 因使用了第三方的APU库, 在VS中以`Debug`编译的时候, 会有卡顿现象. 需要以 `Release` 进行编译

> 如果需要用 `Debug` 来进行调试, 可以把 `MainBoard.cpp`的Run函数中的 `m_pAPU->Clock();` 注释掉

#### Windows (MinGW)

1. 使用cmake生成Makefile
```bash
cmake -S . -G "MSYS Makefiles" -B build
```

2. 进入build目录
```bash
cd build
```

3. 构建exe文件
```bash
make
```

> 单独运行程序要求 SDL2.dll, libstdc++-6.dll, libgcc_s_seh-1.dll 动态库和exe在相同目录下

> 删除build目录就可以清除cmake,make产生的所有文件

#### 其他平台

> 因为使用cmake, 所以和Windows平台类似, 只是在构建时使用如下命令

```shell
cmake -S . -G "Unix Makefiles" -B build
```
> 或使用默认的Generators

```shell
cmake -S . -B build
```

### 已测试游戏(Mapper 0~4)
1. 炸弹人
2. 火箭车
3. 敲冰块
4. 越野机车
5. 气球大战
6. 红巾特工队
7. 碰碰车
8. 超惑星战记
9. 最终幻想1 英文版
10. 塞尔达1
11. 狡猾飞天德
12. 龙战士3
13. 洛克人2
14. 银河战士
15. 光之神话
16. 恶魔城2
17. 忍者龙剑传1
18. 古巴战士
19. 雪人兄弟
20. 松鼠大作战1
21. 松鼠大作战2
22. 大金刚1
23. 冒险岛1
24. 碰碰飞车
25. 反重力战士
26. 冒险岛4
27. 忍者龙剑传2
28. 忍者龙剑传3
29. 超级马里奥兄弟2
30. 超级马里奥兄弟3
31. 赤影战士
32. 功夫猫党
33. 米老鼠3
34. 成龙之龙
35. 魂斗罗力量
36. 热血足球
37. 热血新纪录

### 相关截图

![炸弹人](http://ys-n.ysepan.com/612052426/419597419/n4856354JFTQJjjermJdf/0.png)![松鼠大作战](http://ys-n.ysepan.com/612052426/419597420/n4856354JFTQKjjermJae/1.png)![赤色要塞](http://ys-n.ysepan.com/612052427/419597421/jjermJp8735546FGXPJ92/2.png)

![冒险岛4](http://ys-n.ysepan.com/612052427/419597422/jjermJp8735546FGXPKe2/3.png)![忍者龙剑传3](http://ys-n.ysepan.com/612052428/419597423/s7527462GKWNKjjermJba/4.png)![超惑星战记](http://ys-n.ysepan.com/612052428/419597424/s7527462GKWNTjjermJb1/5.png)

### 参考资料

http://nesdev.com

https://fms.komkon.org/EMUL8/HOWTO.html

https://fms.komkon.org/EMUL8/NES.html

https://patater.com/gbaguy/nesasm.htm

https://fms.komkon.org

https://www.zophar.net

http://emuprog.free.fr (法)

https://www.emuparadise.me

https://gamefaqs.gamespot.com/nes/916386-nes/faqs/2949

https://www.cnblogs.com/memset/archive/2012/07/18/everynes_nes_specifications.html

http://nemulator.com/files/nes_emu.txt

https://medium.com/@bokuweb17/writing-an-nes-emulator-with-rust-and-webassembly-d64de101c49d

http://www.dustmop.io/blog/2015/04/28/nes-graphics-part-1 (\*)

https://github.com/AndreaOrru/LaiNES

https://github.com/aaronmell/6502Net

https://www.qmtpro.com/~nes/nintendulator/

http://www.chrismcovell.com/NESTechFAQ.html

http://content.atalasoft.com/atalasoft-blog/why-writing-an-emulator-is-fun-and-why-you-should-too

http://emulation.gametechwiki.com/index.php/Overclocking

https://codebase64.org/doku.php?id=base:6502_6510_coding

https://www.masswerk.at/6502/6502_instruction_set.html

http://www.my-testsite.co.uk/sites/cc/6502.html

http://nparker.llx.com/a2/index.html

http://nesdev.com/6502.txt

https://en.wikipedia.org/wiki/Frequency
