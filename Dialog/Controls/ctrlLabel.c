#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ctrlControl.h"
#include "ctrlLabel.h"

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
};


typedef struct s_ctrlDrawParam{
    SDL_Renderer*pRenderer;
    TTF_Font*pFont;
}t_ctrlDrawParam;

struct s_label {
	__CONTROL_COMMON_FIELDS__
    /*-------------------------------------------*/
};

t_label*LabelNew(
    int controlID,
    int controlFlags,
    const char*title,
    int iLeft,
    int iTop,
    int iWidth,
    int iHeight,
    int colorBkgnd,
    int colorText){
    return (t_label*)ControlNew(
        controlID,
        controlFlags,
        title,
        iLeft,
        iTop,
        iWidth,
        iHeight,
        colorBkgnd,
        colorText,
        (t_ptfCtrlDel)NULL,
        (t_ptfCtrlDraw)LabelDraw,
        (t_ptfCtrlDoEvent)NULL);
}

t_label*LabelDel(t_label*pLabel){
    return (t_label*)ControlDel((t_control*)pLabel);
}

const t_label*LabelDraw(const t_label*pLabel, void*pParam){
    assert(pLabel);
    if(mIsBitsClr(pLabel->m_iStatus, CTRL_FLAG_SHOWN)) return pLabel;
    
    SDL_Renderer*pRenderer=((t_ctrlDrawParam*)pParam)->pRenderer;
    TTF_Font*pFont=((t_ctrlDrawParam*)pParam)->pFont;

    if(pLabel->m_pTitle) printf("\tLabelDraw()::title: \"%s\" ::id: %d\n", pLabel->m_pTitle, pLabel->m_ID);
    SDL_SetRenderDrawColor(pRenderer, pLabel->m_colorBkgnd.r, pLabel->m_colorBkgnd.g, pLabel->m_colorBkgnd.b, pLabel->m_colorBkgnd.a);
    SDL_RenderFillRect(pRenderer, &pLabel->m_frame);


    if(pLabel->m_iTitleLength==0) return pLabel;

    SDL_Surface*pSurf=TTF_RenderText_Blended(pFont, pLabel->m_pTitle, pLabel->m_colorText);
    SDL_Texture*pText=SDL_CreateTextureFromSurface(pRenderer, pSurf);
    SDL_Rect rSrc={0, 0, pSurf->w, pSurf->h},  rDst = *InflatRect(&pLabel->m_frame, __CONTROL_INFLAT_PADDING);

    if(mIsBitsSet(pLabel->m_iStatus, CTRL_FLAG_CENTERED)){
        if(pSurf->w>rDst.w){
            rSrc.x=(pSurf->w-rDst.w)/2;
            rSrc.w=rDst.w;
        }
        else{
            rDst.x=(rDst.w-pSurf->w)/2;
            rDst.w = pSurf->w;
        }
    }
    else if(mIsBitsSet(pLabel->m_iStatus, CTRL_FLAG_RIGHTJUSTIFIED)){
        if(pSurf->w>rDst.w){
            rSrc.x=(pSurf->w-rDst.w);
            rSrc.w=rDst.w;
        }
        else{
            rDst.x+=rDst.w-pSurf->w;
            rDst.w = pSurf->w;
        }
    }
    else{ // default, left justified
        if(pSurf->w>rDst.w){
            rSrc.w=rDst.w;
        }
        else{
            rDst.w = pSurf->w;
        }        
    }

    SDL_RenderCopy(pRenderer, pText, &rSrc, &rDst);

    SDL_FreeSurface(pSurf);
    SDL_DestroyTexture(pText);
    return pLabel;
}



