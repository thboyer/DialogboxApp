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
    CTRL_IFLAG_DRAW_BY_END      = 0x02000000,
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
    SDL_Rect    m_frameEffective;
    char*       m_pTextString;
    int         m_iTextCapacity;
    int         m_iTextLength;
    int         m_iTextOffset;
    int         m_iCaretIndex;
};

/* char* m_pTitle acts as m_pPlaceHolder            */
#define __m_pPlaceHolder          m_pTitle
/* int   m_iTitleLength acts as m_placeHolderLength */
#define __m_placeHolderLength     m_iTitleLength

static int _LineEditTextWidth(char*pText, int iOffset, int iNbChar, TTF_Font*pFont){
    char*pSubText=(char*)malloc(iNbChar+1);
    strncpy(pSubText, pText+iOffset, iNbChar);
    pSubText[iNbChar]='\0';
    int w;
    TTF_SizeText(pFont, pSubText, &w, NULL);
    free(pSubText);
    return w;
}

static t_lineEdit*_LineEditAdjustRightAlignment(t_lineEdit*pLineEdit){
    int subStringLen=pLineEdit->m_iCaretIndex-pLineEdit->m_iTextOffset;
    if(subStringLen){
        char*subString=(char*)malloc(subStringLen+1);
        strncpy(subString, pLineEdit->m_pTextString+pLineEdit->m_iTextOffset, subStringLen);
        subString[subStringLen]='\0';
        TTF_SizeText(pLineEdit->m_pFont, subString, &subStringLen, NULL);
        if(subStringLen>pLineEdit->m_frameEffective.w){
            pLineEdit->m_iTextOffset++;
            mBitsSet(pLineEdit->m_iStatus, CTRL_IFLAG_DRAW_BY_END);
        }
        free(subString);
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
        (t_ptfCtrlDel)LineEditDel,
        (t_ptfCtrlDraw)LineEditDraw,
        (t_ptfCtrlDoEvent)LineEditDoEvent);
    pLineEdit=(t_lineEdit*)realloc(pLineEdit, sizeof(t_lineEdit));
    pLineEdit->m_frameEffective=*InflatRect(&pLineEdit->m_frame, __CONTROL_INFLAT_PADDING);
    pLineEdit->m_pTextString=(char*)malloc(pLineEdit->m_iTextCapacity=10);
    pLineEdit->m_pTextString[pLineEdit->m_iTextLength=0]='\0';
    pLineEdit->m_iCaretIndex=pLineEdit->m_iTextOffset=0;
    printf("\t\tLineEditNew()::m_iTextCapacity:%d\tm_iTextLength:%d\tm_iTextOffset:%d\tm_iCaretIndex:%d\n",
        pLineEdit->m_iTextCapacity,
        pLineEdit->m_iTextLength,
        pLineEdit->m_iTextOffset,
        pLineEdit->m_iCaretIndex);
    return pLineEdit;
}

