#ifndef __CTRLCONTROL_H
#define __CTRLCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#define __CONTROL_INFLAT_PADDING        (-2)

#define __CONTROL_COMMON_FIELDS__                       \
        int                     m_ID;                   \
        int                     m_iStatus;              \
        char*                   m_pTitle;               \
        int                     m_iTitleLength;         \
        SDL_Rect                m_frame;                \
        SDL_Color               m_colorBkgnd;           \
        SDL_Color               m_colorText;            \
        /*----------vfptrs-------------------------*/   \
        t_ptfCtrlDel            m_ptfCtrlDel;           \
        t_ptfCtrlDraw           m_ptfCtrlDraw;          \
        t_ptfCtrlDoEvent        m_ptfCtrlDoEvent;       \


typedef struct s_control t_control;

typedef       t_control*(*t_ptfCtrlDel)(t_control*);
typedef const t_control*(*t_ptfCtrlDraw)(const t_control*, SDL_Renderer*);
typedef       t_control*(*t_ptfCtrlDoEvent)(t_control*, SDL_Event*, int);

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
        t_ptfCtrlDel            ptfCtrlDel,
        t_ptfCtrlDraw           ptfCtrlDraw,
        t_ptfCtrlDoEvent        ptfCtrlDoEvent);

t_control*ControlDel(t_control*pControl);
const t_control*ControlDraw(const t_control*pControl, void*pParam);

const SDL_Rect*InflatRect(const SDL_Rect*r, int inflat);

const t_control*ControlIsThisByID(const t_control*pControl, int ctrlID);

const t_control*ControlHasFocus(const t_control*pControl, void*pParam);

const t_control*ControlCanFocus(const t_control*pControl, void*pParam);

const t_control*ControlCanTabStop(const t_control*pControl, void*pParam);

t_control*ControlSetFocus(t_control*pControl, void*pParam);

t_control*ControlClrFocus(t_control*pControl, void*pParam);



char*ControlGetTitle(const t_control*pControl);

char*ControlSetTitle(t_control*pControl, char*title);

int  ControlGetFlags(t_control*pControl);

int  ControlSetFlags(t_control*pControl, int flags);

int  ControlClrFlags(t_control*pControl, int flags);

int  ControlSetWidth(t_control*pControl, int width);

int  ControlSetHeight(t_control*pControl, int height);

t_control*ControlDoCaret(t_control*pControl, void*pParam);

t_control*ControlDoEvent(t_control*pControl, SDL_Event*pEvent, int iStatus);



#ifdef __cplusplus
}
#endif

#endif //__CTRLCONTROL_H