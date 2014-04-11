/*
 * PROJECT:     ReactOS system properties, control panel applet
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        dll/cpl/sysdm/virtual.c
 * PURPOSE:     Virtual memory control dialog
 * COPYRIGHT:   Copyright 2006 Ged Murphy <gedmurphy@gmail.com>
 *
 */

#include "precomp.h"

static BOOL OnSelChange(HWND hwndDlg, PVIRTMEM pVirtMem);
static LPCTSTR lpKey = _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management");

static BOOL
ReadPageFileSettings(PVIRTMEM pVirtMem)
{
    HKEY hkey = NULL;
    DWORD dwType;
    DWORD dwDataSize;
    BOOL bRet = FALSE;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       lpKey,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_QUERY_VALUE,
                       NULL,
                       &hkey,
                       NULL) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hkey,
                            _T("PagingFiles"),
                            NULL,
                            &dwType,
                            NULL,
                            &dwDataSize) == ERROR_SUCCESS)
        {
            pVirtMem->szPagingFiles = (LPTSTR)HeapAlloc(GetProcessHeap(),
                                                        0,
                                                        dwDataSize);
            if (pVirtMem->szPagingFiles != NULL)
            {
                ZeroMemory(pVirtMem->szPagingFiles,
                           dwDataSize);
                if (RegQueryValueEx(hkey,
                                    _T("PagingFiles"),
                                    NULL,
                                    &dwType,
                                    (PBYTE)pVirtMem->szPagingFiles,
                                    &dwDataSize) == ERROR_SUCCESS)
                {
                    bRet = TRUE;
                }
            }
        }
    }

    if (!bRet)
        ShowLastWin32Error(pVirtMem->hSelf);

    if (hkey != NULL)
        RegCloseKey(hkey);

    return bRet;
}


static VOID
GetPageFileSizes(LPTSTR lpPageFiles,
                 LPINT lpInitialSize,
                 LPINT lpMaximumSize)
{
    INT i = 0;

    *lpInitialSize = -1;
    *lpMaximumSize = -1;

    while (*lpPageFiles != _T('\0'))
    {
        if (*lpPageFiles == _T(' '))
        {
            lpPageFiles++;

            switch (i)
            {
                case 0:
                    *lpInitialSize = (INT)_ttoi(lpPageFiles);
                    i = 1;
                    break;

                case 1:
                    *lpMaximumSize = (INT)_ttoi(lpPageFiles);
                    return;
            }
        }

        lpPageFiles++;
    }
}


static VOID
ParseMemSettings(PVIRTMEM pVirtMem)
{
    TCHAR szDrives[1024];    // All drives
    LPTSTR DrivePtr = szDrives;
    TCHAR szDrive[3];        // Single drive
    TCHAR szVolume[MAX_PATH + 1];
    INT InitialSize;
    INT MaximumSize;
    INT DriveLen;
    INT PgCnt = 0;
    INT Len;

    DriveLen = GetLogicalDriveStrings(1023,
                                      szDrives);

    while (DriveLen != 0)
    {
        Len = lstrlen(DrivePtr) + 1;
        DriveLen -= Len;

        DrivePtr = _tcsupr(DrivePtr);

        /* Copy the 'X:' portion */
        lstrcpyn(szDrive, DrivePtr, sizeof(szDrive) / sizeof(TCHAR));

        if (GetDriveType(DrivePtr) == DRIVE_FIXED)
        {
            InitialSize = -1;
            MaximumSize = -1;

            /* Does drive match the one in the registry ? */
            if (!_tcsncmp(pVirtMem->szPagingFiles, szDrive, 2))
            {
                GetPageFileSizes(pVirtMem->szPagingFiles,
                                 &InitialSize,
                                 &MaximumSize);
            }

            pVirtMem->Pagefile[PgCnt].InitialSize = InitialSize;
            pVirtMem->Pagefile[PgCnt].MaximumSize = MaximumSize;
            pVirtMem->Pagefile[PgCnt].bUsed = TRUE;
            lstrcpy(pVirtMem->Pagefile[PgCnt].szDrive, szDrive);


            /* Get the volume label if there is one */
            if (GetVolumeInformation(DrivePtr,
                                     szVolume,
                                     MAX_PATH + 1,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     0))
            {
                pVirtMem->Pagefile[PgCnt].pszVolume = HeapAlloc(GetProcessHeap(),
                                                                0,
                                                                (_tcslen(szVolume) + 1) * sizeof(TCHAR));
                if (pVirtMem->Pagefile[PgCnt].pszVolume != NULL)
                    _tcscpy(pVirtMem->Pagefile[PgCnt].pszVolume, szVolume);
            }

            PgCnt++;
        }

        DrivePtr += Len;
    }

    pVirtMem->Count = PgCnt;
}