t_lineEdit*LineEditDel(t_lineEdit*pLineEdit){
    printf("\t\tLineEditDel()::@%p\n", pLineEdit);
    free(pLineEdit->m_pTextString);
    return pLineEdit;
}

 t_lineEdit*LineEditDraw( t_lineEdit*pLineEdit, SDL_Renderer*pRenderer){
    assert(pLineEdit);
    if(mIsBitsClr(pLineEdit->m_iStatus, CTRL_FLAG_SHOWN)) return pLineEdit;

    /* Fill the line edit frame background */
    SDL_SetRenderDrawColor(
        pRenderer,
        pLineEdit->m_colorBkgnd.r,
        pLineEdit->m_colorBkgnd.g,
        pLineEdit->m_colorBkgnd.b,
        pLineEdit->m_colorBkgnd.a);
    SDL_RenderFillRect(pRenderer, &pLineEdit->m_frame);

    /* Fill the line edit text foreground */
    char*pTextToProcess=NULL;
    int iTextOffset=0;
    int iTextLength=0;
    SDL_Color colorText;

    if(pLineEdit->m_iTextLength){
        assert(pLineEdit->m_pTextString);
        pTextToProcess=pLineEdit->m_pTextString;
        iTextLength=pLineEdit->m_iTextLength;
        iTextOffset=pLineEdit->m_iTextOffset;
        colorText=pLineEdit->m_colorText;
    }
    else{
        if(pLineEdit->__m_placeHolderLength){
            assert(pLineEdit->__m_pPlaceHolder);
            pTextToProcess=pLineEdit->__m_pPlaceHolder;
            iTextLength=pLineEdit->__m_placeHolderLength;
            colorText=(SDL_Color){110,110,110,255};
        }
    }

    if(pTextToProcess){
        SDL_Surface*pSurf=TTF_RenderText_Blended(
            pLineEdit->m_pFont,
            pTextToProcess+iTextOffset,
            colorText);
        SDL_Texture*pText=SDL_CreateTextureFromSurface(pRenderer, pSurf);

        SDL_Rect rSrc={0, 0, pSurf->w, pSurf->h},
                 rDst=pLineEdit->m_frameEffective;

        if(rSrc.w>rDst.w){
            if(mIsBitsSet(pLineEdit->m_iStatus, CTRL_IFLAG_DRAW_BY_END)){
                int subStringLen=iTextLength-iTextOffset;
                char*subString=(char*)malloc(subStringLen+1);
                strncpy(subString, pTextToProcess+iTextOffset, subStringLen);
                subString[subStringLen]='\0';
                TTF_SizeText(pLineEdit->m_pFont, subString, &subStringLen, NULL);
                if(subStringLen>rDst.w){
                    rSrc.x=subStringLen-rDst.w;
                }
                free(subString);
            }
            rSrc.w=rDst.w;
        }
        else{
            rDst.w=rSrc.w;
        }
        SDL_RenderCopy(pRenderer, pText, &rSrc, &rDst);

        SDL_FreeSurface(pSurf);
        SDL_DestroyTexture(pText);
    }
    /* Processing the caret */
    if(mIsBitsSet(pLineEdit->m_iStatus, CTRL_FLAG_ENABLED|CTRL_FLAG_FOCUSED|CTRL_IFLAG_CARET_ON)){
        int subStringLen=pLineEdit->m_iCaretIndex-pLineEdit->m_iTextOffset;
        char*subString=(char*)malloc(subStringLen+1);
        strncpy(subString, pLineEdit->m_pTextString+pLineEdit->m_iTextOffset, subStringLen);
        subString[subStringLen]='\0';
        int caretPos;
        TTF_SizeText(pLineEdit->m_pFont, subString, &caretPos, NULL);
        if(caretPos>pLineEdit->m_frameEffective.w){
            caretPos=pLineEdit->m_frameEffective.w;
        }
        SDL_SetRenderDrawColor(
            pRenderer,
            pLineEdit->m_colorText.r,
            pLineEdit->m_colorText.g,
            pLineEdit->m_colorText.b,
            pLineEdit->m_colorText.a);
        SDL_RenderDrawLine(
            pRenderer,
            pLineEdit->m_frameEffective.x+caretPos,
            pLineEdit->m_frameEffective.y,
            pLineEdit->m_frameEffective.x+caretPos,
            pLineEdit->m_frameEffective.y+pLineEdit->m_frameEffective.h);
        free(subString);
    }
    if(pLineEdit->m_iTextLength)
        printf("\t\tLineEditDraw()::m_iTextCapacity:%d\tm_iTextLength:%d\tm_iTextOffset:%d\tm_iCaretIndex:%d\n",
            pLineEdit->m_iTextCapacity,
            pLineEdit->m_iTextLength,
            pLineEdit->m_iTextOffset,
            pLineEdit->m_iCaretIndex);

    return pLineEdit;
}

