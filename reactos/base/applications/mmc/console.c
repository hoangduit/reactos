/*
 * ReactOS Management Console
 * Copyright (C) 2006 - 2007 Thomas Weidenmueller
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "precomp.h"

static const TCHAR szMMCMainFrame[] = TEXT("MMCMainFrame");
static const TCHAR szMMCChildFrm[] = TEXT("MMCChildFrm");

static ULONG NewConsoleCount = 0;

static LPTSTR
CreateNewConsoleTitle(VOID)
{
    LPTSTR lpTitle;

    if (LoadAndFormatString(hAppInstance,
                            IDS_CONSOLETITLE,
                            &lpTitle,
                            ++NewConsoleCount) == 0)
    {
        lpTitle = NULL;
    }

    return lpTitle;
}


typedef struct _CONSOLE_MAINFRAME_WND
{
    HWND hwnd;
    HWND hwndMDIClient;

    LPCTSTR lpConsoleTitle;
    HMENU hMenuConsoleSmall;
    HMENU hMenuConsoleLarge;
    INT nConsoleCount;
    union
    {
        DWORD Flags;
        struct
        {
            DWORD AppAuthorMode : 1;
        };
    };
} CONSOLE_MAINFRAME_WND, *PCONSOLE_MAINFRAME_WND;


HWND
CreateNewMDIChild(PCONSOLE_MAINFRAME_WND Info,
                  HWND hwndMDIClient)
{
    MDICREATESTRUCT mcs;
    HWND hChild;

    mcs.szTitle = CreateNewConsoleTitle();
    mcs.szClass = szMMCChildFrm;
    mcs.hOwner  = GetModuleHandle(NULL);
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
    mcs.style = MDIS_ALLCHILDSTYLES;

    hChild = (HWND)SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LONG)&mcs);
    if (hChild)
    {
        Info->nConsoleCount++;
    }

    return hChild;
}


static LRESULT
FrameOnCreate(HWND hwnd,
              LPARAM lParam)
{
    PCONSOLE_MAINFRAME_WND Info;
    CLIENTCREATESTRUCT ccs;
    LPCTSTR lpFileName = (LPCTSTR)(((LPCREATESTRUCT)lParam)->lpCreateParams);

    Info = HeapAlloc(hAppHeap,
                     0,
                     sizeof(CONSOLE_MAINFRAME_WND));
    if (Info == NULL)
        return -1;

    ZeroMemory(Info,
               sizeof(CONSOLE_MAINFRAME_WND));

    Info->hwnd = hwnd;

    SetWindowLongPtr(hwnd,
                     0,
                     (LONG_PTR)Info);

    Info->hMenuConsoleSmall = LoadMenu(hAppInstance,
                                       MAKEINTRESOURCE(IDM_CONSOLE_SMALL));

    Info->hMenuConsoleLarge = LoadMenu(hAppInstance,
                                       MAKEINTRESOURCE(IDM_CONSOLE_LARGE));

    if (lpFileName == NULL)
    {
        /* FIXME */
        Info->AppAuthorMode = TRUE;
        Info->lpConsoleTitle = TEXT("ReactOS Management Console");
    }
    else
    {
        Info->AppAuthorMode = TRUE;
        Info->lpConsoleTitle = CreateNewConsoleTitle();
    }

    SetMenu(Info->hwnd,
            Info->hMenuConsoleSmall);

    SetWindowText(Info->hwnd,
                  Info->lpConsoleTitle);

    ccs.hWindowMenu = GetSubMenu(Info->hMenuConsoleLarge, 1);
    ccs.idFirstChild = IDM_MDI_FIRSTCHILD;

    /* Create the MDI client window */
    hwndMDIClient = CreateWindowEx(WS_EX_CLIENTEDGE,
                                   L"MDICLIENT",
                                   (LPCTSTR)NULL,
                                   WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | WS_BORDER,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   hwnd,
                                   (HMENU)0xCAC,
                                   hAppInstance,
                                   (LPVOID)&ccs);

    ShowWindow(Info->hwndMDIClient, SW_SHOW);

    return 0;
}


static VOID
FrameOnCommand(HWND hwnd,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam)
{
    PCONSOLE_MAINFRAME_WND Info;
    HWND hChild;

    Info = (PCONSOLE_MAINFRAME_WND)GetWindowLongPtr(hwnd, 0);

    switch (LOWORD(wParam))
    {
        case IDM_FILE_NEW:
            CreateNewMDIChild(Info, hwndMDIClient);
            SetMenu(Info->hwnd,
                    Info->hMenuConsoleLarge);
            break;

        case IDM_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        default:
            if (LOWORD(wParam) >= IDM_MDI_FIRSTCHILD)
            {
                DefFrameProc(hwnd, hwndMDIClient, uMsg, wParam, lParam);
            }
            else
            {
                hChild = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0);
                if (hChild)
                {
                    SendMessage(hChild, WM_COMMAND, wParam, lParam);
                }
            }
            break;
    }
}


static VOID
FrameOnSize(HWND hMainWnd,
            WORD cx,
            WORD cy)
{
    RECT rcClient; //, rcTool, rcStatus;
//    int lvHeight, iToolHeight, iStatusHeight;

    /* Size toolbar and get height */
//    SendMessage(Info->hTool, TB_AUTOSIZE, 0, 0);
//    GetWindowRect(Info->hTool, &rcTool);
//    iToolHeight = rcTool.bottom - rcTool.top;

    /* Size status bar and get height */
//    SendMessage(Info->hStatus, WM_SIZE, 0, 0);
//    GetWindowRect(Info->hStatus, &rcStatus);
//    iStatusHeight = rcStatus.bottom - rcStatus.top;

    /* Calculate remaining height and size list view */
    GetClientRect(hMainWnd, &rcClient);
//    lvHeight = rcClient.bottom - iToolHeight - iStatusHeight;
    SetWindowPos(hwndMDIClient, //Info->hTreeView,
                 NULL,
                 0,
                 0, //iToolHeight,
                 rcClient.right,
                 rcClient.bottom, //lvHeight,
                 SWP_NOZORDER);
}


