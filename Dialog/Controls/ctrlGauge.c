#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ctrlControl.h"
#include "ctrlGauge.h"

#define mBitsSet(f,m)       ((f)|=(m))
#define mBitsClr(f,m)       ((f)&=(~(m)))
#define mBitsTgl(f,m)       ((f)^=(m))
#define mBitsMsk(f,m)       ((f)& (m))
#define mIsBitsSet(f,m)     (((f)&(m))==(m))
#define mIsBitsClr(f,m)     (((~(f))&(m))==(m))

enum e_controlFlags{
    CTRL_FLAG_ENABLED           = 0x00000001,
    CTRL_FLAG_SHOWN             = 0x00000002,
    CTRL_FLAG_READONLY          = 0x00000004,
    CTRL_FLAG_CENTERED          = 0x00000008,
    CTRL_FLAG_RIGHTJUSTIFIED    = 0x00000010,
    CTRL_FLAG_TABSTOPPED        = 0x00000020,
    CTRL_FLAG_FOCUSED           = 0x00000040,
    /*---------------------------------------------------*/
    // CTRL_IFLAG_CARET_ON         = 0x08000000,
    CTRL_IFLAG_CANFOCUSE        = 0x04000000,
    // CTRL_IFLAG_DRAW_BY_END      = 0x02000000,
    /*---------------------------------------------------*/
    // CTRL_DLG_CAPSLOCKED         = 0x00010000,
    // CTRL_DLG_NUMLOCKED          = 0x00020000,
    // CTRL_DLG_INSERTLOCKED       = 0x00040000,
    // CTRL_DLG_LEFTSHIFT          = 0x00080000,
    // CTRL_DLG_RIGHTSHIFT         = 0x00100000,
};

struct s_gauge {
	__CONTROL_COMMON_FIELDS__
    /*-------------------------------------------*/
};


t_gauge*GaugeNew(
    int controlID,
    int controlFlags,
    const char*title,
    int iLeft,  
    int iTop,
    int iWidth,
    int iHeight,
    int colorBkgnd,
    int colorText,
    TTF_Font*pFont){
    return (t_gauge*)ControlNew(
        controlID,
        controlFlags&(~CTRL_IFLAG_CANFOCUSE)&(~CTRL_FLAG_TABSTOPPED),
        title,
        iLeft,
        iTop,
        iWidth,
        iHeight,
        colorBkgnd,
        colorText,
        pFont,
        (t_ptfCtrlDel)NULL,
        (t_ptfCtrlDraw)GaugeDraw,
        (t_ptfCtrlDoEvent)NULL);
}

t_gauge*GaugeDel(t_gauge*pGauge){
    return NULL;
}

const t_gauge*GaugeDraw(const t_gauge*pGauge, SDL_Renderer*pRenderer){
    assert(pGauge);
    if(mIsBitsClr(pGauge->m_iStatus, CTRL_FLAG_SHOWN)) return pGauge;
    
    SDL_SetRenderDrawColor(pRenderer, pGauge->m_colorBkgnd.r, pGauge->m_colorBkgnd.g, pGauge->m_colorBkgnd.b, pGauge->m_colorBkgnd.a);
    SDL_RenderFillRect(pRenderer, &pGauge->m_frame);

    return pGauge;
}

t_gauge*GaugeDoEvent(t_gauge*pGauge, SDL_Event*pEvent, int iStatus){
    return NULL;
}