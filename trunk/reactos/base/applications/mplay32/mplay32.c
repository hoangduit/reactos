/*
 * PROJECT:         ReactOS Multimedia Player
 * FILE:            base\applications\mplay32\mplay32.c
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@reactos.org)
 */

#include "mplay32.h"

#define MAIN_WINDOW_HEIGHT    125
#define MAIN_WINDOW_MIN_WIDTH 250

HINSTANCE hInstance = NULL;
HWND hTrackBar = NULL;
HWND hToolBar = NULL;
HMENU hMainMenu = NULL;
TCHAR szAppTitle[256] = _T("");
TCHAR szPrevFile[MAX_PATH] = _T("\0");
WORD wDeviceId;
BOOL bIsOpened = FALSE;
BOOL bIsPaused = FALSE;
BOOL bRepeat = FALSE;
BOOL bIsSingleWindow = FALSE;
UINT MaxFilePos = 0;
RECT PrevWindowPos;


/* ToolBar Buttons */
static const TBBUTTON Buttons[] =
{   /* iBitmap,        idCommand,    fsState,         fsStyle,     bReserved[2], dwData, iString */
    {TBICON_PLAY,      IDC_PLAY,     TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
    {TBICON_STOP,      IDC_STOP,     TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
    {TBICON_EJECT,     IDC_EJECT,    TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
    {15,               0,            TBSTATE_ENABLED, BTNS_SEP,    {0}, 0, 0},
    {TBICON_BACKWARD,  IDC_BACKWARD, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
    {TBICON_SEEKBACK,  IDC_SEEKBACK, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
    {TBICON_SEEKFORW,  IDC_SEEKFORW, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0},
    {TBICON_FORWARD,   IDC_FORWARD,  TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, 0}
};

void EnableMenuItems(HWND hwnd)
{
    MCI_GENERIC_PARMS mciGeneric;
    MCI_DGV_RECT_PARMS mciVideoRect;
    MCI_DGV_WINDOW_PARMSW mciVideoWindow; 
    DWORD dwError;

    EnableMenuItem(hMainMenu, IDM_CLOSE_FILE, MF_BYCOMMAND | MF_ENABLED);

    dwError = mciSendCommand(wDeviceId, MCI_CONFIGURE, MCI_TEST, (DWORD_PTR)&mciGeneric);
    if (dwError == 0)
    {
        EnableMenuItem(hMainMenu, IDM_DEVPROPS, MF_BYCOMMAND | MF_ENABLED);
    }

    mciVideoWindow.hWnd = hwnd;

    dwError = mciSendCommand(wDeviceId, MCI_WINDOW, MCI_DGV_WINDOW_HWND | MCI_TEST, (DWORD_PTR)&mciVideoWindow); 
    if (!dwError)
    {
        dwError = mciSendCommand(wDeviceId, MCI_WHERE, MCI_DGV_WHERE_SOURCE | MCI_TEST, (DWORD_PTR)&mciVideoRect);
        if (!dwError)
        {
            EnableMenuItem(hMainMenu, IDM_SWITCHVIEW, MF_BYCOMMAND | MF_ENABLED);
        }
    }
}

void DisableMenuItems(void)
{
    EnableMenuItem(hMainMenu, IDM_CLOSE_FILE, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hMainMenu, IDM_DEVPROPS, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hMainMenu, IDM_SWITCHVIEW, MF_BYCOMMAND | MF_GRAYED);
}

void ResizeClientArea(HWND hwnd, int nWidth, int nHeight)
{
    RECT rcClientRect;
    RECT rcWindowRect;
    POINT ptDifference;

    GetClientRect(hwnd, &rcClientRect);
    GetWindowRect(hwnd, &rcWindowRect);
    ptDifference.x = (rcWindowRect.right - rcWindowRect.left) - rcClientRect.right;
    ptDifference.y = (rcWindowRect.bottom - rcWindowRect.top) - rcClientRect.bottom;
    MoveWindow(hwnd, rcWindowRect.left, rcWindowRect.top, nWidth + ptDifference.x, nHeight + ptDifference.y, TRUE);
}

static VOID
ShowLastWin32Error(HWND hwnd)
{
    LPTSTR lpMessageBuffer;
    DWORD dwError = GetLastError();

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      dwError,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&lpMessageBuffer,
                      0, NULL) != 0)
    {
        MessageBox(hwnd, lpMessageBuffer, szAppTitle, MB_OK | MB_ICONERROR);
        if (lpMessageBuffer) LocalFree(lpMessageBuffer);
    }
}

static VOID
SetImageList(HWND hwnd)
{
    HIMAGELIST hImageList;

    hImageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR24, 1, 1);
    if (!hImageList)
    {
        ShowLastWin32Error(hwnd);
        return;
    }

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_PLAYICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_STOPICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_EJECTICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_BACKWARDICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_SEEKBACKICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_SEEKFORWICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_AddMasked(hImageList,
                        LoadImage(hInstance, MAKEINTRESOURCE(IDB_FORWARDICON), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR),
                        RGB(255, 255, 255));

    ImageList_Destroy((HIMAGELIST)SendMessage(hToolBar,
                                              TB_SETIMAGELIST,
                                              0,
                                              (LPARAM)hImageList));
}

static VOID
ShowMCIError(HWND hwnd, DWORD dwError)
{
    TCHAR szErrorMessage[256];
    TCHAR szTempMessage[300];

    if (mciGetErrorString(dwError, szErrorMessage, sizeof(szErrorMessage) / sizeof(TCHAR)) == FALSE)
    {
        LoadString(hInstance, IDS_DEFAULTMCIERRMSG, szErrorMessage, sizeof(szErrorMessage) / sizeof(TCHAR));
    }

    _stprintf(szTempMessage, _T("MMSYS%lu: %s"), dwError, szErrorMessage);
    MessageBox(hwnd, szTempMessage, szAppTitle, MB_OK | MB_ICONEXCLAMATION);
}

static VOID
InitControls(HWND hwnd)
{
    INT NumButtons = sizeof(Buttons) / sizeof(Buttons[0]);

    InitCommonControls();

    /* Create trackbar */
    hTrackBar = CreateWindowEx(0,
                               TRACKBAR_CLASS,
                               NULL,
                               TBS_ENABLESELRANGE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS,
                               0,
                               0,
                               340,
                               20,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);
    if (!hTrackBar)
    {
        ShowLastWin32Error(hwnd);
        return;
    }

    /* Create toolbar */
    hToolBar = CreateWindowEx(0,
                              TOOLBARCLASSNAME,
                              NULL,
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS |
                              TBSTYLE_FLAT | CCS_BOTTOM | TBSTYLE_TOOLTIPS,
                              0,
                              40,
                              340,
                              30,
                              hwnd,
                              NULL,
                              hInstance,
                              NULL);
    if (!hToolBar)
    {
        ShowLastWin32Error(hwnd);
        return;
    }

    SetImageList(hwnd);
    SendMessage(hToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)Buttons);
}

static BOOL
IsSupportedFileExtension(LPTSTR lpFileName, LPTSTR lpDeviceName, LPDWORD dwSize)
{
    HKEY hKey;
    DWORD dwType;
    TCHAR *pathend;

    pathend = _tcsrchr(lpFileName, '.');

    if (pathend == NULL)
    {
        return FALSE;
    }

    pathend++;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\MCI Extensions"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey, pathend, NULL, &dwType, (LPBYTE)lpDeviceName, dwSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            if (dwType != REG_SZ)
            {
                return FALSE;
            }

            return TRUE;
        }

        RegCloseKey(hKey);
    }

    return FALSE;
}

