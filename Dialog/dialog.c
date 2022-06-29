#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "./Container/container.h"
#include "./Controls/ctrlControl.h"
#include "./Controls/ctrlLabel.h"
#include "./Controls/ctrlLineEdit.h"
#include "./Controls/ctrlGauge.h"
#include "dialog.h"

#define mBitsSet(f,m)       ((f)|=(m))
#define mBitsClr(f,m)       ((f)&=(~(m)))
#define mBitsTgl(f,m)       ((f)^=(m))
#define mBitsMsk(f,m)       ((f)& (m))
#define mIsBitsSet(f,m)     (((f)&(m))==(m))
#define mIsBitsClr(f,m)     (((~(f))&(m))==(m))


enum e_userEvents{
    USER_EVENT_BASE = SDL_USEREVENT,
    USER_EVENT_REFRESH,
};

#define mDialogRefresh()    SDL_PushEvent(&(SDL_Event){.type=USER_EVENT_REFRESH})



typedef struct s_ctrlDrawParam{
    SDL_Renderer*pRenderer;
    TTF_Font*pFont;
}t_ctrlDrawParam;


typedef enum e_statusMasks{
    ST_ALL_CLEARED          = 0x00000000,
    ST_DLG_INIT_FAILED      = 0x80000000,
    ST_DLG_SDL_INITIATED    = 0x40000000,
    ST_DLG_TTF_INITIATED    = 0x20000000,
    ST_DLG_CAPSLOCKED       = 0x00010000,
    ST_DLG_NUMLOCKED        = 0x00020000,
    ST_DLG_INSERTLOCKED     = 0x00040000,
    ST_DLG_LEFTSHIFT        = 0x00080000,
    ST_DLG_RIGHTSHIFT       = 0x00100000,
    ST_DLG_CTRL_STATUS_MASK = ST_DLG_CAPSLOCKED|ST_DLG_NUMLOCKED|ST_DLG_INSERTLOCKED|ST_DLG_LEFTSHIFT|ST_DLG_RIGHTSHIFT,
}t_status;

struct s_dialog{
    t_status        m_uStatus;
    SDL_Window  *   m_pWindow;
    SDL_Renderer*   m_pRenderer;
    char*           m_pTitle;
    int             m_iWidth;
    int             m_iHeight;
    SDL_Color       m_colorBkgnd;

    TTF_Font    *   m_pFont;

    t_container*    m_pControls;
    SDL_TimerID     m_timerID;
};

Uint32 _DialogTimerCallbackDoCaret(Uint32 interval, t_dialog*pDialog){
    t_control*pFocusedCtrl=ContainerParse(pDialog->m_pControls, (t_ptfVV)ControlDoCaret, NULL);
    if(pFocusedCtrl) {
        ControlDraw(pFocusedCtrl, pDialog->m_pRenderer);
        SDL_RenderPresent(pDialog->m_pRenderer);
    }
    return interval;
}


