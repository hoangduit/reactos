#define __ROS_LONG64__

#define STANDALONE
#include <wine/test.h>

extern void func_AttachThreadInput(void);
extern void func_CreateIconFromResourceEx(void);
extern void func_DeferWindowPos(void);
extern void func_DestroyCursorIcon(void);
extern void func_DrawIconEx(void);
extern void func_desktop(void);
extern void func_GetIconInfo(void);
extern void func_GetKeyState(void);
extern void func_GetPeekMessage(void);
extern void func_GetSystemMetrics(void);
extern void func_InitializeLpkHooks(void);
extern void func_LoadImage(void);
extern void func_RealGetWindowClass(void);
extern void func_ScrollDC(void);
extern void func_ScrollWindowEx(void);
extern void func_SetActiveWindow(void);
extern void func_SetCursorPos(void);
extern void func_SystemParametersInfo(void);
extern void func_TrackMouseEvent(void);
extern void func_WndProc(void);
extern void func_wsprintf(void);

const struct test winetest_testlist[] =
{
    { "AttachThreadInput", func_AttachThreadInput },
    { "CreateIconFromResourceEx", func_CreateIconFromResourceEx },
    { "DeferWindowPos", func_DeferWindowPos },
    { "DestroyCursorIcon", func_DestroyCursorIcon },
    { "DrawIconEx", func_DrawIconEx },
    { "desktop", func_desktop },
    { "GetIconInfo", func_GetIconInfo },
    { "GetKeyState", func_GetKeyState },
    { "GetPeekMessage", func_GetPeekMessage },
    { "GetSystemMetrics", func_GetSystemMetrics },
    { "InitializeLpkHooks", func_InitializeLpkHooks },
    { "LoadImage", func_LoadImage },
    { "RealGetWindowClass", func_RealGetWindowClass },
    { "ScrollDC", func_ScrollDC },
    { "ScrollWindowEx", func_ScrollWindowEx },
    { "SetActiveWindow", func_SetActiveWindow },
    { "SetCursorPos", func_SetCursorPos },
    { "SystemParametersInfo", func_SystemParametersInfo },
    { "TrackMouseEvent", func_TrackMouseEvent },
    { "WndProc", func_WndProc },
    { "wsprintfApi", func_wsprintf },
    { 0, 0 }
};

