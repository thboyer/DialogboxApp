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
    CTRL_IFLAG_CARET_ON         = 0x08000000,
    CTRL_IFLAG_CANFOCUSE        = 0x04000000,
    CTRL_IFLAG_DRAW_BY_END      = 0x08000000,
    /*---------------------------------------------------*/
    CTRL_DLG_CAPSLOCKED         = 0x00010000,
    CTRL_DLG_NUMLOCKED          = 0x00020000,
    CTRL_DLG_INSERTLOCKED       = 0x00040000,
    CTRL_DLG_LEFTSHIFT          = 0x00080000,
    CTRL_DLG_RIGHTSHIFT         = 0x00100000,
};


struct s_lineEdit {
	__CONTROL_COMMON_FIELDS__
    /*-------------------------------------------*/
    int     m_iLineIndex;
    int     m_iLineOffset;
};




static int _LineEditTextWidth(char*pText, int iOffset, int iNbChar, TTF_Font*pFont){
    char pSubText[iNbChar+1];
    strncpy(pSubText, pText+iOffset, iNbChar);
    pSubText[iNbChar]='\0';
    int w;
    TTF_SizeText(pFont, pSubText, &w, NULL);
    return w;
}

static t_lineEdit*_LineEditAdjustRightAlignment(t_lineEdit*pLineEdit){
    int subStringLen=pLineEdit->m_iLineIndex-pLineEdit->m_iLineOffset;
    if(subStringLen){
        char subString[subStringLen+1];
        strncpy(subString, pLineEdit->m_pTitle+pLineEdit->m_iLineOffset, subStringLen);
        subString[subStringLen]='\0';
        TTF_SizeText(pLineEdit->m_pFont, subString, &subStringLen, NULL);
        if(subStringLen>InflatRect(&pLineEdit->m_frame, __CONTROL_INFLAT_PADDING)->w){
            pLineEdit->m_iLineOffset++;
            mBitsSet(pLineEdit->m_iStatus, CTRL_IFLAG_DRAW_BY_END);
        }
    }
    return pLineEdit;
}




t_lineEdit*LineEditNew(
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
    t_lineEdit*pLineEdit=(t_lineEdit*)ControlNew(
        controlID,
        controlFlags|CTRL_IFLAG_CANFOCUSE,
        title,
        iLeft,
        iTop,
        iWidth,
        iHeight,
        colorBkgnd,
        colorText,
        pFont,
        (t_ptfCtrlDel)NULL,
        (t_ptfCtrlDraw)LineEditDraw,
        (t_ptfCtrlDoEvent)LineEditDoEvent);
    pLineEdit=(t_lineEdit*)realloc(pLineEdit, sizeof(t_lineEdit));
    pLineEdit->m_iLineIndex=pLineEdit->m_iLineOffset=0;
    return pLineEdit;
}

t_lineEdit*LineEditDel(t_lineEdit*pLineEdit){
    return (t_lineEdit*)ControlDel((t_control*)pLineEdit);
}

const t_lineEdit*LineEditDraw(const t_lineEdit*pLineEdit, SDL_Renderer*pRenderer){
    assert(pLineEdit);
    if(mIsBitsClr(pLineEdit->m_iStatus, CTRL_FLAG_SHOWN)) return pLineEdit;
    
    SDL_SetRenderDrawColor(pRenderer, pLineEdit->m_colorBkgnd.r, pLineEdit->m_colorBkgnd.g, pLineEdit->m_colorBkgnd.b, pLineEdit->m_colorBkgnd.a);
    SDL_RenderFillRect(pRenderer, &pLineEdit->m_frame);

    if(pLineEdit->m_iTitleLength){

        
        /*$$$$$$$$*/
        SDL_Surface*pSurf=TTF_RenderText_Blended(pLineEdit->m_pFont,     pLineEdit->m_pTitle+pLineEdit->m_iLineOffset,   pLineEdit->m_colorText);
        /*$$$$$$$$*/
        

        
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
        SDL_FreeSurface(pSurf);
        SDL_DestroyTexture(pText);
    }

    /****************************PROCESSING THE CARET***************************************************/
    if(mIsBitsSet(pLineEdit->m_iStatus, CTRL_FLAG_FOCUSED | CTRL_IFLAG_CARET_ON)){
        int x=pLineEdit->m_frame.x+(-__CONTROL_INFLAT_PADDING);
        if(pLineEdit->m_iLineIndex){
            x+=_LineEditTextWidth(pLineEdit->m_pTitle, 0, pLineEdit->m_iLineIndex, pLineEdit->m_pFont);
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
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)&&(pLineEdit->m_iLineIndex<pLineEdit->m_iTitleLength)){ 
                pLineEdit->m_iLineIndex++;
                _LineEditAdjustRightAlignment(pLineEdit);
            }
            break;
        case 89:    /* END NUM PAD or 1 */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)) pLineEdit->m_iLineIndex=pLineEdit->m_iTitleLength;
            break;
        case 95:    /* HOME NUM PAD or 7 */
        case 74:    /* Home/Fin PC Portable */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)) pLineEdit->m_iLineIndex=0;
            break;
        default:
            return NULL;
        }
        break;
    
    default:
        return NULL;
    }
    return pLineEdit;
}