static VOID
SwitchViewMode(HWND hwnd)
{
    MCIERROR mciError;
    MCI_DGV_RECT_PARMS mciVideoRect;
    MCI_DGV_WINDOW_PARMSW mciVideoWindow; 
    RECT rcToolbarRect;
    RECT rcTempRect;

    mciVideoWindow.hWnd = hwnd;

    mciError = mciSendCommand(wDeviceId, MCI_WINDOW, MCI_DGV_WINDOW_HWND | MCI_TEST, (DWORD_PTR)&mciVideoWindow); 
    if (mciError)
    {
        return;
    }

    mciError = mciSendCommand(wDeviceId, MCI_WHERE, MCI_DGV_WHERE_SOURCE | MCI_TEST, (DWORD_PTR)&mciVideoRect);
    if (mciError)
    {
        return;
    }

    if (!bIsSingleWindow)
    {
        GetWindowRect(hwnd, &PrevWindowPos);

        SetParent(hTrackBar, hToolBar);

        mciError = mciSendCommand(wDeviceId, MCI_WHERE, MCI_DGV_WHERE_SOURCE, (DWORD_PTR)&mciVideoRect);
        if (mciError)
        {
            ShowMCIError(hwnd, mciError);
            return;
        }

        GetWindowRect(hToolBar, &rcToolbarRect);         
        ResizeClientArea(hwnd, mciVideoRect.rc.right, mciVideoRect.rc.bottom + (rcToolbarRect.bottom - rcToolbarRect.top));

        mciError = mciSendCommand(wDeviceId, MCI_WINDOW, MCI_DGV_WINDOW_HWND, (DWORD_PTR)&mciVideoWindow); 
        if (mciError)
        {
            ShowMCIError(hwnd, mciError);
            return;
        }

        GetWindowRect(hToolBar, &rcTempRect);
        MoveWindow(hTrackBar, 180, 0, rcTempRect.right - rcTempRect.left - 180, 25, TRUE);

        CheckMenuItem(hMainMenu, IDM_SWITCHVIEW, MF_BYCOMMAND | MF_CHECKED);
        bIsSingleWindow = TRUE;
    }
    else
    {
        bIsSingleWindow = FALSE;
        CheckMenuItem(hMainMenu, IDM_SWITCHVIEW, MF_BYCOMMAND | MF_UNCHECKED);

        mciVideoWindow.hWnd = MCI_DGV_WINDOW_DEFAULT;
        mciError = mciSendCommand(wDeviceId, MCI_WINDOW, MCI_DGV_WINDOW_HWND, (DWORD_PTR)&mciVideoWindow);
        if (mciError)
        {
            ShowMCIError(hwnd, mciError);
            return;
        }

        SetParent(hTrackBar, hwnd);

        MoveWindow(hwnd, PrevWindowPos.left, PrevWindowPos.top, PrevWindowPos.right - PrevWindowPos.left, PrevWindowPos.bottom - PrevWindowPos.top, TRUE);
    }
}