static LRESULT CALLBACK
ConsoleMainFrameWndProc(IN HWND hwnd,
                        IN UINT uMsg,
                        IN WPARAM wParam,
                        IN LPARAM lParam)
{
    PCONSOLE_MAINFRAME_WND Info;

    Info = (PCONSOLE_MAINFRAME_WND)GetWindowLongPtr(hwnd,
                                                    0);

    switch (uMsg)
    {
        case WM_CREATE:
            return FrameOnCreate(hwnd,
                                 lParam);

        case WM_COMMAND:
            FrameOnCommand(hwnd,
                           uMsg,
                           wParam,
                           lParam);
            break;

        case WM_SIZE:
            FrameOnSize(hwnd,
                        LOWORD(lParam),
                        HIWORD(lParam));
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            if (Info != NULL)
            {
                SetMenu(Info->hwnd,
                        NULL);

                if (Info->hMenuConsoleSmall != NULL)
                {
                    DestroyMenu(Info->hMenuConsoleSmall);
                    Info->hMenuConsoleSmall = NULL;
                }

                if (Info->hMenuConsoleLarge != NULL)
                {
                    DestroyMenu(Info->hMenuConsoleLarge);
                    Info->hMenuConsoleLarge = NULL;
                }

                HeapFree(hAppHeap,
                         0,
                         Info);
            }

            PostQuitMessage(0);
            break;

        case WM_USER_CLOSE_CHILD:
            Info->nConsoleCount--;
            if (Info->nConsoleCount == 0)
            {
                SetMenu(Info->hwnd,
                        Info->hMenuConsoleSmall);
            }
            break;

        default:
            return DefFrameProc(hwnd,
                                hwndMDIClient,
                                uMsg,
                                wParam,
                                lParam);
    }

    return 0;
}


typedef struct _CONSOLE_CHILDFRM_WND
{
    HWND hwnd;
    PCONSOLE_MAINFRAME_WND MainFrame;
} CONSOLE_CHILDFRM_WND, *PCONSOLE_CHILDFRM_WND;

static LRESULT CALLBACK
ConsoleChildFrmProc(IN HWND hwnd,
                    IN UINT uMsg,
                    IN WPARAM wParam,
                    IN LPARAM lParam)
{
    PCONSOLE_CHILDFRM_WND Info;

    Info = (PCONSOLE_CHILDFRM_WND)GetWindowLongPtr(hwnd,
                                                   0);

    switch (uMsg)
    {
        case WM_CREATE:
            Info = HeapAlloc(hAppHeap,
                             0,
                             sizeof(CONSOLE_CHILDFRM_WND));
            if (Info != NULL)
            {
                ZeroMemory(Info,
                           sizeof(CONSOLE_CHILDFRM_WND));

                Info->hwnd = hwnd;

                SetWindowLongPtr(hwnd,
                                 0,
                                 (LONG_PTR)Info);
            }
            break;

       case WM_DESTROY:
           if (Info != NULL)
                HeapFree(hAppHeap, 0, Info);

           PostMessage(hwndMainConsole, WM_USER_CLOSE_CHILD, 0, 0);
           break;

       default:
            return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


BOOL
RegisterMMCWndClasses(VOID)
{
    WNDCLASSEX wc = {0};
    BOOL Ret;

    /* Register the MMCMainFrame window class */
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = ConsoleMainFrameWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(PCONSOLE_MAINFRAME_WND);
    wc.hInstance = hAppInstance;
    wc.hIcon = LoadIcon(hAppInstance,
                        MAKEINTRESOURCE(IDI_MAINAPP));
    wc.hCursor = LoadCursor(NULL,
                            MAKEINTRESOURCE(IDC_ARROW));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szMMCMainFrame;

    Ret = (RegisterClassEx(&wc) != (ATOM)0);

    if (Ret)
    {
        /* Register the MMCChildFrm window class */
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ConsoleChildFrmProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(PCONSOLE_CHILDFRM_WND);
        wc.hInstance = hAppInstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = szMMCChildFrm;
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        Ret = (RegisterClassEx(&wc) != (ATOM)0);
        if (!Ret)
        {
            UnregisterClass(szMMCMainFrame,
                            hAppInstance);
        }
    }

    return Ret;
}

VOID
UnregisterMMCWndClasses(VOID)
{
    UnregisterClass(szMMCChildFrm,
                    hAppInstance);
    UnregisterClass(szMMCMainFrame,
                    hAppInstance);
}

HWND
CreateConsoleWindow(IN LPCTSTR lpFileName  OPTIONAL,
                    int nCmdShow)
{
    HWND hWndConsole;
    LONG_PTR FileName = (LONG_PTR)lpFileName;

    hWndConsole = CreateWindowEx(WS_EX_WINDOWEDGE,
                                 szMMCMainFrame,
                                 NULL,
                                 WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 NULL,
                                 NULL,
                                 hAppInstance,
                                 (PVOID)FileName);

    if (hWndConsole != NULL)
    {
        ShowWindow(hWndConsole, nCmdShow);
    }

    return hWndConsole;
}
