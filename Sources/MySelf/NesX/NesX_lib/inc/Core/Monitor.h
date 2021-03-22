
#ifndef NESX_NESX_LIB_INC_CORE_MONITOR_H_
#define NESX_NESX_LIB_INC_CORE_MONITOR_H_

#include <SDL.h>

class CMonitor
{
public:
    CMonitor();
    ~CMonitor();
    bool Init(SDL_Window *pWindow);
    void Quit();

    void PowerUp();
    void Reset();
    void PowerDown();

public:
    void BeginDraw();
    void DEBUG_Draw(int x, int y, const SDL_Color &color);
    void DEBUG_DrawRect(const SDL_Rect& rect, const SDL_Color &color, bool fill = true);
    void Draw(int x, int y, const SDL_Color &color);
    void EndDraw();

private:
    SDL_Renderer *m_pRenderer;
    SDL_Texture *m_pTexture;
    int m_Width;
    int m_Height;

    SDL_Color *m_pTmpPixels;
    int m_TmpPitch;
};

#endif //NESX_NESX_LIB_INC_CORE_MONITOR_H_
