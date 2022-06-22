#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ctrlControl.h"

struct s_control {
	__CONTROL_COMMON_FIELDS__
};

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
    CTRL_IFLAG_CARET_ON         = 0x08000000,
    CTRL_IFLAG_CANFOCUSE        = 0x04000000,

};

t_control*ControlNew(
        int controlID,
        int controlFlags,
        const char*title,
        int iLeft,
        int iTop,
        int iWidth,
        int iHeight,
        int colorBkgnd,
        int colorText,
        TTF_Font*pFont,
        t_ptfCtrlDel        ptfCtrlDel,
        t_ptfCtrlDraw       ptfCtrlDraw,
        t_ptfCtrlDoEvent    ptfCtrlDoEvent){
    printf("\tControlNew()\n");
    t_control*pControl=(t_control*)malloc(sizeof(t_control));
    assert(pControl);
    if(title==NULL) title="";
    *pControl=(t_control){
        .m_ID               = controlID,
        .m_iStatus          = controlFlags,
        .m_iTitleLength     = strlen(title),
        .m_pTitle           = (char*)malloc(strlen(title)+1),
        .m_ptfCtrlDel       = ptfCtrlDel,
        .m_ptfCtrlDraw      = ptfCtrlDraw,
        .m_ptfCtrlDoEvent   = ptfCtrlDoEvent,
        .m_frame            = (SDL_Rect){iLeft, iTop, iWidth, iHeight},
        .m_colorBkgnd       = *(SDL_Color*)&colorBkgnd,
        .m_colorText        = *(SDL_Color*)&colorText,
        .m_pFont            = pFont,
    };
    strcpy(pControl->m_pTitle, title);
    return pControl;
}

t_control*ControlDel(t_control*pControl){
    printf("\tControlDel()\n");
    assert(pControl);
    if(pControl->m_pTitle) free(pControl->m_pTitle);
    if(pControl->m_ptfCtrlDel) pControl->m_ptfCtrlDel(pControl);
    free(pControl);
    return NULL;
}

const t_control*ControlDraw(const t_control*pControl, SDL_Renderer*pRenderer){
    assert(pControl);
    if(pControl->m_ptfCtrlDraw) pControl->m_ptfCtrlDraw(pControl, pRenderer);
    else if(pControl->m_pTitle) printf("\tControlDraw()::title: \"%s\" ::id: %d\n", pControl->m_pTitle, pControl->m_ID);
    return pControl;
}

const SDL_Rect*InflatRect(const SDL_Rect*r, int inflat){
    static SDL_Rect rf;
    rf=(SDL_Rect){
        .x = r->x-inflat,
        .y = r->y-inflat,
        .w = r->w+2*inflat,
        .h = r->h+2*inflat
    };
    return (const SDL_Rect*)&rf;
}

const t_control*ControlIsThisByID(const t_control*pControl, int ctrlID){
    assert(pControl);
    return (pControl->m_ID==ctrlID)?pControl:NULL; 
}

const t_control*ControlHasFocus(const t_control*pControl, void*pParam){
    assert(pControl);
    return mIsBitsSet(pControl->m_iStatus, CTRL_FLAG_ENABLED|CTRL_FLAG_SHOWN|CTRL_FLAG_FOCUSED)?pControl:NULL;
}

const t_control*ControlCanFocus(const t_control*pControl, void*pParam){
    assert(pControl);
    return mIsBitsSet(pControl->m_iStatus, CTRL_FLAG_ENABLED|CTRL_FLAG_SHOWN|CTRL_IFLAG_CANFOCUSE)?pControl:NULL;
}

const t_control*ControlCanTabStop(const t_control*pControl, void*pParam){
    assert(pControl);
    return mIsBitsSet(pControl->m_iStatus, CTRL_FLAG_ENABLED|CTRL_FLAG_SHOWN|CTRL_IFLAG_CANFOCUSE|CTRL_FLAG_TABSTOPPED)?pControl:NULL;
}


t_control*ControlSetFocus(t_control*pControl, void*pParam){
    assert(pControl);
    if(mIsBitsSet(pControl->m_iStatus, CTRL_IFLAG_CANFOCUSE)) mBitsSet(pControl->m_iStatus, CTRL_FLAG_FOCUSED);
    return pControl;
}

t_control*ControlClrFocus(t_control*pControl, void*pParam){
    assert(pControl);
    if(mIsBitsSet(pControl->m_iStatus, CTRL_IFLAG_CANFOCUSE)) mBitsClr(pControl->m_iStatus, CTRL_FLAG_FOCUSED);
    return pControl;
}

char*ControlGetTitle(const t_control*pControl){
    assert(pControl);
    return pControl->m_pTitle;
}

char*ControlSetTitle(t_control*pControl, char*title){
    assert(pControl);
    free(pControl->m_pTitle);
    pControl->m_pTitle=(char*)malloc(strlen(title)+1);
    strcpy(pControl->m_pTitle, title);
    return pControl->m_pTitle;
}


int  ControlGetFlags(t_control*pControl){
    assert(pControl);
    return pControl->m_iStatus;
}

int  ControlSetFlags(t_control*pControl, int flags){
    assert(pControl);
    return (pControl->m_iStatus|=flags);
}

int  ControlClrFlags(t_control*pControl, int flags){
    assert(pControl);
    return (pControl->m_iStatus&=~flags);
}

int  ControlSetWidth(t_control*pControl, int width){
    assert(pControl);
    return (pControl->m_frame.w=width);
}

int  ControlSetHeight(t_control*pControl, int height){
    assert(pControl);
    return (pControl->m_frame.h=height);
}

t_control*ControlDoCaret(t_control*pControl, void*pParam){
    assert(pControl);
    if(mIsBitsClr(pControl->m_iStatus, CTRL_FLAG_FOCUSED)) return NULL;
    mBitsTgl(pControl->m_iStatus, CTRL_IFLAG_CARET_ON);
    return pControl;
}

t_control*ControlDoEvent(t_control*pControl, SDL_Event*pEvent, int iStatus){
    if(pControl){
        if(pControl->m_ptfCtrlDoEvent) return pControl->m_ptfCtrlDoEvent(pControl, pEvent, iStatus);
    }
    return 0;
}