static VOID
WritePageFileSettings(PVIRTMEM pVirtMem)
{
    HKEY hk = NULL;
    TCHAR szPagingFiles[2048];
    TCHAR szText[256];
    INT i, nPos = 0;
    BOOL bErr = TRUE;

    for (i = 0; i < pVirtMem->Count; ++i)
    {
        if (pVirtMem->Pagefile[i].bUsed &&
            pVirtMem->Pagefile[i].InitialSize != -1 &&
            pVirtMem->Pagefile[i].MaximumSize != -1)
        {
            _stprintf(szText,
                      _T("%s\\pagefile.sys %i %i"),
                      pVirtMem->Pagefile[i].szDrive,
                      pVirtMem->Pagefile[i].InitialSize,
                      pVirtMem->Pagefile[i].MaximumSize);

            /* Add it to our overall registry string */
            lstrcpy(szPagingFiles + nPos, szText);

            /* Record the position where the next string will start */
            nPos += (INT)lstrlen(szText) + 1;

            /* Add another NULL for REG_MULTI_SZ */
            szPagingFiles[nPos] = _T('\0');
            nPos++;
        }
    }

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       lpKey,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &hk,
                       NULL) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(hk,
                          _T("PagingFiles"),
                          0,
                          REG_MULTI_SZ,
                          (LPBYTE) szPagingFiles,
                          (DWORD) nPos * sizeof(TCHAR)) == ERROR_SUCCESS)
        {
            bErr = FALSE;
        }

        RegCloseKey(hk);
    }

    if (bErr)
        ShowLastWin32Error(pVirtMem->hSelf);
}


static VOID
SetListBoxColumns(HWND hwndListBox)
{
    RECT rect = {0, 0, 103, 0};
    MapDialogRect(hwndListBox, &rect);

    SendMessage(hwndListBox, LB_SETTABSTOPS, (WPARAM)1, (LPARAM)&rect.right);
}


static VOID
OnNoPagingFile(PVIRTMEM pVirtMem)
{
    /* Disable the page file custom size boxes */
    EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_INITIALSIZE), FALSE);
    EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_MAXSIZE), FALSE);
}


static VOID
OnSysManSize(PVIRTMEM pVirtMem)
{
    /* Disable the page file custom size boxes */
    EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_INITIALSIZE), FALSE);
    EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_MAXSIZE), FALSE);
}


static VOID
OnCustom(PVIRTMEM pVirtMem)
{
    /* Enable the page file custom size boxes */
    EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_INITIALSIZE), TRUE);
    EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_MAXSIZE), TRUE);
}


static VOID
InitPagefileList(PVIRTMEM pVirtMem)
{
    TCHAR szDisplayString[256];
    TCHAR szSize[64];
    INT Index;
    INT i;

    for (i = 0; i < 26; i++)
    {
        if (pVirtMem->Pagefile[i].bUsed)
        {
            if ((pVirtMem->Pagefile[i].InitialSize == -1) &&
                (pVirtMem->Pagefile[i].MaximumSize == -1))
            {
                LoadString(hApplet,
                           IDS_PAGEFILE_NONE,
                           szSize,
                           sizeof(szSize) / sizeof(szSize[0]));
            }
            else if ((pVirtMem->Pagefile[i].InitialSize == 0) &&
                     (pVirtMem->Pagefile[i].MaximumSize == 0))
            {
                LoadString(hApplet,
                           IDS_PAGEFILE_SYSTEM,
                           szSize,
                           sizeof(szSize) / sizeof(szSize[0]));
            }
            else
            {
                _stprintf(szSize, _T("%d - %d"),
                          pVirtMem->Pagefile[i].InitialSize,
                          pVirtMem->Pagefile[i].MaximumSize);
            }

            _stprintf(szDisplayString,
                      _T("%s  [%s]\t%s"),
                      pVirtMem->Pagefile[i].szDrive,
                      pVirtMem->Pagefile[i].pszVolume ? pVirtMem->Pagefile[i].pszVolume : _T(""),
                      szSize);

            Index = SendMessage(pVirtMem->hListBox, LB_ADDSTRING, (WPARAM)0, (LPARAM)szDisplayString);
            SendMessage(pVirtMem->hListBox, LB_SETITEMDATA, Index, i);
        }
    }

    SendMessage(pVirtMem->hListBox, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);

    OnSelChange(pVirtMem->hSelf, pVirtMem);
}


