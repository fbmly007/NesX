
#include "Core/Monitor.h"
#include "Common.h"
#include <SDL_render.h>

CMonitor::CMonitor()
        : m_pRenderer(nullptr),
          m_pTexture(nullptr),
          m_Width(0),
          m_Height(0),
          m_pTmpPixels(nullptr),
          m_TmpPitch(0)
{
}

CMonitor::~CMonitor()
{
    Quit();
}

bool CMonitor::Init(SDL_Window *pWindow)
{
    // 创建渲染器
    m_pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!m_pRenderer)
        return false;

    m_Width = NES_SCREEN_WIDTH;
    m_Height = NES_SCREEN_HEIGHT;

    // 创建纹理
    m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                   m_Width, m_Height);

    return m_pTexture != nullptr;
}

void CMonitor::Quit()
{
    if (m_pTexture)
    {
        SDL_UnlockTexture(m_pTexture);
        SDL_DestroyTexture(m_pTexture);
        m_pTexture = nullptr;
    }

    m_pTmpPixels = nullptr;
    m_TmpPitch = 0;

    if (m_pRenderer)
    {
        SDL_DestroyRenderer(m_pRenderer);
        m_pRenderer = nullptr;
    }

    m_Width = 0;
    m_Height = 0;
}

void CMonitor::PowerUp()
{
}

void CMonitor::Reset()
{
}

void CMonitor::PowerDown()
{
}

void CMonitor::BeginDraw()
{
    SDL_LockTexture(m_pTexture, nullptr, (void **) &m_pTmpPixels, &m_TmpPitch);
}

void CMonitor::DEBUG_Draw(int x, int y, const SDL_Color &color)
{
    if (!m_pTmpPixels) return;
    if (x < 0 || x >= m_Width) return;
    if (y < 0 || y >= m_Height) return;
    auto pixels = (SDL_Color *) ((Uint8 *) m_pTmpPixels + y * m_TmpPitch);
    pixels[x] = color;
}

void CMonitor::DEBUG_DrawRect(const SDL_Rect &rect, const SDL_Color &color, bool bFill)
{
    if(bFill)
    {
        for(int y = rect.y; y < rect.y + rect.h; ++y)
        {
            for(int x = rect.x; x < rect.x + rect.w; ++x)
            {
                DEBUG_Draw(x, y, color);
            }
        }
    }
    else
    {
        for(int y = rect.y; y < rect.y + rect.h; ++y)
        {
            for(int x = rect.x; x < rect.x + rect.w; ++x)
            {
                if(y == rect.y || y == (rect.y + rect.h - 1) || x == rect.x || x == rect.x + rect.w - 1)
                    DEBUG_Draw(x, y, color);
            }
        }
    }


}

void CMonitor::Draw(int x, int y, const SDL_Color &color)
{
    if (!m_pTmpPixels) return;
    if (x < 0 || x >= m_Width) return;
    if (y < 0 || y >= m_Height) return;
    auto pixels = (SDL_Color *) ((Uint8 *) m_pTmpPixels + y * m_TmpPitch);
    pixels[x] = color;
}

void CMonitor::EndDraw()
{
    SDL_UnlockTexture(m_pTexture);
    m_pTmpPixels = nullptr;
    m_TmpPitch = 0;
    SDL_RenderCopy(m_pRenderer, m_pTexture, nullptr, nullptr);
    SDL_RenderPresent(m_pRenderer);
}

