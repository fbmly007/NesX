
#ifndef NESX_NESX_LIB_INC_CORE_NESXFRAME_H_
#define NESX_NESX_LIB_INC_CORE_NESXFRAME_H_

#include "Common.h"
#include <SDL.h>

class CMonitor;
class CMainBoard;
class CCartridge;
class CJoystick;

class CNesXFrame
{
public:
  CNesXFrame();
  ~CNesXFrame();
  bool Init();
  void Update();
  void Quit();

public:
  bool SaveSnapshot(const char *szSnapshotFileName);
  bool LoadSnapshot(const char *szSnapshotFileName);
  bool LoadGame(const char *szFileName);
  bool ResetGame();
  void UnloadGame();
private:
  CMonitor *m_pMonitor;
  CMainBoard *m_pMainBoard;
  CCartridge *m_pCartridge;
  CJoystick  *m_pJoystick;
  SDL_Window *m_pWindow;
};

#endif //NESX_NESX_LIB_INC_CORE_NESXFRAME_H_
