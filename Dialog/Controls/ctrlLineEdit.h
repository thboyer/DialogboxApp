#ifndef __CTRLLINEEDIT_H
#define __CTRLLINEEDIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct s_lineEdit t_lineEdit;

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
    TTF_Font*pFont);

t_lineEdit*LineEditDel(t_lineEdit*pLineEdit);

const t_lineEdit*LineEditDraw(const t_lineEdit*pLineEdit, SDL_Renderer*pRenderer);

t_lineEdit*LineEditDoEvent(t_lineEdit*pLineEdit, SDL_Event*pEvent, int iStatus);

#ifdef __cplusplus
}
#endif

#endif // __CTRLLINEEDIT_H