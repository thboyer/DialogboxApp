#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ctrlControl.h"
#include "ctrlLineEdit.h"

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
    CRTL_IFLAG_CARET_ON         = 0x08000000,
    CRTL_IFLAG_CANFOCUSE        = 0x04000000,
    /*---------------------------------------------------*/
    CTRL_DLG_CAPSLOCKED         = 0x00010000,
    CTRL_DLG_NUMLOCKED          = 0x00020000,
    CTRL_DLG_INSERTLOCKED       = 0x00040000,
    CTRL_DLG_LEFTSHIFT          = 0x00080000,
    CTRL_DLG_RIGHTSHIFT         = 0x00100000,
};


typedef struct s_ctrlDrawParam{
    SDL_Renderer*pRenderer;
    TTF_Font*pFont;
}t_ctrlDrawParam;

struct s_lineEdit {
	__CONTROL_COMMON_FIELDS__
    /*-------------------------------------------*/
    int     m_iLineIndex;
};

t_lineEdit*LineEditNew(
    int controlID,
    int controlFlags,
    const char*title,
    int iLeft,
    int iTop,
    int iWidth,
    int iHeight,
    int colorBkgnd,
    int colorText){
    t_lineEdit*pLineEdit=(t_lineEdit*)ControlNew(
        controlID,
        controlFlags|CRTL_IFLAG_CANFOCUSE,
        title,
        iLeft,
        iTop,
        iWidth,
        iHeight,
        colorBkgnd,
        colorText,
        (t_ptfCtrlDel)NULL,
        (t_ptfCtrlDraw)LineEditDraw,
        (t_ptfCtrlDoEvent)LineEditDoEvent);
    pLineEdit=(t_lineEdit*)realloc(pLineEdit, sizeof(t_lineEdit));
    pLineEdit->m_iLineIndex=pLineEdit->m_iTitleLength;
    return pLineEdit;
}

t_lineEdit*LineEditDel(t_lineEdit*pLineEdit){
    return (t_lineEdit*)ControlDel((t_control*)pLineEdit);
}

const t_lineEdit*LineEditDraw(const t_lineEdit*pLineEdit, void*pParam){
    assert(pLineEdit);
    if(mIsBitsClr(pLineEdit->m_iStatus, CTRL_FLAG_SHOWN)) return pLineEdit;
    
    SDL_Renderer*pRenderer=((t_ctrlDrawParam*)pParam)->pRenderer;
    TTF_Font*pFont=((t_ctrlDrawParam*)pParam)->pFont;

    //if(pLineEdit->m_pTitle) printf("\tLineEditDraw()::title: \"%s\" ::id: %d\n", pLineEdit->m_pTitle, pLineEdit->m_ID);
    SDL_SetRenderDrawColor(pRenderer, pLineEdit->m_colorBkgnd.r, pLineEdit->m_colorBkgnd.g, pLineEdit->m_colorBkgnd.b, pLineEdit->m_colorBkgnd.a);
    SDL_RenderFillRect(pRenderer, &pLineEdit->m_frame);

    if(pLineEdit->m_iTitleLength==0) return pLineEdit;

    SDL_Surface*pSurf=TTF_RenderText_Blended(pFont, pLineEdit->m_pTitle, pLineEdit->m_colorText);
    SDL_Texture*pText=SDL_CreateTextureFromSurface(pRenderer, pSurf);
    SDL_Rect rSrc={0, 0, pSurf->w, pSurf->h},  rDst = *InflatRect(&pLineEdit->m_frame, __CONTROL_INFLAT_PADDING);

    /*if(mIsBitsSet(pLineEdit->m_iStatus, CTRL_FLAG_CENTERED)){
        if(pSurf->w>rDst.w){
            rSrc.x=(pSurf->w-rDst.w)/2;
            rSrc.w=rDst.w;
        }
        else{
            rDst.x=(rDst.w-pSurf->w)/2;
            rDst.w = pSurf->w;
        }
    }
    else */if(mIsBitsSet(pLineEdit->m_iStatus, CTRL_FLAG_RIGHTJUSTIFIED)){
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


    /****************************PROCESSING THE CARET***************************************************/
    if(mIsBitsSet(pLineEdit->m_iStatus, CTRL_FLAG_FOCUSED | CRTL_IFLAG_CARET_ON)){
        int x=pLineEdit->m_frame.x+(-__CONTROL_INFLAT_PADDING);
        if(pLineEdit->m_iLineIndex){
            char*pSubTitle=(char*)malloc(pLineEdit->m_iLineIndex+1);
            strncpy(pSubTitle, pLineEdit->m_pTitle, pLineEdit->m_iLineIndex);
            pSubTitle[pLineEdit->m_iLineIndex]='\0';
            SDL_FreeSurface(pSurf);
            pSurf=TTF_RenderText_Blended(pFont, pSubTitle, pLineEdit->m_colorText);
            free(pSubTitle);
            x+=pSurf->w;
        }
        SDL_SetRenderDrawColor(pRenderer, pLineEdit->m_colorText.r, pLineEdit->m_colorText.g, pLineEdit->m_colorText.b, pLineEdit->m_colorText.a);
        SDL_RenderDrawLine(
            pRenderer,
            x,
            pLineEdit->m_frame.y+(-__CONTROL_INFLAT_PADDING),
            x,
            pLineEdit->m_frame.y+(-__CONTROL_INFLAT_PADDING)+pLineEdit->m_frame.h-2*(-__CONTROL_INFLAT_PADDING)
        );
    }

    SDL_FreeSurface(pSurf);
    SDL_DestroyTexture(pText);
    return pLineEdit;
}

t_lineEdit*LineEditDoEvent(t_lineEdit*pLineEdit, SDL_Event*pEvent, int iStatus){
    assert(pLineEdit);
    switch (pEvent->type){
    case SDL_KEYDOWN:
        switch (pEvent->key.keysym.scancode){
        case 79:    /* RIGHT ARROW */
            if(pLineEdit->m_iLineIndex<pLineEdit->m_iTitleLength) pLineEdit->m_iLineIndex++;
            break;
        case 80:    /* LEFT ARROW */
            if(pLineEdit->m_iLineIndex) pLineEdit->m_iLineIndex--;
            break;
        case 92:    /* LEFT ARROW NUM PAD or 4 */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)&&(pLineEdit->m_iLineIndex)) pLineEdit->m_iLineIndex--;
            break;
        case 94:    /* RIGHT ARROW NUM PAD or 6 */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)&&(pLineEdit->m_iLineIndex<pLineEdit->m_iTitleLength)) pLineEdit->m_iLineIndex++;
            break;
        case 89:    /* END NUM PAD or 1 */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)) pLineEdit->m_iLineIndex=pLineEdit->m_iTitleLength;
            break;
        case 95:    /* HOME NUM PAD or 7 */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)) pLineEdit->m_iLineIndex=0;
            break;
        default:
            return NULL;
        }
        break;
    
    default:
        return NULL;
    }
    //return pLineEdit;
    return pLineEdit;
}

