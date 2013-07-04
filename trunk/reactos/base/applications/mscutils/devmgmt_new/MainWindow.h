#pragma once
#include "DeviceView.h"

typedef struct _MENU_HINT
{
    WORD CmdId;
    UINT HintId;
} MENU_HINT, *PMENU_HINT;

class CMainWindow
{
    CAtlString m_szMainWndClass;
    CDeviceView *m_DeviceView;
    HWND m_hMainWnd;
    HWND m_hStatusBar;
    HWND m_hToolBar;
    int m_CmdShow;

private:
    static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnCreate(HWND hwnd);
    LRESULT OnDestroy();
    LRESULT OnSize();
    LRESULT OnNotify(LPARAM lParam);
    LRESULT OnContext(LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

    BOOL CreateToolBar();
    BOOL CreateStatusBar();
    BOOL StatusBarLoadString(HWND hStatusBar, INT PartId, HINSTANCE hInstance, UINT uID);

public:
    CMainWindow(void);
    ~CMainWindow(void);

    BOOL Initialize(LPCTSTR lpCaption, int nCmdShow);
    INT Run();
    VOID Uninitialize();


};

