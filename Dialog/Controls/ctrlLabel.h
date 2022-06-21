#ifndef __CTRLLABEL_H
#define __CTRLLABEL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct s_label t_label;

t_label*LabelNew(
    int controlID,
    int controlFlags,
    const char*title,
    int iLeft,
    int iTop,
    int iWidth,
    int iHeight,
    int colorBkgnd,
    int colorText);

t_label*LabelDel(t_label*pLabel);

const t_label*LabelDraw(const t_label*pLabel, void*pParam);

#ifdef __cplusplus
}
#endif

#endif // __CTRLLABEL_H