static DWORD
CloseMciDevice(VOID)
{
    MCI_GENERIC_PARMS mciGeneric;
    DWORD dwError;

    if (bIsOpened)
    {
        dwError = mciSendCommand(wDeviceId, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&mciGeneric);
        if (dwError) return dwError;
        bIsOpened = FALSE;
    }

    DisableMenuItems();

    return 0;
}

static DWORD
OpenMciDevice(HWND hwnd, LPTSTR lpType, LPTSTR lpFileName)
{
    MCI_STATUS_PARMS mciStatus;
    MCI_OPEN_PARMS mciOpen;
    TCHAR szNewTitle[MAX_PATH];
    DWORD dwError;

    if (bIsOpened)
    {
        CloseMciDevice();
    }

    mciOpen.lpstrDeviceType = lpType;
    mciOpen.lpstrElementName = lpFileName;
    mciOpen.dwCallback = 0;
    mciOpen.wDeviceID = 0;
    mciOpen.lpstrAlias = NULL;

    dwError = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_WAIT, (DWORD_PTR)&mciOpen);
    if (dwError != 0)
    {
        return dwError;
    }

    mciStatus.dwItem = MCI_STATUS_LENGTH;

    dwError = mciSendCommand(mciOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&mciStatus);
    if (dwError != 0)
    {
        return dwError;
    }

    SendMessage(hTrackBar, TBM_SETRANGEMIN, (WPARAM) TRUE, (LPARAM) 1);
    SendMessage(hTrackBar, TBM_SETRANGEMAX, (WPARAM) TRUE, (LPARAM) mciStatus.dwReturn);
    SendMessage(hTrackBar, TBM_SETPAGESIZE, 0, 10);
    SendMessage(hTrackBar, TBM_SETLINESIZE, 0, 1);
    SendMessage(hTrackBar, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) 1);

    if (mciStatus.dwReturn < 10000)
    {
        SendMessage(hTrackBar, TBM_SETTICFREQ, (WPARAM) 100, (LPARAM) 0);
    }
    else if (mciStatus.dwReturn < 100000)
    {
        SendMessage(hTrackBar, TBM_SETTICFREQ, (WPARAM) 1000, (LPARAM) 0);
    }
    else if (mciStatus.dwReturn < 1000000)
    {
        SendMessage(hTrackBar, TBM_SETTICFREQ, (WPARAM) 10000, (LPARAM) 0);
    }
    else
    {
        SendMessage(hTrackBar, TBM_SETTICFREQ, (WPARAM) 100000, (LPARAM) 0);
    }

    _stprintf(szNewTitle, _T("%s - %s"), szAppTitle, lpFileName);
    SetWindowText(hwnd, szNewTitle);

    MaxFilePos = mciStatus.dwReturn;
    wDeviceId = mciOpen.wDeviceID;
    bIsOpened = TRUE;
    _tcscpy(szPrevFile, lpFileName);

    EnableMenuItems(hwnd);

    return 0;
}