static VOID
UpdatePagefileEntry(PVIRTMEM pVirtMem,
                    INT ListIndex,
                    INT DriveIndex)
{
    TCHAR szDisplayString[256];
    TCHAR szSize[64];

    if ((pVirtMem->Pagefile[DriveIndex].InitialSize == -1) &&
        (pVirtMem->Pagefile[DriveIndex].MaximumSize == -1))
    {
        LoadString(hApplet,
                   IDS_PAGEFILE_NONE,
                   szSize,
                   sizeof(szSize) / sizeof(szSize[0]));
    }
    else if ((pVirtMem->Pagefile[DriveIndex].InitialSize == 0) &&
             (pVirtMem->Pagefile[DriveIndex].MaximumSize == 0))
    {
        LoadString(hApplet,
                   IDS_PAGEFILE_SYSTEM,
                   szSize,
                   sizeof(szSize) / sizeof(szSize[0]));
    }
    else
    {
        _stprintf(szSize,
                  _T("%d - %d"),
                  pVirtMem->Pagefile[DriveIndex].InitialSize,
                  pVirtMem->Pagefile[DriveIndex].MaximumSize);
    }

    _stprintf(szDisplayString,
              _T("%s  [%s]\t%s"),
              pVirtMem->Pagefile[DriveIndex].szDrive,
              pVirtMem->Pagefile[DriveIndex].pszVolume ? pVirtMem->Pagefile[DriveIndex].pszVolume : L"",
              szSize);

    SendMessage(pVirtMem->hListBox, LB_DELETESTRING, (WPARAM)ListIndex, 0);
    SendMessage(pVirtMem->hListBox, LB_INSERTSTRING, (WPARAM)ListIndex, (LPARAM)szDisplayString);
    SendMessage(pVirtMem->hListBox, LB_SETCURSEL, (WPARAM)ListIndex, 0);
}


static VOID
OnSet(PVIRTMEM pVirtMem)
{
    INT Index;
    UINT InitialSize;
    UINT MaximumSize;
    BOOL bTranslated;
    TCHAR szTitle[64];
    TCHAR szMessage[256];
    INT DriveIndex = 0;

    pVirtMem->bSave = TRUE;

    Index  = (INT)SendDlgItemMessage(pVirtMem->hSelf,
                                     IDC_PAGEFILELIST,
                                     LB_GETCURSEL,
                                     0,
                                     0);
    if (Index >= 0 && Index < pVirtMem->Count)
    {
        DriveIndex = SendDlgItemMessage(pVirtMem->hSelf,
                                        IDC_PAGEFILELIST,
                                        LB_GETITEMDATA,
                                        0,
                                        0);

        /* Check if custom settings are checked */
        if (IsDlgButtonChecked(pVirtMem->hSelf,
                               IDC_CUSTOM) == BST_CHECKED)
        {
            InitialSize = GetDlgItemInt(pVirtMem->hSelf,
                                        IDC_INITIALSIZE,
                                        &bTranslated,
                                        FALSE);
            if (!bTranslated)
            {
                if (LoadString(hApplet,
                               IDS_MESSAGEBOXTITLE,
                               szTitle,
                               sizeof(szTitle) / sizeof(szTitle[0])) == 0)
                    _tcscpy(szTitle, _T("System control panel applet"));

                if (LoadString(hApplet,
                               IDS_WARNINITIALSIZE,
                               szMessage,
                               sizeof(szMessage) / sizeof(szMessage[0])) == 0)
                    _tcscpy(szMessage, _T("Enter a numeric value for the initial size of the paging file."));

                MessageBox(NULL,
                           szMessage,
                           szTitle,
                           MB_ICONWARNING | MB_OK);
                return;
            }

            MaximumSize = GetDlgItemInt(pVirtMem->hSelf,
                                        IDC_MAXSIZE,
                                        &bTranslated,
                                        FALSE);
            if (!bTranslated)
            {
                if (LoadString(hApplet,
                               IDS_MESSAGEBOXTITLE,
                               szTitle,
                               sizeof(szTitle) / sizeof(szTitle[0])) == 0)
                    _tcscpy(szTitle, _T("System control panel applet"));

                if (LoadString(hApplet,
                               IDS_WARNMAXIMUMSIZE,
                               szMessage,
                               sizeof(szMessage) / sizeof(szMessage[0])) == 0)
                    _tcscpy(szMessage, _T("Enter a numeric value for the maximum size of the paging file."));

                MessageBox(NULL,
                           szMessage,
                           szTitle,
                           MB_ICONWARNING | MB_OK);
                return;
            }

            /* Check the valid range of the inial size */
            if (InitialSize < 2 ||
                InitialSize > pVirtMem->Pagefile[DriveIndex].FreeSize)
            {
                if (LoadString(hApplet,
                               IDS_MESSAGEBOXTITLE,
                               szTitle,
                               sizeof(szTitle) / sizeof(szTitle[0])) == 0)
                    _tcscpy(szTitle, _T("System control panel applet"));

                LoadString(hApplet,
                           IDS_WARNINITIALRANGE,
                           szMessage,
                           sizeof(szMessage) / sizeof(szMessage[0]));

                MessageBox(NULL,
                           szMessage,
                           szTitle,
                           MB_ICONWARNING | MB_OK);
                return;
            }

            /* Check the valid range of the maximum size */
            if (MaximumSize < InitialSize ||
                MaximumSize > pVirtMem->Pagefile[DriveIndex].FreeSize)
            {
                if (LoadString(hApplet,
                               IDS_MESSAGEBOXTITLE,
                               szTitle,
                               sizeof(szTitle) / sizeof(szTitle[0])) == 0)
                    _tcscpy(szTitle, _T("System control panel applet"));

                LoadString(hApplet,
                           IDS_WARNMAXIMUMRANGE,
                           szMessage,
                           sizeof(szMessage) / sizeof(szMessage[0]));

                MessageBox(NULL,
                           szMessage,
                           szTitle,
                           MB_ICONWARNING | MB_OK);
                return;
            }

            pVirtMem->Pagefile[DriveIndex].InitialSize = InitialSize;
            pVirtMem->Pagefile[DriveIndex].MaximumSize = MaximumSize;
            pVirtMem->Pagefile[DriveIndex].bUsed = TRUE;
        }
        else if (IsDlgButtonChecked(pVirtMem->hSelf,
                                    IDC_NOPAGEFILE) == BST_CHECKED)
        {
            /* Set sizes to -1 */
            pVirtMem->Pagefile[DriveIndex].InitialSize = -1;
            pVirtMem->Pagefile[DriveIndex].MaximumSize = -1;
            pVirtMem->Pagefile[DriveIndex].bUsed = TRUE;
        }
        else
        {
            pVirtMem->Pagefile[DriveIndex].InitialSize = 0;
            pVirtMem->Pagefile[DriveIndex].MaximumSize = 0;
            pVirtMem->Pagefile[DriveIndex].bUsed = TRUE;
        }

        UpdatePagefileEntry(pVirtMem, Index, DriveIndex);
    }
}