t_dialog*DialogNew(char*title, int iWidth, int iHeight, int colorBkgnd, char*fontFileStr, int fontSize){
    printf("DialogNew()\n");

    t_dialog*pDialog=(t_dialog*)malloc(sizeof(t_dialog));
    assert(pDialog);
    *pDialog=(t_dialog){
        .m_uStatus      = ST_DLG_CAPSLOCKED|ST_DLG_INSERTLOCKED,
        .m_iWidth       = iWidth,
        .m_iHeight      = iHeight,
        .m_colorBkgnd   = *(SDL_Color*)&colorBkgnd,
        .m_pTitle       = (char*)malloc(strlen(title)+1),
        .m_pControls    = ContainerNew((t_ptfV)ControlDel),
    };
    strcpy(pDialog->m_pTitle, title);

    if(SDL_Init(SDL_INIT_EVERYTHING)!=0){
        fprintf(stderr,"Failed to initiate SDL: %s\n", SDL_GetError());
        mBitsSet(pDialog->m_uStatus, ST_DLG_INIT_FAILED);
        return pDialog;
    }
    mBitsSet(pDialog->m_uStatus, ST_DLG_SDL_INITIATED);

    if((pDialog->m_pWindow=SDL_CreateWindow(
                                title,
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                iWidth,
                                iHeight,
                                SDL_WINDOW_SHOWN))==NULL){

        fprintf(stderr,"Failed to create window: %s\n", SDL_GetError());
        mBitsSet(pDialog->m_uStatus, ST_DLG_INIT_FAILED);
        return pDialog;
    }

    if((pDialog->m_pRenderer=SDL_CreateRenderer(
                                    pDialog->m_pWindow,
                                    -1,
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
                                    ))==NULL){

        fprintf(stderr,"Failed to create accelerated renderer: %s\n", SDL_GetError());

        if((pDialog->m_pRenderer=SDL_CreateRenderer(
                                        pDialog->m_pWindow,
                                        -1,
                                        SDL_RENDERER_SOFTWARE
                                        ))==NULL){

            fprintf(stderr,"Failed to create software renderer: %s\n", SDL_GetError());
            mBitsSet(pDialog->m_uStatus, ST_DLG_INIT_FAILED);
            return pDialog;
        }
        fprintf(stderr,"Created software renderer instead!\n");
    }

    if(TTF_Init()!=0){
        fprintf(stderr,"Failed to initiate TTF: %s\n", SDL_GetError());
        mBitsSet(pDialog->m_uStatus, ST_DLG_INIT_FAILED);
        return pDialog;
    }
    mBitsSet(pDialog->m_uStatus,ST_DLG_TTF_INITIATED);

    if((pDialog->m_pFont=TTF_OpenFont(fontFileStr, fontSize))==NULL){
        fprintf(stderr,"Failed to open font: %s\n", SDL_GetError());
        mBitsSet(pDialog->m_uStatus, ST_DLG_INIT_FAILED);
        return pDialog;
    }
    return pDialog;    
}

t_dialog*DialogDel(t_dialog*pDialog){
    printf("DialogDel()\n");
    assert(pDialog);

    if(pDialog->m_timerID) SDL_RemoveTimer(pDialog->m_timerID);
    if(pDialog->m_pControls) ContainerDel(pDialog->m_pControls);

    if(pDialog->m_pTitle)    free(pDialog->m_pTitle);
    if(pDialog->m_pRenderer) SDL_DestroyRenderer(pDialog->m_pRenderer);
    if(pDialog->m_pWindow)   SDL_DestroyWindow(pDialog->m_pWindow);
    if(pDialog->m_pFont)     TTF_CloseFont(pDialog->m_pFont);
    if(mIsBitsSet(pDialog->m_uStatus, ST_DLG_TTF_INITIATED)) TTF_Quit();
    // if(mIsBitsSet(pDialog->m_uStatus, ST_DLG_SDL_INITIATED)) SDL_Quit(); //*** To avoid sanitize memory leaks from SDL module ***//

    free(pDialog);
    return NULL;    
}