static VOID
StopPlayback(HWND hwnd)
{
    if (bIsOpened)
    {
        SendMessage(hTrackBar, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) 1);
        KillTimer(hwnd, IDT_PLAYTIMER);

        if (bIsSingleWindow)
        {
            SwitchViewMode(hwnd);
        }

        CloseMciDevice();
    }
}

static VOID
SeekPlayback(HWND hwnd, DWORD dwNewPos)
{
    MCI_SEEK_PARMS mciSeek;
    MCI_PLAY_PARMS mciPlay;
    DWORD dwError;

    if (bIsOpened)
    {
        mciSeek.dwTo = (DWORD_PTR)dwNewPos;
        dwError = mciSendCommand(wDeviceId, MCI_SEEK, MCI_WAIT | MCI_TO, (DWORD_PTR)&mciSeek);
        if (dwError != 0)
        {
            ShowMCIError(hwnd, dwError);
        }

        mciPlay.dwCallback = (DWORD_PTR)hwnd;
        dwError = mciSendCommand(wDeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlay);
        if (dwError != 0)
        {
            ShowMCIError(hwnd, dwError);
        }
    }
}

static VOID
SeekBackPlayback(HWND hwnd)
{
    MCI_STATUS_PARMS mciStatus;
    DWORD dwNewPos;

    if (!bIsOpened) return;

    mciStatus.dwItem = MCI_STATUS_POSITION;
    mciSendCommand(wDeviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

    dwNewPos = mciStatus.dwReturn - 1;

    if((UINT)dwNewPos <= 1)
    {
        StopPlayback(hwnd);
    }
    else
    {
        SeekPlayback(hwnd, dwNewPos);
    }
}

static VOID
SeekForwPlayback(HWND hwnd)
{
    MCI_STATUS_PARMS mciStatus;
    DWORD dwNewPos;

    if (!bIsOpened) return;

    mciStatus.dwItem = MCI_STATUS_POSITION;
    mciSendCommand(wDeviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

    dwNewPos = mciStatus.dwReturn + 1;

    if((UINT)dwNewPos >= MaxFilePos)
    {
        StopPlayback(hwnd);
    }
    else
    {
        SeekPlayback(hwnd, dwNewPos);
    }
}

static VOID
PausePlayback(HWND hwnd)
{
    MCI_GENERIC_PARMS mciGeneric;
    DWORD dwError;

    if (bIsOpened)
    {
        dwError = mciSendCommand(wDeviceId, MCI_PAUSE, MCI_WAIT, (DWORD_PTR)&mciGeneric);
        if (dwError != 0)
        {
            ShowMCIError(hwnd, dwError);
        }
        bIsPaused = TRUE;
    }
}

static VOID
ResumePlayback(HWND hwnd)
{
    MCI_GENERIC_PARMS mciGeneric;
    DWORD dwError;

    if (bIsPaused)
    {
        dwError = mciSendCommand(wDeviceId, MCI_RESUME, MCI_WAIT, (DWORD_PTR)&mciGeneric);
        if (dwError != 0)
        {
            ShowMCIError(hwnd, dwError);
        }
        bIsPaused = FALSE;
    }
}

static VOID
ShowDeviceProperties(HWND hwnd)
{
    MCI_GENERIC_PARMS mciGeneric;
    DWORD dwError;

    dwError = mciSendCommand(wDeviceId, MCI_CONFIGURE, MCI_WAIT, (DWORD_PTR)&mciGeneric);
    if (dwError != 0)
    {
        ShowMCIError(hwnd, dwError);
    }
}

VOID CALLBACK
PlayTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    MCI_STATUS_PARMS mciStatus;
    MCI_PLAY_PARMS mciPlay;
    DWORD dwPos;

    if (!bIsOpened) KillTimer(hwnd, IDT_PLAYTIMER);

    mciStatus.dwItem = MCI_STATUS_POSITION;
    mciSendCommand(wDeviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);
    dwPos = mciStatus.dwReturn;

    if((UINT)dwPos >= MaxFilePos)
    {
        if (!bRepeat)
        {
            StopPlayback(hwnd);
        }
        else
        {
            mciSendCommand(wDeviceId, MCI_SEEK, MCI_WAIT | MCI_SEEK_TO_START, 0);
            mciPlay.dwCallback = (DWORD_PTR)hwnd;
            mciSendCommand(wDeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlay);
        }
    }
    else
    {
        SendMessage(hTrackBar, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) dwPos);
    }
}

