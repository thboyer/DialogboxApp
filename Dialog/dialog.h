#ifndef __DIALOG_H
#define __DIALOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct s_dialog t_dialog;

t_dialog*DialogNew(char*title, int iWidth, int iHeight, int colorBkgnd, char*fontFileStr, int fontSize);
t_dialog*DialogDel(t_dialog*pDialog);
int DialogDoModal(t_dialog*pDialog);

typedef enum e_controlType{
    CTRL_TYPE_LABEL = 1,
    CTRL_TYPE_LINEEDIT,
    CTRL_TYPE_PUSHBUTTON,
    CTRL_TYPE_LISTBOX,
}t_controlType;

typedef enum e_controlFlags{
    CTRL_FLAG_ENABLED           = 0x00000001,
    CTRL_FLAG_SHOWN             = 0x00000002,
    CTRL_FLAG_READONLY          = 0x00000004,
    CTRL_FLAG_CENTERED          = 0x00000008,
    CTRL_FLAG_LEFTJUSTIFIED     = 0x00000000,
    CTRL_FLAG_RIGHTJUSTIFIED    = 0x00000010,
    CTRL_FLAG_TABSTOPPED        = 0x00000020,
    CTRL_FLAG_FOCUSED           = 0x00000040,
}t_controlFlags;

/*=================================================================================*/
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
);

char*DialogControlGetTitle(t_dialog*pDialog, int ctrlID);
char*DialogControlSetTitle(t_dialog*pDialog, int ctrlID, char*title);
int  DialogControlGetFlags(t_dialog*pDialog, int ctrlID);
int  DialogControlSetFlags(t_dialog*pDialog, int ctrlID, int flags);
int  DialogControlClrFlags(t_dialog*pDialog, int ctrlID, int flags);
int  DialogControlSetWidth(t_dialog*pDialog, int ctrlID, int width);
int  DialogControlSetHeight(t_dialog*pDialog, int ctrlID, int height);
int  DialogControlShow(t_dialog*pDialog, int ctrlID, int shown);

#ifdef __cplusplus
}
#endif

#endif //__DIALOG_H