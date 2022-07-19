
#include "Core/NesXFrame.h"
#include "Core/Monitor.h"
#include "Units/MainBoard.h"
#include "Units/Cartridge.h"
#include "Units/Joystick.h"
#include "Snapshots/GameSnapshot.h"
#include <fstream>
using std::ofstream;
using std::ifstream;

CNesXFrame::CNesXFrame()
        : m_pMonitor(nullptr),
          m_pMainBoard(nullptr),
          m_pCartridge(nullptr),
          m_pJoystick(nullptr),
          m_pWindow(nullptr)
{
}

CNesXFrame::~CNesXFrame()
{
    Quit();
}

bool CNesXFrame::Init()
{
    // 初始化SDL子系统
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
        return false;

    // 创建窗口
    m_pWindow = SDL_CreateWindow("NesX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_pWindow)
        return false;

    // 构建Monitor
    m_pMonitor = new CMonitor;
    if (!m_pMonitor->Init(m_pWindow))
        return false;

    // 创建MainBoard
    m_pMainBoard = new CMainBoard;
    if (!m_pMainBoard->Init(m_pMonitor))
        return false;

    // 创建卡带(万能:))
    m_pCartridge = new CCartridge;
    if (!m_pCartridge->Init(m_pMainBoard))
        return false;

    // 创建手柄
    m_pJoystick = new CJoystick;
    if(!m_pJoystick->Init(m_pMainBoard))
        return false;

    // 插在主板上(这里设计成重用的!)
    m_pMainBoard->InsertCartridge(m_pCartridge);

    // 连接手柄
    m_pMainBoard->ConnectJoystick(m_pJoystick);
    return true;
}

void CNesXFrame::Update()
{
    bool isQuit = false;
    SDL_Event e;

    static bool bPause = false;
    static Uint8 bLastKeyState = 0;

    while (!isQuit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                isQuit = true;
            }
        }

        if(SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_P])
        {
            if(bLastKeyState != SDL_SCANCODE_P)
            {
                bLastKeyState = SDL_SCANCODE_P;
                bPause = !bPause;
            }
        }

        else if(SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_1])
        {
            if(bLastKeyState != SDL_SCANCODE_1)
            {
                bLastKeyState = SDL_SCANCODE_1;
                SaveSnapshot("saved.nesx");
            }
        }

        else if(SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_2])
        {
            if(bLastKeyState != SDL_SCANCODE_2)
            {
                bLastKeyState = SDL_SCANCODE_2;
                LoadSnapshot("saved.nesx");
            }
        }

        else
        {
            bLastKeyState = SDL_SCANCODE_UNKNOWN;
        }

        if(bPause) continue;

        if (m_pCartridge->IsLoaded())
        {
            m_pMainBoard->Run();
        }
    }
}

void CNesXFrame::Quit()
{
    SAFE_RELEASE(m_pMainBoard)
    SAFE_RELEASE(m_pCartridge)
    SAFE_RELEASE(m_pJoystick)
    SAFE_RELEASE(m_pMonitor)

    if (m_pWindow)
    {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }

    // 退出SDL子系统
    SDL_Quit();
}

bool CNesXFrame::SaveSnapshot(const char *szSnapshotFileName)
{
    if(!m_pCartridge->IsLoaded()) return false;

    ofstream ofs(szSnapshotFileName, std::ios::binary | std::ios::trunc);
    if(!ofs) return false;

    GameSnapshot snapshot;
    memset(&snapshot, 0, sizeof(snapshot));

    m_pCartridge->SaveSnapshot(snapshot.cartridge);
    m_pMainBoard->SaveSnapshot(snapshot.mainboard);
    m_pJoystick->SaveSnapshot(snapshot.joystick);

    ofs.write((char *)&snapshot, sizeof(snapshot));

    ofs.close();
    return true;
}

bool CNesXFrame::LoadSnapshot(const char *szSnapshotFileName)
{
    if(!m_pCartridge->IsLoaded()) return false;

    ifstream ifs(szSnapshotFileName, std::ios::binary);
    if(!ifs) return false;

    ifs.seekg(0, std::ios::end);
    size_t fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // 判断文件是否太小
    if(fileSize < sizeof(GameSnapshot))
    {
        ifs.close();
        return false;
    }

    // 强制只读sizeof(GameSnapshot)的大小
    fileSize = sizeof(GameSnapshot);

    GameSnapshot snapshot{};
    ifs.read((char *)&snapshot, fileSize);
    ifs.close();

    // 开始恢复
    if(!m_pCartridge->LoadSnapshot(snapshot.cartridge))
    {
        return false;
    }
    m_pMainBoard->LoadSnapshot(snapshot.mainboard);
    m_pJoystick->LoadSnapshot(snapshot.joystick);
    return true;
}

bool CNesXFrame::LoadGame(const char *szFileName)
{
    if(m_pCartridge->IsLoaded())
    {
        m_pMonitor->PowerDown();
        m_pMainBoard->PowerDown();
    }

    if (!m_pCartridge->Load(szFileName))
        return false;
    m_pMonitor->PowerUp();
    m_pMainBoard->PowerUp();
    return true;
}

bool CNesXFrame::ResetGame()
{
    if (!m_pCartridge->IsLoaded()) return false;
    m_pMonitor->Reset();
    m_pMainBoard->Reset();
    return true;
}

void CNesXFrame::UnloadGame()
{
    m_pMonitor->PowerDown();
    m_pMainBoard->PowerDown();
    m_pCartridge->Unload();
}