static VOID
PlayFile(HWND hwnd, LPTSTR lpFileName)
{
    MCI_PLAY_PARMS mciPlay;
    TCHAR szLocalFileName[MAX_PATH];
    TCHAR szDeviceName[MAX_PATH];
    DWORD dwSize;
    MCIERROR mciError;

    if (lpFileName == NULL)
    {
        if (szPrevFile[0] == _T('\0'))
            return;

        _tcscpy(szLocalFileName, szPrevFile);
    }
    else
    {
        _tcscpy(szLocalFileName, lpFileName);
    }

    if (GetFileAttributes(szLocalFileName) == INVALID_FILE_ATTRIBUTES)
    {
        return;
    }

    dwSize = sizeof(szDeviceName) - 2;
    _tcsnset(szDeviceName, _T('\0'), dwSize / sizeof(TCHAR));

    if (!IsSupportedFileExtension(szLocalFileName, szDeviceName, &dwSize))
    {
        TCHAR szErrorMessage[256];

        LoadString(hInstance, IDS_UNKNOWNFILEEXT, szErrorMessage, sizeof(szErrorMessage) / sizeof(TCHAR));
        MessageBox(hwnd, szErrorMessage, szAppTitle, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    if (bIsOpened)
    {
        StopPlayback(hwnd);
    }

    mciError = OpenMciDevice(hwnd, szDeviceName, szLocalFileName);
    if (mciError != 0)
    {
        ShowMCIError(hwnd, mciError);
        return;
    }

    SetTimer(hwnd, IDT_PLAYTIMER, 100, (TIMERPROC) PlayTimerProc);

    mciSendCommand(wDeviceId, MCI_SEEK, MCI_WAIT | MCI_SEEK_TO_START, 0);

    mciPlay.dwCallback = (DWORD_PTR)hwnd;
    mciPlay.dwFrom = 0;
    mciPlay.dwTo = MaxFilePos;

    mciError = mciSendCommand(wDeviceId, MCI_PLAY, MCI_NOTIFY | MCI_FROM /*| MCI_TO*/, (DWORD_PTR)&mciPlay);
    if (mciError != 0)
    {
        ShowMCIError(hwnd, mciError);
    }
}

static VOID
OpenFileDialog(HWND hwnd)
{
    OPENFILENAME OpenFileName;
    TCHAR szFile[MAX_PATH + 1] = _T("\0");
    TCHAR szFilter[MAX_PATH], szCurrentDir[MAX_PATH];

    ZeroMemory(&OpenFileName, sizeof(OpenFileName));

    LoadString(hInstance, IDS_ALL_TYPES_FILTER, szFilter, sizeof(szFilter) / sizeof(TCHAR));

    if (!GetCurrentDirectory(sizeof(szCurrentDir) / sizeof(TCHAR), szCurrentDir))
    {
        _tcscpy(szCurrentDir, _T("c:\\"));
    }

    OpenFileName.lStructSize     = sizeof(OpenFileName);
    OpenFileName.hwndOwner       = hwnd;
    OpenFileName.hInstance       = hInstance;
    OpenFileName.lpstrFilter     = szFilter;
    OpenFileName.lpstrFile       = szFile;
    OpenFileName.nMaxFile        = sizeof(szFile) / sizeof((szFile)[0]);
    OpenFileName.lpstrInitialDir = szCurrentDir;
    OpenFileName.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_SHAREAWARE;
    OpenFileName.lpstrDefExt     = _T("\0");

    if (GetOpenFileName(&OpenFileName))
    {
        PlayFile(hwnd, OpenFileName.lpstrFile);
    }
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_CREATE:
        {
            InitControls(hwnd);
            hMainMenu = GetMenu(hwnd);
            break;
        }

        case WM_DROPFILES:
        {
            HDROP drophandle;
            TCHAR droppedfile[MAX_PATH];

            drophandle = (HDROP)wParam;
            DragQueryFile(drophandle, 0, droppedfile, sizeof(droppedfile) / sizeof(TCHAR));
            DragFinish(drophandle);
            PlayFile(hwnd, droppedfile);
            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;

            switch (pnmhdr->code)
            {
                case TTN_GETDISPINFO:
                {
                    LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam;
                    UINT idButton = (UINT)lpttt->hdr.idFrom;

                    switch (idButton)
                    {
                        case IDC_PLAY:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_PLAY);
                            break;
                        case IDC_STOP:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_STOP);
                            break;
                        case IDC_EJECT:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_EJECT);
                            break;
                        case IDC_BACKWARD:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_BACKWARD);
                            break;
                        case IDC_SEEKBACK:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_SEEKBACK);
                            break;
                        case IDC_SEEKFORW:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_SEEKFORW);
                            break;
                        case IDC_FORWARD:
                            lpttt->lpszText = MAKEINTRESOURCE(IDS_TOOLTIP_FORWARD);
                            break;
                    }
                    break;
                }
            }
        }
        break;

        case WM_SIZING:
        {
            LPRECT pRect = (LPRECT)lParam;

            if (!bIsSingleWindow)
            {
                if (pRect->right - pRect->left < MAIN_WINDOW_MIN_WIDTH)
                    pRect->right = pRect->left + MAIN_WINDOW_MIN_WIDTH;

                if (pRect->bottom - pRect->top != MAIN_WINDOW_HEIGHT)
                    pRect->bottom = pRect->top + MAIN_WINDOW_HEIGHT;
            }
            return TRUE;
        }

        case WM_SIZE:
        {
            RECT Rect;
            UINT Size;
            RECT ToolbarRect;
            MCI_DGV_PUT_PARMS mciPut;

            if (hToolBar && hTrackBar)
            {
                SendMessage(hToolBar, TB_AUTOSIZE, 0, 0);
                SendMessage(hToolBar, TB_GETITEMRECT, 1, (LPARAM)&Rect);

                if (!bIsSingleWindow)
                {
                    Size = GetSystemMetrics(SM_CYMENU) + Rect.bottom;
                    MoveWindow(hTrackBar, 0, 0, LOWORD(lParam), HIWORD(lParam) - Size, TRUE);
                }
                else
                {
                    MoveWindow(hTrackBar, 180, 0, LOWORD(lParam) - 180, 25, TRUE);

                    GetClientRect(hwnd, &Rect);
                    GetClientRect(hToolBar, &ToolbarRect);

                    mciPut.rc.top = 0;
                    mciPut.rc.left = 0;
                    mciPut.rc.right = Rect.right;
                    mciPut.rc.bottom = Rect.bottom - (ToolbarRect.bottom - ToolbarRect.top) - 2;

                    mciSendCommand(wDeviceId, MCI_PUT, MCI_DGV_PUT_DESTINATION | MCI_DGV_RECT | MCI_WAIT, (DWORD_PTR)&mciPut);
                }
            }
            return 0L;
        }

        case WM_HSCROLL:
        {
            if (hTrackBar == (HWND) lParam)
            {
                if (bIsOpened)
                {
                    DWORD dwNewPos = (DWORD) SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
                    SeekPlayback(hwnd, dwNewPos);
                }
                else
                {
                    SendMessage(hTrackBar, TBM_SETPOS, TRUE, 0);
                }
            }
        }
        break;

        case WM_NCLBUTTONDBLCLK:
        {
            if (wParam == HTCAPTION)
            {
                SwitchViewMode(hwnd);
            }
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_PLAY:
                {
                    if (bIsOpened)
                    {
                        if (bIsPaused)
                            ResumePlayback(hwnd);
                        else
                            PausePlayback(hwnd);
                    }
                    else
                    {
                        if (szPrevFile[0] == _T('\0'))
                            OpenFileDialog(hwnd);
                        else
                            PlayFile(hwnd, NULL);
                    }
                    break;
                }

                case IDC_STOP:
                    StopPlayback(hwnd);
                    break;

                case IDC_EJECT:
                    break;

                case IDC_BACKWARD:
                    break;

                case IDC_SEEKBACK:
                    SeekBackPlayback(hwnd);
                    break;

                case IDC_SEEKFORW:
                    SeekForwPlayback(hwnd);
                    break;

                case IDC_FORWARD:
                    break;

                case IDM_OPEN_FILE:
                    OpenFileDialog(hwnd);
                    return 0;

                case IDM_CLOSE_FILE:
                    StopPlayback(hwnd);
                    _tcscpy(szPrevFile, _T("\0"));
                    break;

                case IDM_REPEAT:
                {
                    if (!bRepeat)
                    {
                        CheckMenuItem(hMainMenu, IDM_REPEAT, MF_BYCOMMAND | MF_CHECKED);
                        bRepeat = TRUE;
                    }
                    else
                    {
                        CheckMenuItem(hMainMenu, IDM_REPEAT, MF_BYCOMMAND | MF_UNCHECKED);
                        bRepeat = FALSE;
                    }
                    break;
                }

                case IDM_SWITCHVIEW:
                    SwitchViewMode(hwnd);
                    break;

                case IDM_DEVPROPS:
                    ShowDeviceProperties(hwnd);
                    break;

                case IDM_VOLUMECTL:
                    ShellExecute(hwnd, NULL, _T("SNDVOL32.EXE"), NULL, NULL, SW_SHOWNORMAL);
                    break;

                case IDM_ABOUT:
                {
                    HICON mplayIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
                    ShellAbout(hwnd, szAppTitle, 0, mplayIcon);
                    DeleteObject(mplayIcon);
                    break;
                }

                case IDM_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    return 0;
            }
            break;
        }

        case WM_DESTROY:
            StopPlayback(hwnd);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, Message, wParam, lParam);
}