t_lineEdit*LineEditDoEvent(t_lineEdit*pLineEdit, SDL_Event*pEvent, int iStatus){
    assert(pLineEdit);
    switch (pEvent->type){
    case SDL_KEYDOWN:
        switch (pEvent->key.keysym.scancode){
        case SDL_SCANCODE_KP_6:    /* 94 : RIGHT ARROW NUM PAD or 6 */
            if(mIsBitsSet(iStatus, CTRL_DLG_NUMLOCKED)) break; 
            //no break;
        case SDL_SCANCODE_RIGHT:    /* 79 : RIGHT ARROW */
            if(pLineEdit->m_iCaretIndex<pLineEdit->m_iTextLength){
                pLineEdit->m_iCaretIndex++;
                _LineEditAdjustRightAlignment(pLineEdit);
            }
            break;
        case SDL_SCANCODE_KP_4:    /* 92 : LEFT ARROW NUM PAD or 4 */
            if(mIsBitsSet(iStatus, CTRL_DLG_NUMLOCKED)) break;
            //no break;
        case SDL_SCANCODE_LEFT:    /* 80 : LEFT ARROW */
            mBitsClr(pLineEdit->m_iStatus, CTRL_IFLAG_DRAW_BY_END);
            if(pLineEdit->m_iTextOffset && pLineEdit->m_iCaretIndex==pLineEdit->m_iTextOffset){
                pLineEdit->m_iTextOffset--;
                pLineEdit->m_iCaretIndex--;
            }
            else  if(pLineEdit->m_iCaretIndex) pLineEdit->m_iCaretIndex--;
            break;
        case SDL_SCANCODE_KP_1:    /* 89 : END NUM PAD or 1 */
            if(mIsBitsClr(iStatus, CTRL_DLG_NUMLOCKED)){
                while(pLineEdit->m_iCaretIndex<pLineEdit->m_iTextLength){
                    pLineEdit->m_iCaretIndex++;
                    _LineEditAdjustRightAlignment(pLineEdit);
                }
            }
            break;
        case SDL_SCANCODE_KP_7:    /* 95 : HOME NUM PAD or 7 */
            if(mIsBitsSet(iStatus, CTRL_DLG_NUMLOCKED)) break;
        case SDL_SCANCODE_HOME:    /* 74 : Home/Fin PC Portable */
            pLineEdit->m_iTextOffset=pLineEdit->m_iCaretIndex=0;
            mBitsClr(pLineEdit->m_iStatus, CTRL_IFLAG_DRAW_BY_END);
            break;
        case SDL_SCANCODE_BACKSPACE:
            if(pLineEdit->m_iCaretIndex){
                for(int k=pLineEdit->m_iCaretIndex;k<=pLineEdit->m_iTextLength;k++){
                    pLineEdit->m_pTextString[k-1]=pLineEdit->m_pTextString[k];
                }
                pLineEdit->m_iTextLength--;
                pLineEdit->m_iCaretIndex--;
                _LineEditAdjustRightAlignment(pLineEdit);
            }
            break;
        default:
            /* 4 to 56 */
            /* SDL_SCANCODE_A (4) to SDL_SCANCODE_0 (39) */
            if(
                (pEvent->key.keysym.scancode>=SDL_SCANCODE_A        )&&(pEvent->key.keysym.scancode<=SDL_SCANCODE_0         ) ||
                (pEvent->key.keysym.scancode>=SDL_SCANCODE_SPACE    )&&(pEvent->key.keysym.scancode<=SDL_SCANCODE_SLASH     ) ||
                (pEvent->key.keysym.scancode>=SDL_SCANCODE_KP_DIVIDE)&&(pEvent->key.keysym.scancode<=SDL_SCANCODE_KP_PLUS   )
            ){
                printf("key:%c\n", pEvent->key.keysym.sym);
                // pLineEdit->m_pTextString[pLineEdit->m_iTextLength]=pEvent->key.keysym.sym;
                pLineEdit->m_pTextString[pLineEdit->m_iCaretIndex]=pEvent->key.keysym.sym;
                if(pLineEdit->m_iCaretIndex==pLineEdit->m_iTextLength){
                    pLineEdit->m_iTextLength++;
                    pLineEdit->m_pTextString[pLineEdit->m_iTextLength]='\0';
                }
                pLineEdit->m_iCaretIndex++;
                if(pLineEdit->m_iCaretIndex==pLineEdit->m_iTextCapacity-1){
                    pLineEdit->m_pTextString=(char*)realloc(pLineEdit->m_pTextString, pLineEdit->m_iTextCapacity*=1.5);
                }
                _LineEditAdjustRightAlignment(pLineEdit);
                return pLineEdit;
            }
            return NULL;
        } // switch (pEvent->key.keysym.scancode)
        break; // case SDL_KEYDOWN:
    
    default:
        return NULL;
    } // switch (pEvent->type)

    return pLineEdit;
}