static BOOL
OnSelChange(HWND hwndDlg, PVIRTMEM pVirtMem)
{
    TCHAR szBuffer[64];
    MEMORYSTATUSEX MemoryStatus;
    ULARGE_INTEGER FreeDiskSpace;
    UINT /*i,*/ FreeMemMb /*, PageFileSizeMb*/;
    INT Index;

    Index = (INT)SendDlgItemMessage(hwndDlg,
                                    IDC_PAGEFILELIST,
                                    LB_GETCURSEL,
                                    0,
                                    0);
    if (Index >= 0 && Index < pVirtMem->Count)
    {
        /* Set drive letter */
        SetDlgItemText(hwndDlg, IDC_DRIVE,
                       pVirtMem->Pagefile[Index].szDrive);

        /* Set available disk space */
        if (GetDiskFreeSpaceEx(pVirtMem->Pagefile[Index].szDrive,
                               NULL, NULL, &FreeDiskSpace))
        {
            pVirtMem->Pagefile[Index].FreeSize = (UINT)(FreeDiskSpace.QuadPart / (1024 * 1024));
            _stprintf(szBuffer, _T("%u MB"), pVirtMem->Pagefile[Index].FreeSize);
            SetDlgItemText(hwndDlg, IDC_SPACEAVAIL, szBuffer);
        }

        if (pVirtMem->Pagefile[Index].InitialSize == -1 &&
            pVirtMem->Pagefile[Index].MaximumSize == -1)
        {
            /* No pagefile */

            EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_MAXSIZE), FALSE);
            EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_INITIALSIZE), FALSE);

            CheckDlgButton(pVirtMem->hSelf, IDC_NOPAGEFILE, BST_CHECKED);
        }
        else if (pVirtMem->Pagefile[Index].InitialSize == 0 &&
                 pVirtMem->Pagefile[Index].MaximumSize == 0)
        {
            /* System managed size*/

            EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_MAXSIZE), FALSE);
            EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_INITIALSIZE), FALSE);

            CheckDlgButton(pVirtMem->hSelf, IDC_SYSMANSIZE, BST_CHECKED);
        }
        else
        {
            /* Custom size */

            /* Enable and fill the custom values */
            EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_MAXSIZE), TRUE);
            EnableWindow(GetDlgItem(pVirtMem->hSelf, IDC_INITIALSIZE), TRUE);

            SetDlgItemInt(pVirtMem->hSelf,
                          IDC_INITIALSIZE,
                          pVirtMem->Pagefile[Index].InitialSize,
                          FALSE);

            SetDlgItemInt(pVirtMem->hSelf,
                          IDC_MAXSIZE,
                          pVirtMem->Pagefile[Index].MaximumSize,
                          FALSE);

            CheckDlgButton(pVirtMem->hSelf,
                           IDC_CUSTOM,
                           BST_CHECKED);
        }

        /* Set minimum pagefile size */
        SetDlgItemText(hwndDlg, IDC_MINIMUM, _T("2 MB"));

        /* Set recommended pagefile size */
        MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&MemoryStatus))
        {
            FreeMemMb = (UINT)(MemoryStatus.ullTotalPhys / (1024 * 1024));
            _stprintf(szBuffer, _T("%u MB"), FreeMemMb + (FreeMemMb / 2));
            SetDlgItemText(hwndDlg, IDC_RECOMMENDED, szBuffer);
        }

        /* Set current pagefile size */