INT WINAPI
_tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASSEX WndClass = {0};
    TCHAR szClassName[] = _T("ROSMPLAY32");
    HWND hwnd;
    MSG msg;
    DWORD dwError;
    HANDLE hAccel;

    hInstance = hInst;

    LoadString(hInstance, IDS_APPTITLE, szAppTitle, sizeof(szAppTitle) / sizeof(TCHAR));

    WndClass.cbSize            = sizeof(WNDCLASSEX);
    WndClass.lpszClassName     = szClassName;
    WndClass.lpfnWndProc       = MainWndProc;
    WndClass.hInstance         = hInstance;
    WndClass.style             = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon             = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    WndClass.hCursor           = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground     = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName      = MAKEINTRESOURCE(IDR_MAINMENU);

    if (!RegisterClassEx(&WndClass))
    {
        ShowLastWin32Error(NULL);
        return 0;
    }

    hwnd = CreateWindow(szClassName,
                        szAppTitle,
                        WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_CLIPCHILDREN,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        350,
                        MAIN_WINDOW_HEIGHT,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
    if (!hwnd)
    {
        ShowLastWin32Error(NULL);
        return 0;
    }

    hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(ID_ACCELERATORS));

    DragAcceptFiles(hwnd, TRUE);

    DisableMenuItems();

    dwError = SearchPath(NULL, _T("SNDVOL32.EXE"), NULL, 0, NULL, NULL);
    if (dwError == 0)
    {
        EnableMenuItem(hMainMenu, IDM_VOLUMECTL, MF_BYCOMMAND | MF_GRAYED);
    }

    /* Show it */
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    PlayFile(hwnd, lpCmdLine);

    /* Message Loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hwnd, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyAcceleratorTable(hAccel);

    return (INT)msg.wParam;
}
