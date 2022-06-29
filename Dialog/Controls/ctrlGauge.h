#ifndef __CTRLGAUGE_H
#define __CTRLGAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct s_gauge t_gauge;

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
    TTF_Font*pFont);

t_gauge*GaugeDel(t_gauge*pGauge);

const t_gauge*GaugeDraw(const t_gauge*pGauge, SDL_Renderer*pRenderer);

t_gauge*GaugeDoEvent(t_gauge*pGauge, SDL_Event*pEvent, int iStatus);

#ifdef __cplusplus
}
#endif

#endif // __CTRLGAUGE_H