#if 0
        PageFileSizeMb = 0;
        for (i = 0; i < 26; i++)
        {
            PageFileSizeMb += pVirtMem->Pagefile[i].InitialSize;
        }
        _stprintf(szBuffer, _T("%u MB"), PageFileSizeMb);
        SetDlgItemText(hwndDlg, IDC_CURRENT, szBuffer);
#endif
    }

    return TRUE;
}


static VOID
OnOk(PVIRTMEM pVirtMem)
{
    if (pVirtMem->bSave == TRUE)
    {
        WritePageFileSettings(pVirtMem);
    }
}


static VOID
OnInitDialog(HWND hwnd, PVIRTMEM pVirtMem)
{
    INT i;

    pVirtMem->hSelf = hwnd;
    pVirtMem->hListBox = GetDlgItem(hwnd, IDC_PAGEFILELIST);
    pVirtMem->bSave = FALSE;

    SetListBoxColumns(pVirtMem->hListBox);

    for (i = 0; i < 26; i++)
    {
        pVirtMem->Pagefile[i].bUsed = FALSE;
        pVirtMem->Pagefile[i].InitialSize = -1;
        pVirtMem->Pagefile[i].MaximumSize = -1;
    }

    /* Load the pagefile systems from the reg */
    ReadPageFileSettings(pVirtMem);

    /* Parse our settings and set up dialog */
    ParseMemSettings(pVirtMem);

    InitPagefileList(pVirtMem);
}


static VOID
OnDestroy(PVIRTMEM pVirtMem)
{
    INT i;

    for (i = 0; i < 26; i++)
    {
        if (pVirtMem->Pagefile[i].pszVolume != NULL)
            HeapFree(GetProcessHeap(), 0, pVirtMem->Pagefile[i].pszVolume);
    }

    if (pVirtMem->szPagingFiles)
        HeapFree(GetProcessHeap(), 0, pVirtMem->szPagingFiles);

    HeapFree(GetProcessHeap(), 0, pVirtMem);
}


INT_PTR CALLBACK
VirtMemDlgProc(HWND hwndDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam)
{
    PVIRTMEM pVirtMem;

    UNREFERENCED_PARAMETER(lParam);

    pVirtMem = (PVIRTMEM)GetWindowLongPtr(hwndDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
            pVirtMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(VIRTMEM));
            if (pVirtMem == NULL)
            {
                EndDialog(hwndDlg, 0);
                return FALSE;
            }

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pVirtMem);

            OnInitDialog(hwndDlg, pVirtMem);
            break;

        case WM_DESTROY:
            OnDestroy(pVirtMem);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, 0);
                    return TRUE;

                case IDOK:
                    OnOk(pVirtMem);
                    EndDialog(hwndDlg, 0);
                    return TRUE;

                case IDC_NOPAGEFILE:
                    OnNoPagingFile(pVirtMem);
                    return TRUE;

                case IDC_SYSMANSIZE:
                    OnSysManSize(pVirtMem);
                    return TRUE;

                case IDC_CUSTOM:
                    OnCustom(pVirtMem);
                    return TRUE;

                case IDC_SET:
                    OnSet(pVirtMem);
                    return TRUE;

                case IDC_PAGEFILELIST:
                    switch (HIWORD(wParam))
                    {
                        case LBN_SELCHANGE:
                            OnSelChange(hwndDlg, pVirtMem);
                            return TRUE;
                    }
                    break;
            }
            break;
    }

    return FALSE;
}