int DialogDoModal(t_dialog*pDialog){
    printf("DialogDoModal()\n");
    assert(pDialog);

    if(mIsBitsSet(pDialog->m_uStatus, ST_DLG_INIT_FAILED)) return EXIT_FAILURE;

    pDialog->m_timerID = SDL_AddTimer(500, (SDL_TimerCallback)_DialogTimerCallbackDoCaret, pDialog);

    int iQuit=0;
    SDL_Event event;

    int f=0;

    mDialogRefresh();

    int cpt=0;

    while (!iQuit){
        while (SDL_PollEvent(&event)){
            switch (event.type){
            case SDL_QUIT:
                iQuit=1;
                break;
            case USER_EVENT_REFRESH:
                SDL_SetRenderDrawColor(pDialog->m_pRenderer, pDialog->m_colorBkgnd.r, pDialog->m_colorBkgnd.g, pDialog->m_colorBkgnd.b, pDialog->m_colorBkgnd.a);
                SDL_RenderClear(pDialog->m_pRenderer);
                if(pDialog->m_pControls)
                    ContainerParseAll(
                        pDialog->m_pControls,
                        (t_ptfVV)ControlDraw,
                        pDialog->m_pRenderer);
                SDL_RenderPresent(pDialog->m_pRenderer);               
                break;
            case SDL_KEYDOWN:
            {
                char buf[256];
                sprintf(buf,"it: %07d scan: %d sym: %d", cpt++, event.key.keysym.scancode, event.key.keysym.sym);
                SDL_SetWindowTitle(pDialog->m_pWindow, buf);
            }
                if(ControlDoEvent(ContainerParse(pDialog->m_pControls, (t_ptfVV)ControlHasFocus, NULL), &event, pDialog->m_uStatus&ST_DLG_CTRL_STATUS_MASK)==0){
                    /* DialogBox can process the pressed key *****************************************/
                    switch(event.key.keysym.scancode){
                    case 43:    /* TAB */
                        if(mIsBitsClr(pDialog->m_uStatus, ST_DLG_LEFTSHIFT)){
                            /* TAB */ /* Process tab stop on controls */
                            t_control*pCtrlFocused=(t_control*)ContainerParse(pDialog->m_pControls, (t_ptfVV)ControlHasFocus, NULL);
                            if(pCtrlFocused){
                                ControlClrFocus(pCtrlFocused, NULL);
                                pCtrlFocused=(t_control*)ContainerGetNextElemByCriteria(pDialog->m_pControls,(t_ptfVV)ControlCanTabStop, pCtrlFocused);
                                if(pCtrlFocused) ControlSetFocus(pCtrlFocused, NULL);
                                mDialogRefresh();
                            }
                        }
                        else{
                            /* LEFTSHIFT + TAB */ /* Process back tab stop on controls */
                            t_control*pCtrlFocused=(t_control*)ContainerParse(pDialog->m_pControls, (t_ptfVV)ControlHasFocus, NULL);
                            if(pCtrlFocused){
                                ControlClrFocus(pCtrlFocused, NULL);
                                pCtrlFocused=(t_control*)ContainerGetPrevElemByCriteria(pDialog->m_pControls,(t_ptfVV)ControlCanTabStop, pCtrlFocused);
                                if(pCtrlFocused) ControlSetFocus(pCtrlFocused, NULL);
                                mDialogRefresh();
                            }
                        }
                        break;
                    case 57:    /* CAPS Lock */
                        mBitsTgl(pDialog->m_uStatus, ST_DLG_CAPSLOCKED);
                        break;
                    case 83:    /* NUM Lock */
                        mBitsTgl(pDialog->m_uStatus, ST_DLG_NUMLOCKED);
                        break;
                    case 225:   /* LEFT SHIFT */
                        mBitsSet(pDialog->m_uStatus, ST_DLG_LEFTSHIFT);
                        break;
                    case 229:   /* RIGHT SHIFT */
                        mBitsSet(pDialog->m_uStatus, ST_DLG_RIGHTSHIFT);
                        break;
                    default:
                        switch (event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            event.type=SDL_QUIT;
                            SDL_PushEvent(&event);
                            break;
                        case SDLK_x:
                            // printf("%s\n", DialogControlGetTitle(pDialog, 4));
                            {
                                char*pTitle=(char*)malloc(strlen(DialogControlGetTitle(pDialog, 1))+1);
                                strcpy(pTitle, DialogControlGetTitle(pDialog, 1));
                                DialogControlSetTitle(pDialog, 1, DialogControlGetTitle(pDialog, 2));
                                DialogControlSetTitle(pDialog, 2, DialogControlGetTitle(pDialog, 3));
                                DialogControlSetTitle(pDialog, 3, pTitle);
                                free(pTitle);
                            }
                            break;
                        case SDLK_f:
                            DialogControlSetFlags(pDialog, 1, CTRL_FLAG_RIGHTJUSTIFIED);
                            DialogControlSetWidth(pDialog, 1, 400);
                            printf("H: %d\n",DialogControlSetHeight(pDialog, 3, 160));
                            DialogControlShow(pDialog, 2, f); f^=1;
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }
                else{
                    mDialogRefresh();
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.scancode){
                case 225:   /* LEFT SHIFT */
                    mBitsClr(pDialog->m_uStatus, ST_DLG_LEFTSHIFT);
                    break;
                case 229:   /* RIGHT SHIFT */
                    mBitsClr(pDialog->m_uStatus, ST_DLG_RIGHTSHIFT);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
    }
    return EXIT_SUCCESS;    
}
















/*-----------------------------------------------------------------------------------*/

void DialogAddNewControl(
    t_dialog*pDialog,
    t_controlType ctrlType,
    t_controlFlags ctrlFlags,
    int ctrlID,
    int iLeft,
    int iTop,
    int iWidth,
    int iHeight,
    int colorBkgnd,
    int colorText,
    const char*titleStr
){
    assert(pDialog);
    switch (ctrlType){
    case CTRL_TYPE_LABEL:
        ContainerPushback(
            pDialog->m_pControls,
            LabelNew(
                ctrlID,
                ctrlFlags,
                titleStr,
                iLeft,
                iTop,
                iWidth,
                iHeight, 
                colorBkgnd,
                colorText,
                pDialog->m_pFont));
        break;
    case CTRL_TYPE_LINEEDIT:
        ContainerPushback(
            pDialog->m_pControls,
            LineEditNew(
                ctrlID,
                ctrlFlags,
                titleStr,
                iLeft,
                iTop,
                iWidth,
                iHeight, 
                colorBkgnd,
                colorText,
                pDialog->m_pFont));
        break;
    case CTRL_TYPE_PUSHBUTTON:
        break;
    case CTRL_TYPE_LISTBOX:
        break;
    case CTRL_TYPE_GAUGE:
        ContainerPushback(
            pDialog->m_pControls,
            GaugeNew(
                ctrlID,
                ctrlFlags,
                titleStr,
                iLeft,
                iTop,
                iWidth,
                iHeight, 
                colorBkgnd,
                colorText,
                pDialog->m_pFont));
        break;
    default:
        break;
    }
}

char*DialogControlGetTitle(t_dialog*pDialog, int ctrlID){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    return pControl?ControlGetTitle(pControl):"";
}

char*DialogControlSetTitle(t_dialog*pDialog, int ctrlID, char*title){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    if(pControl){
        mDialogRefresh();
        return ControlSetTitle(pControl, title);
    }
    else return "";
}

int  DialogControlGetFlags(t_dialog*pDialog, int ctrlID){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    if(pControl){
        return ControlGetFlags(pControl);
    }
    else return 0;
}

int  DialogControlSetFlags(t_dialog*pDialog, int ctrlID, int flags){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    if(pControl){
        mDialogRefresh();
        return ControlSetFlags(pControl, flags);
    }
    else return 0;
}

int  DialogControlSetWidth(t_dialog*pDialog, int ctrlID, int width){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    if(pControl){
        mDialogRefresh();
        return ControlSetWidth(pControl, width);
    }
    else return 0;
}

int  DialogControlSetHeight(t_dialog*pDialog, int ctrlID, int height){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    if(pControl){
        mDialogRefresh();
        return ControlSetHeight(pControl, height);
    }
    else return 0;
}

int  DialogControlShow(t_dialog*pDialog, int ctrlID, int shown){
    assert(pDialog);
    t_control*pControl=ContainerParse(pDialog->m_pControls,(t_ptfVV)ControlIsThisByID,(void*)(long)ctrlID);
    if(pControl){
        mDialogRefresh();
        return shown?ControlSetFlags(pControl, CTRL_FLAG_SHOWN):ControlClrFlags(pControl, CTRL_FLAG_SHOWN);
    }
    else return 0;
}
