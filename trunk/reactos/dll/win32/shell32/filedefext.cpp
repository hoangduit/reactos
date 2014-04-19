/*
 * Provides default file shell extension
 *
 * Copyright 2005 Johannes Anderwald
 * Copyright 2012 Rafal Harabien
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "precomp.h"

WINE_DEFAULT_DEBUG_CHANNEL(shell);

BOOL PathIsExeW(LPCWSTR lpszPath);

BOOL CFileVersionInfo::Load(LPCWSTR pwszPath)
{
    ULONG cbInfo = GetFileVersionInfoSizeW(pwszPath, NULL);
    if (!cbInfo)
    {
        WARN("GetFileVersionInfoSize %ls failed\n", pwszPath);
        return FALSE;
    }

    m_pInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbInfo);
    if (!m_pInfo)
    {
        ERR("HeapAlloc failed bytes %x\n", cbInfo);
        return FALSE;
    }

    if (!GetFileVersionInfoW(pwszPath, 0, cbInfo, m_pInfo))
    {
        ERR("GetFileVersionInfoW failed\n");
        return FALSE;
    }

    LPLANGANDCODEPAGE lpLangCode;
    UINT cBytes;
    if (!VerQueryValueW(m_pInfo, L"\\VarFileInfo\\Translation", (LPVOID *)&lpLangCode, &cBytes) || cBytes < sizeof(LANGANDCODEPAGE))
    {
        ERR("VerQueryValueW failed\n");
        return FALSE;
    }

    /* FIXME: find language from current locale / if not available,
     * default to english
     * for now default to first available language
     */
    m_wLang = lpLangCode->wLang;
    m_wCode = lpLangCode->wCode;
    TRACE("Lang %hx Code %hu\n", m_wLang, m_wCode);

    return TRUE;
}

LPCWSTR CFileVersionInfo::GetString(LPCWSTR pwszName)
{
    if (!m_pInfo)
        return NULL;

    WCHAR wszBuf[256];
    swprintf(wszBuf, L"\\StringFileInfo\\%04x%04x\\%s", m_wLang, m_wCode, pwszName);

    /* Query string in version block */
    LPCWSTR pwszResult = NULL;
    UINT cBytes = 0;
    if (!VerQueryValueW(m_pInfo, wszBuf, (LPVOID *)&pwszResult, &cBytes))
        pwszResult = NULL;

    if (!m_wLang && !m_wCode)
    {
        /* Try US English */
        swprintf(wszBuf, L"\\StringFileInfo\\%04x%04x\\%s", MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 1252, pwszName);
        if (!VerQueryValueW(m_pInfo, wszBuf, (LPVOID *)&pwszResult, &cBytes))
            pwszResult = NULL;
    }

    if (!pwszResult)
        ERR("VerQueryValueW %ls failed\n", pwszName);
    else
        TRACE("%ls: %ls\n", pwszName, pwszResult);

    return pwszResult;
}

VS_FIXEDFILEINFO *CFileVersionInfo::GetFixedInfo()
{
    if (!m_pInfo)
        return NULL;

    VS_FIXEDFILEINFO *pInfo;
    UINT cBytes;
    if (!VerQueryValueW(m_pInfo, L"\\", (PVOID*)&pInfo, &cBytes))
        return NULL;
    return pInfo;
}

LPCWSTR CFileVersionInfo::GetLangName()
{
    if (!m_pInfo)
        return NULL;

    if (!m_wszLang[0])
    {
        if (!VerLanguageNameW(m_wLang, m_wszLang, _countof(m_wszLang)))
            ERR("VerLanguageNameW failed\n");
    }

    return m_wszLang;
}

UINT
SH_FormatInteger(LONGLONG Num, LPWSTR pwszResult, UINT cchResultMax)
{
    // Print the number in uniform mode
    WCHAR wszNumber[24];
    swprintf(wszNumber, L"%I64u", Num);

    // Get system strings for decimal and thousand separators.
    WCHAR wszDecimalSep[8], wszThousandSep[8];
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, wszDecimalSep, _countof(wszDecimalSep));
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, wszThousandSep, _countof(wszThousandSep));

    // Initialize format for printing the number in bytes
    NUMBERFMTW nf;
    ZeroMemory(&nf, sizeof(nf));
    nf.lpDecimalSep = wszDecimalSep;
    nf.lpThousandSep = wszThousandSep;

    // Get system string for groups separator
    WCHAR wszGrouping[12];
    INT cchGrouping = GetLocaleInfoW(LOCALE_USER_DEFAULT,
                                     LOCALE_SGROUPING,
                                     wszGrouping,
                                     _countof(wszGrouping));

    // Convert grouping specs from string to integer
    for (INT i = 0; i < cchGrouping; i++)
    {
        WCHAR wch = wszGrouping[i];

        if (wch >= L'0' && wch <= L'9')
            nf.Grouping = nf.Grouping * 10 + (wch - L'0');
        else if (wch != L';')
            break;
    }

    if ((nf.Grouping % 10) == 0)
        nf.Grouping /= 10;
    else
        nf.Grouping *= 10;

    // Format the number
    INT cchResult = GetNumberFormatW(LOCALE_USER_DEFAULT,
                                    0,
                                    wszNumber,
                                    &nf,
                                    pwszResult,
                                    cchResultMax);

    if (!cchResult)
        return 0;

    // GetNumberFormatW returns number of characters including UNICODE_NULL
    return cchResult - 1;
}

UINT
SH_FormatByteSize(LONGLONG cbSize, LPWSTR pwszResult, UINT cchResultMax)
{
    /* Write formated bytes count */
    INT cchWritten = SH_FormatInteger(cbSize, pwszResult, cchResultMax);
    if (!cchWritten)
        return 0;

    /* Copy " bytes" to buffer */
    LPWSTR pwszEnd = pwszResult + cchWritten;
    size_t cchRemaining = cchResultMax - cchWritten;
    StringCchCopyExW(pwszEnd, cchRemaining, L" ", &pwszEnd, &cchRemaining, NULL);
    cchWritten = LoadStringW(shell32_hInstance, IDS_BYTES_FORMAT, pwszEnd, cchRemaining);
    cchRemaining -= cchWritten;

    return cchResultMax - cchRemaining;
}

/*************************************************************************
 *
 * SH_FormatFileSizeWithBytes
 *
 * Format a size in bytes to string.
 *
 * lpQwSize = Pointer to 64bit large integer to format
 * pszBuf   = Buffer to fill with output string
 * cchBuf   = size of pszBuf in characters
 *
 */

LPWSTR
SH_FormatFileSizeWithBytes(const PULARGE_INTEGER lpQwSize, LPWSTR pwszResult, UINT cchResultMax)
{
    /* Format bytes in KBs, MBs etc */
    if (StrFormatByteSizeW(lpQwSize->QuadPart, pwszResult, cchResultMax) == NULL)
        return NULL;

    /* If there is less bytes than 1KB, we have nothing to do */
    if (lpQwSize->QuadPart < 1024)
        return pwszResult;

    /* Concate " (" */
    UINT cchWritten = wcslen(pwszResult);
    LPWSTR pwszEnd = pwszResult + cchWritten;
    size_t cchRemaining = cchResultMax - cchWritten;
    StringCchCopyExW(pwszEnd, cchRemaining, L" (", &pwszEnd, &cchRemaining, 0);

    /* Write formated bytes count */
    cchWritten = SH_FormatByteSize(lpQwSize->QuadPart, pwszEnd, cchRemaining);
    pwszEnd += cchWritten;
    cchRemaining -= cchWritten;

    /* Copy ")" to the buffer */
    StringCchCopyW(pwszEnd, cchRemaining, L")");

    return pwszResult;
}

/*************************************************************************
 *
 * SH_CreatePropertySheetPage [Internal]
 *
 * creates a property sheet page from an resource name
 *
 */

HPROPSHEETPAGE
SH_CreatePropertySheetPage(WORD wDialogId, DLGPROC pfnDlgProc, LPARAM lParam, LPCWSTR pwszTitle)
{
    HRSRC hRes = FindResourceW(shell32_hInstance, MAKEINTRESOURCEW(wDialogId), (LPWSTR)RT_DIALOG);
    if (hRes == NULL)
    {
        ERR("failed to find resource id\n");
        return NULL;
    }

    LPVOID pTemplate = LoadResource(shell32_hInstance, hRes);
    if (pTemplate == NULL)
    {
        ERR("failed to load resource\n");
        return NULL;
    }

    PROPSHEETPAGEW Page;
    memset(&Page, 0x0, sizeof(PROPSHEETPAGEW));
    Page.dwSize = sizeof(PROPSHEETPAGEW);
    Page.dwFlags = PSP_DLGINDIRECT;
    Page.pResource = (DLGTEMPLATE*)pTemplate;
    Page.pfnDlgProc = pfnDlgProc;
    Page.lParam = lParam;
    Page.pszTitle = pwszTitle;

    if (pwszTitle)
        Page.dwFlags |= PSP_USETITLE;

    return CreatePropertySheetPageW(&Page);
}

VOID
CFileDefExt::InitOpensWithField(HWND hwndDlg)
{
    WCHAR wszBuf[MAX_PATH] = L"";
    WCHAR wszPath[MAX_PATH] = L"";
    DWORD dwSize = sizeof(wszBuf);
    BOOL bUnknownApp = TRUE;
    LPCWSTR pwszExt = PathFindExtensionW(m_wszPath);

    if (RegGetValueW(HKEY_CLASSES_ROOT, pwszExt, L"", RRF_RT_REG_SZ, NULL, wszBuf, &dwSize) == ERROR_SUCCESS)
    {
        bUnknownApp = FALSE;
        StringCbCatW(wszBuf, sizeof(wszBuf), L"\\shell\\open\\command");
        dwSize = sizeof(wszPath);
        if (RegGetValueW(HKEY_CLASSES_ROOT, wszBuf, L"", RRF_RT_REG_SZ, NULL, wszPath, &dwSize) == ERROR_SUCCESS)
        {
            /* Get path from command line */
            ExpandEnvironmentStringsW(wszPath, wszBuf, _countof(wszBuf));
            PathRemoveArgs(wszBuf);
            PathUnquoteSpacesW(wszBuf);
            PathSearchAndQualify(wszBuf, wszPath, _countof(wszPath));

            HICON hIcon;
            if (ExtractIconExW(wszPath, 0, NULL, &hIcon, 1))
            {
                HWND hIconCtrl = GetDlgItem(hwndDlg, 14025);
                HWND hDescrCtrl = GetDlgItem(hwndDlg, 14007);
                ShowWindow(hIconCtrl, SW_SHOW);
                RECT rcIcon, rcDescr;
                GetWindowRect(hIconCtrl, &rcIcon);
                if (rcIcon.left == rcIcon.right)
                    ERR("Icon control has invalid width: %d-%d\n", rcIcon.left, rcIcon.right);
                MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rcIcon, 2);
                GetWindowRect(hDescrCtrl, &rcDescr);
                MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rcDescr, 2);
                INT cxOffset = rcIcon.right + 2 - rcDescr.left;
                SetWindowPos(hDescrCtrl, NULL,
                             rcDescr.left + cxOffset, rcDescr.top,
                             rcDescr.right - rcDescr.left - cxOffset, rcDescr.bottom - rcDescr.top,
                             SWP_NOZORDER);
                SendMessageW(hIconCtrl, STM_SETICON, (WPARAM)hIcon, 0);
            } else
                ERR("Failed to extract icon\n");

            if (PathFileExistsW(wszPath))
            {
                /* Get file description */
                CFileVersionInfo VerInfo;
                VerInfo.Load(wszPath);
                LPCWSTR pwszDescr = VerInfo.GetString(L"FileDescription");
                if (pwszDescr)
                    SetDlgItemTextW(hwndDlg, 14007, pwszDescr);
                else
                {
                    /* File has no description - display filename */
                    LPWSTR pwszFilename = PathFindFileNameW(wszPath);
                    PathRemoveExtension(pwszFilename);
                    pwszFilename[0] = towupper(pwszFilename[0]);
                    SetDlgItemTextW(hwndDlg, 14007, pwszFilename);
                }
            }
            else
                bUnknownApp = TRUE;
        } else
            WARN("RegGetValueW %ls failed\n", wszBuf);
    } else
        WARN("RegGetValueW %ls failed\n", pwszExt);

    if (bUnknownApp)
    {
        /* Unknown application */
        LoadStringW(shell32_hInstance, IDS_UNKNOWN_APP, wszBuf, _countof(wszBuf));
        SetDlgItemTextW(hwndDlg, 14007, wszBuf);
    }
}

/*************************************************************************
 *
 * SH_FileGeneralFileType [Internal]
 *
 * retrieves file extension description from registry and sets it in dialog
 *
 * TODO: retrieve file extension default icon and load it
 *       find executable name from registry, retrieve description from executable
 */

BOOL
CFileDefExt::InitFileType(HWND hwndDlg)
{
    TRACE("path %s\n", debugstr_w(m_wszPath));

    HWND hDlgCtrl = GetDlgItem(hwndDlg, 14005);
    if (hDlgCtrl == NULL)
        return FALSE;

    /* Get file information */
    SHFILEINFO fi;
    if (!SHGetFileInfoW(m_wszPath, 0, &fi, sizeof(fi), SHGFI_TYPENAME|SHGFI_ICON))
    {
        ERR("SHGetFileInfoW failed for %ls (%lu)\n", m_wszPath, GetLastError());
        fi.szTypeName[0] = L'\0';
        fi.hIcon = NULL;
    }

    LPCWSTR pwszExt = PathFindExtensionW(m_wszPath);
    if (pwszExt[0])
    {
        WCHAR wszBuf[256];

        if (!fi.szTypeName[0])
        {
            /* The file type is unknown, so default to string "FileExtension File" */
            size_t cchRemaining = 0;
            LPWSTR pwszEnd = NULL;

            StringCchPrintfExW(wszBuf, _countof(wszBuf), &pwszEnd, &cchRemaining, 0, L"%s ", pwszExt + 1);
            SendMessageW(hDlgCtrl, WM_GETTEXT, (WPARAM)cchRemaining, (LPARAM)pwszEnd);

            SendMessageW(hDlgCtrl, WM_SETTEXT, (WPARAM)NULL, (LPARAM)wszBuf);
        }
        else
        {
            /* Update file type */
            StringCbPrintfW(wszBuf, sizeof(wszBuf), L"%s (%s)", fi.szTypeName, pwszExt);
            SendMessageW(hDlgCtrl, WM_SETTEXT, (WPARAM)NULL, (LPARAM)wszBuf);
        }
    }

    /* Update file icon */
    if (fi.hIcon)
        SendDlgItemMessageW(hwndDlg, 14000, STM_SETICON, (WPARAM)fi.hIcon, 0);
    else
        ERR("No icon %ls\n", m_wszPath);

    return TRUE;
}

/*************************************************************************
 *
 * CFileDefExt::InitFilePath [Internal]
 *
 * sets file path string and filename string
 *
 */

BOOL
CFileDefExt::InitFilePath(HWND hwndDlg)
{
    /* Find the filename */
    WCHAR *pwszFilename = PathFindFileNameW(m_wszPath);

    if (pwszFilename > m_wszPath)
    {
        /* Location field */
        WCHAR wszLocation[MAX_PATH];
        StringCchCopyNW(wszLocation, _countof(wszLocation), m_wszPath, pwszFilename - m_wszPath);
        PathRemoveBackslashW(wszLocation);

        SetDlgItemTextW(hwndDlg, 14009, wszLocation);
    }

    /* Filename field */
    SetDlgItemTextW(hwndDlg, 14001, pwszFilename);

    return TRUE;
}

/*************************************************************************
 *
 * CFileDefExt::GetFileTimeString [Internal]
 *
 * formats a given LPFILETIME struct into readable user format
 */

BOOL
CFileDefExt::GetFileTimeString(LPFILETIME lpFileTime, LPWSTR pwszResult, UINT cchResult)
{
    FILETIME ft;
    SYSTEMTIME st;

    if (!FileTimeToLocalFileTime(lpFileTime, &ft) || !FileTimeToSystemTime(&ft, &st))
        return FALSE;

    size_t cchRemaining = cchResult;
    LPWSTR pwszEnd = pwszResult;
    int cchWritten = GetDateFormatW(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, pwszEnd, cchRemaining);
    if (cchWritten)
        --cchWritten; // GetDateFormatW returns count with terminating zero
    else
        ERR("GetDateFormatW failed\n");
    cchRemaining -= cchWritten;
    pwszEnd += cchWritten;

    StringCchCopyExW(pwszEnd, cchRemaining, L", ", &pwszEnd, &cchRemaining, 0);

    cchWritten = GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &st, NULL, pwszEnd, cchRemaining);
    if (cchWritten)
        --cchWritten; // GetTimeFormatW returns count with terminating zero
    else
        ERR("GetTimeFormatW failed\n");
    TRACE("result %s\n", debugstr_w(pwszResult));
    return TRUE;
}

/*************************************************************************
 *
 * CFileDefExt::InitFileAttr [Internal]
 *
 * retrieves file information from file and sets in dialog
 *
 */

BOOL
CFileDefExt::InitFileAttr(HWND hwndDlg)
{
    WCHAR wszBuf[MAX_PATH];

    TRACE("InitFileAttr %ls\n", m_wszPath);

    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
    if (GetFileAttributesExW(m_wszPath, GetFileExInfoStandard, &FileInfo))
    {
        /* Update attribute checkboxes */
        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            SendDlgItemMessage(hwndDlg, 14021, BM_SETCHECK, BST_CHECKED, 0);
        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            SendDlgItemMessage(hwndDlg, 14022, BM_SETCHECK, BST_CHECKED, 0);
        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
            SendDlgItemMessage(hwndDlg, 14023, BM_SETCHECK, BST_CHECKED, 0);

        /* Update creation time */
        if (GetFileTimeString(&FileInfo.ftCreationTime, wszBuf, _countof(wszBuf)))
            SetDlgItemTextW(hwndDlg, 14015, wszBuf);

        /* For files display last access and last write time */
        if (!m_bDir)
        {
            if (GetFileTimeString(&FileInfo.ftLastAccessTime, wszBuf, _countof(wszBuf)))
                SetDlgItemTextW(hwndDlg, 14019, wszBuf);

            if (GetFileTimeString(&FileInfo.ftLastWriteTime, wszBuf, _countof(wszBuf)))
                SetDlgItemTextW(hwndDlg, 14017, wszBuf);

            /* Update size of file */
            ULARGE_INTEGER FileSize;
            FileSize.u.LowPart = FileInfo.nFileSizeLow;
            FileSize.u.HighPart = FileInfo.nFileSizeHigh;
            if (SH_FormatFileSizeWithBytes(&FileSize, wszBuf, _countof(wszBuf)))
                SetDlgItemTextW(hwndDlg, 14011, wszBuf);
        }
    }

    if (m_bDir)
    {
        /* For directories files have to be counted */

        _CountFolderAndFilesData *data = static_cast<_CountFolderAndFilesData*>(HeapAlloc(GetProcessHeap(), 0, sizeof(_CountFolderAndFilesData)));
        data->This = this;
        data->pwszBuf = static_cast<LPWSTR>(HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR) * MAX_PATH));
        data->cchBufMax = MAX_PATH;
        data->hwndDlg = hwndDlg;
        this->AddRef();
        StringCchCopyW(data->pwszBuf, MAX_PATH, m_wszPath);

        SHCreateThread(CFileDefExt::_CountFolderAndFilesThreadProc, data, NULL, NULL);

        /* Update size field */
        if (SH_FormatFileSizeWithBytes(&m_DirSize, wszBuf, _countof(wszBuf)))
            SetDlgItemTextW(hwndDlg, 14011, wszBuf);

        /* Display files and folders count */
        WCHAR wszFormat[256];
        LoadStringW(shell32_hInstance, IDS_FILE_FOLDER, wszFormat, _countof(wszFormat));
        StringCchPrintfW(wszBuf, _countof(wszBuf), wszFormat, m_cFiles, m_cFolders);
        SetDlgItemTextW(hwndDlg, 14027, wszBuf);
    }

    /* Hide Advanced button. TODO: Implement advanced dialog and enable this button if filesystem supports compression or encryption */
    ShowWindow(GetDlgItem(hwndDlg, 14028), SW_HIDE);

    return TRUE;
}

/*************************************************************************
 *
 * CFileDefExt::InitGeneralPage [Internal]
 *
 * sets all file general properties in dialog
 */

BOOL
CFileDefExt::InitGeneralPage(HWND hwndDlg)
{
    /* Set general text properties filename filelocation and icon */
    InitFilePath(hwndDlg);

    /* Set file type and icon */
    InitFileType(hwndDlg);

    /* Set open with application */
    if (!m_bDir)
    {
        if (!PathIsExeW(m_wszPath))
            InitOpensWithField(hwndDlg);
        else
        {
            WCHAR wszBuf[MAX_PATH];
            LoadStringW(shell32_hInstance, IDS_EXE_DESCRIPTION, wszBuf, _countof(wszBuf));
            SetDlgItemTextW(hwndDlg, 14006, wszBuf);
            ShowWindow(GetDlgItem(hwndDlg, 14024), SW_HIDE);
            LPCWSTR pwszDescr = m_VerInfo.GetString(L"FileDescription");
            if (pwszDescr)
                SetDlgItemTextW(hwndDlg, 14007, pwszDescr);
            else
            {
                StringCbCopyW(wszBuf, sizeof(wszBuf), PathFindFileNameW(m_wszPath));
                PathRemoveExtension(wszBuf);
                SetDlgItemTextW(hwndDlg, 14007, wszBuf);
            }
        }
    }

    /* Set file created/modfied/accessed time, size and attributes */
    InitFileAttr(hwndDlg);

    return TRUE;
}

/*************************************************************************
 *
 * CFileDefExt::GeneralPageProc
 *
 * wnd proc of 'General' property sheet page
 *
 */

INT_PTR CALLBACK
CFileDefExt::GeneralPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            LPPROPSHEETPAGEW ppsp = (LPPROPSHEETPAGEW)lParam;

            if (ppsp == NULL || !ppsp->lParam)
                break;

            TRACE("WM_INITDIALOG hwnd %p lParam %p ppsplParam %S\n", hwndDlg, lParam, ppsp->lParam);

            CFileDefExt *pFileDefExt = reinterpret_cast<CFileDefExt *>(ppsp->lParam);
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pFileDefExt);
            pFileDefExt->InitGeneralPage(hwndDlg);
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 14024) /* Opens With - Change */
            {
                CFileDefExt *pFileDefExt = reinterpret_cast<CFileDefExt *>(GetWindowLongPtr(hwndDlg, DWLP_USER));
                OPENASINFO oainfo;
                oainfo.pcszFile = pFileDefExt->m_wszPath;
                oainfo.pcszClass = NULL;
                oainfo.oaifInFlags = OAIF_REGISTER_EXT|OAIF_FORCE_REGISTRATION;
                return SUCCEEDED(SHOpenWithDialog(hwndDlg, &oainfo));
            }
            else if (LOWORD(wParam) == 14021 || LOWORD(wParam) == 14022 || LOWORD(wParam) == 14023) /* checkboxes */
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            else if (LOWORD(wParam) == 14001) /* Name */
            {
                if (HIWORD(wParam) == EN_CHANGE)
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
            break;
        case WM_NOTIFY:
        {
            LPPSHNOTIFY lppsn = (LPPSHNOTIFY)lParam;
            if (lppsn->hdr.code == PSN_APPLY)
            {
                CFileDefExt *pFileDefExt = reinterpret_cast<CFileDefExt *>(GetWindowLongPtr(hwndDlg, DWLP_USER));

                /* Update attributes first */
                DWORD dwAttr = GetFileAttributesW(pFileDefExt->m_wszPath);
                if (dwAttr)
                {
                    dwAttr &= ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_ARCHIVE);

                    if (BST_CHECKED == SendDlgItemMessageW(hwndDlg, 14021, BM_GETCHECK, 0, 0))
                        dwAttr |= FILE_ATTRIBUTE_READONLY;
                    if (BST_CHECKED == SendDlgItemMessageW(hwndDlg, 14022, BM_GETCHECK, 0, 0))
                        dwAttr |= FILE_ATTRIBUTE_HIDDEN;
                    if (BST_CHECKED == SendDlgItemMessageW(hwndDlg, 14023, BM_GETCHECK, 0, 0))
                        dwAttr |= FILE_ATTRIBUTE_ARCHIVE;

                    if (!SetFileAttributesW(pFileDefExt->m_wszPath, dwAttr))
                        ERR("SetFileAttributesW failed\n");
                }

                /* Update filename now */
                WCHAR wszBuf[MAX_PATH];
                StringCchCopyW(wszBuf, _countof(wszBuf), pFileDefExt->m_wszPath);
                LPWSTR pwszFilename = PathFindFileNameW(wszBuf);
                UINT cchFilenameMax = _countof(wszBuf) - (pwszFilename - wszBuf);
                if (GetDlgItemTextW(hwndDlg, 14001, pwszFilename, cchFilenameMax))
                {
                    if (!MoveFileW(pFileDefExt->m_wszPath, wszBuf))
                        ERR("MoveFileW failed\n");
                }

                SetWindowLongPtr(hwndDlg, DWL_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
            break;
        }
        default:
            break;
    }

    return FALSE;
}

/*************************************************************************
 *
 * CFileDefExt::InitVersionPage [Internal]
 *
 * sets all file version properties in dialog
 */

BOOL
CFileDefExt::InitVersionPage(HWND hwndDlg)
{
    /* Get fixed info */
    VS_FIXEDFILEINFO *pInfo = m_VerInfo.GetFixedInfo();
    if (pInfo)
    {
        WCHAR wszVersion[256];
        swprintf(wszVersion, L"%u.%u.%u.%u", HIWORD(pInfo->dwFileVersionMS),
                 LOWORD(pInfo->dwFileVersionMS),
                 HIWORD(pInfo->dwFileVersionLS),
                 LOWORD(pInfo->dwFileVersionLS));
        TRACE("MS %x LS %x ver %s \n", pInfo->dwFileVersionMS, pInfo->dwFileVersionLS, debugstr_w(wszVersion));
        SetDlgItemTextW(hwndDlg, 14001, wszVersion);
    }

    /* Update labels */
    SetVersionLabel(hwndDlg, 14003, L"FileDescription");
    SetVersionLabel(hwndDlg, 14005, L"LegalCopyright");

    /* Add items to listbox */
    AddVersionString(hwndDlg, L"CompanyName");
    LPCWSTR pwszLang = m_VerInfo.GetLangName();
    if (pwszLang)
    {
        HWND hDlgCtrl = GetDlgItem(hwndDlg, 14009);
        UINT Index = SendMessageW(hDlgCtrl, LB_ADDSTRING, (WPARAM)-1, (LPARAM)L"Language");
        SendMessageW(hDlgCtrl, LB_SETITEMDATA, (WPARAM)Index, (LPARAM)(WCHAR *)pwszLang);
    }
    AddVersionString(hwndDlg, L"ProductName");
    AddVersionString(hwndDlg, L"InternalName");
    AddVersionString(hwndDlg, L"OriginalFilename");
    AddVersionString(hwndDlg, L"FileVersion");
    AddVersionString(hwndDlg, L"ProductVersion");

    /* Attach file version to dialog window */
    SetWindowLongPtr(hwndDlg, DWL_USER, (LONG_PTR)this);

    /* Select first item */
    HWND hDlgCtrl = GetDlgItem(hwndDlg, 14009);
    SendMessageW(hDlgCtrl, LB_SETCURSEL, 0, 0);
    LPCWSTR pwszText = (LPCWSTR)SendMessageW(hDlgCtrl, LB_GETITEMDATA, (WPARAM)0, (LPARAM)NULL);
    if (pwszText && pwszText != (LPCWSTR)LB_ERR)
        SetDlgItemTextW(hwndDlg, 14010, pwszText);

    return TRUE;
}

/*************************************************************************
 *
 * CFileDefExt::SetVersionLabel [Internal]
 *
 * retrieves a version string and uses it to set label text
 */

BOOL
CFileDefExt::SetVersionLabel(HWND hwndDlg, DWORD idCtrl, LPCWSTR pwszName)
{
    if (hwndDlg == NULL || pwszName == NULL)
        return FALSE;

    LPCWSTR pwszValue = m_VerInfo.GetString(pwszName);
    if (pwszValue)
    {
        /* file description property */
        TRACE("%s :: %s\n", debugstr_w(pwszName), debugstr_w(pwszValue));
        SetDlgItemTextW(hwndDlg, idCtrl, pwszValue);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
 *
 * CFileDefExt::AddVersionString [Internal]
 *
 * retrieves a version string and adds it to listbox
 */

BOOL
CFileDefExt::AddVersionString(HWND hwndDlg, LPCWSTR pwszName)
{
    TRACE("pwszName %s, hwndDlg %p\n", debugstr_w(pwszName), hwndDlg);

    if (hwndDlg == NULL || pwszName == NULL)
        return FALSE;

    LPCWSTR pwszValue = m_VerInfo.GetString(pwszName);
    if (pwszValue)
    {
        /* listbox name property */
        HWND hDlgCtrl = GetDlgItem(hwndDlg, 14009);
        TRACE("%s :: %s\n", debugstr_w(pwszName), debugstr_w(pwszValue));
        UINT Index = SendMessageW(hDlgCtrl, LB_ADDSTRING, (WPARAM) -1, (LPARAM)pwszName);
        SendMessageW(hDlgCtrl, LB_SETITEMDATA, (WPARAM)Index, (LPARAM)(WCHAR *)pwszValue);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
 *
 * CFileDefExt::VersionPageProc
 *
 * wnd proc of 'Version' property sheet page
 */

INT_PTR CALLBACK
CFileDefExt::VersionPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE)lParam;

            if (ppsp == NULL || !ppsp->lParam)
                break;

            TRACE("WM_INITDIALOG hwnd %p lParam %p ppsplParam %x\n", hwndDlg, lParam, ppsp->lParam);

            CFileDefExt *pFileDefExt = reinterpret_cast<CFileDefExt *>(ppsp->lParam);
            return pFileDefExt->InitVersionPage(hwndDlg);
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 14009 && HIWORD(wParam) == LBN_SELCHANGE)
            {
                HWND hDlgCtrl = (HWND)lParam;

                LRESULT Index = SendMessageW(hDlgCtrl, LB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL);
                if (Index == LB_ERR)
                    break;

                LPCWSTR pwszData = (LPCWSTR)SendMessageW(hDlgCtrl, LB_GETITEMDATA, (WPARAM)Index, (LPARAM)NULL);
                if (pwszData == NULL)
                    break;

                TRACE("hDlgCtrl %x string %s\n", hDlgCtrl, debugstr_w(pwszData));
                SetDlgItemTextW(hwndDlg, 14010, pwszData);

                return TRUE;
            }
            break;
        case WM_DESTROY:
            break;
        default:
            break;
    }

    return FALSE;
}

CFileDefExt::CFileDefExt():
    m_bDir(FALSE), m_cFiles(0), m_cFolders(0)
{
    m_wszPath[0] = L'\0';
    m_DirSize.QuadPart = 0ull;
}

CFileDefExt::~CFileDefExt()
{

}

HRESULT WINAPI
CFileDefExt::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pDataObj, HKEY hkeyProgID)
{
    FORMATETC format;
    STGMEDIUM stgm;
    HRESULT hr;

    TRACE("%p %p %p %p\n", this, pidlFolder, pDataObj, hkeyProgID);

    if (!pDataObj)
        return E_FAIL;

    format.cfFormat = CF_HDROP;
    format.ptd = NULL;
    format.dwAspect = DVASPECT_CONTENT;
    format.lindex = -1;
    format.tymed = TYMED_HGLOBAL;

    hr = pDataObj->GetData(&format, &stgm);
    if (FAILED(hr))
        return hr;

    if (!DragQueryFileW((HDROP)stgm.hGlobal, 0, m_wszPath, _countof(m_wszPath)))
    {
        ERR("DragQueryFileW failed\n");
        ReleaseStgMedium(&stgm);
        return E_FAIL;
    }

    ReleaseStgMedium(&stgm);

    TRACE("File properties %ls\n", m_wszPath);
    m_bDir = PathIsDirectoryW(m_wszPath) ? TRUE : FALSE;
    if (!m_bDir)
        m_VerInfo.Load(m_wszPath);

    return S_OK;
}

HRESULT WINAPI
CFileDefExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    UNIMPLEMENTED;
    return E_NOTIMPL;
}

HRESULT WINAPI
CFileDefExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    UNIMPLEMENTED;
    return E_NOTIMPL;
}

HRESULT WINAPI
CFileDefExt::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    UNIMPLEMENTED;
    return E_NOTIMPL;
}

HRESULT WINAPI
CFileDefExt::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE hPage;
    WORD wResId = m_bDir ? IDD_FOLDER_PROPERTIES : IDD_FILE_PROPERTIES;

    hPage = SH_CreatePropertySheetPage(wResId,
                                       GeneralPageProc,
                                       (LPARAM)this,
                                       NULL);
    if (hPage)
        pfnAddPage(hPage, lParam);

    if (!m_bDir && GetFileVersionInfoSizeW(m_wszPath, NULL))
    {
        hPage = SH_CreatePropertySheetPage(IDD_FILE_VERSION,
                                            VersionPageProc,
                                            (LPARAM)this,
                                            NULL);
        if (hPage)
            pfnAddPage(hPage, lParam);
    }

    return S_OK;
}

HRESULT WINAPI
CFileDefExt::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplacePage, LPARAM lParam)
{
    UNIMPLEMENTED;
    return E_NOTIMPL;
}

HRESULT WINAPI
CFileDefExt::SetSite(IUnknown *punk)
{
    UNIMPLEMENTED;
    return E_NOTIMPL;
}

HRESULT WINAPI
CFileDefExt::GetSite(REFIID iid, void **ppvSite)
{
    UNIMPLEMENTED;
    return E_NOTIMPL;
}

DWORD WINAPI
CFileDefExt::_CountFolderAndFilesThreadProc(LPVOID lpParameter)
{
    _CountFolderAndFilesData *data = static_cast<_CountFolderAndFilesData*>(lpParameter);
    DWORD ticks = 0;
    data->This->CountFolderAndFiles(data->hwndDlg, data->pwszBuf, data->cchBufMax, &ticks);

    //Release the CFileDefExt and data object holds in the copying thread.
    data->This->Release();
    HeapFree(GetProcessHeap(), 0, data->pwszBuf);
    HeapFree(GetProcessHeap(), 0, data);

    return 0;
}

BOOL
CFileDefExt::CountFolderAndFiles(HWND hwndDlg, LPWSTR pwszBuf, UINT cchBufMax, DWORD *ticks)
{
    /* Find filename position */
    UINT cchBuf = wcslen(pwszBuf);
    WCHAR *pwszFilename = pwszBuf + cchBuf;
    size_t cchFilenameMax = cchBufMax - cchBuf;
    if (!cchFilenameMax)
        return FALSE;
    *(pwszFilename++) = '\\';
    --cchFilenameMax;

    /* Find all files, FIXME: shouldn't be "*"? */
    StringCchCopyW(pwszFilename, cchFilenameMax, L"*");

    WIN32_FIND_DATAW wfd;
    HANDLE hFind = FindFirstFileW(pwszBuf, &wfd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        ERR("FindFirstFileW %ls failed\n", pwszBuf);
        return FALSE;
    }

    BOOL root = FALSE;
    if (*ticks == 0) {
        *ticks = GetTickCount();
        root = TRUE;
    }

    do
    {
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            /* Don't process "." and ".." items */
            if (!wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L".."))
                continue;

            ++m_cFolders;

            StringCchCopyW(pwszFilename, cchFilenameMax, wfd.cFileName);
            CountFolderAndFiles(hwndDlg, pwszBuf, cchBufMax, ticks);
        }
        else
        {
            m_cFiles++;

            ULARGE_INTEGER FileSize;
            FileSize.u.LowPart  = wfd.nFileSizeLow;
            FileSize.u.HighPart = wfd.nFileSizeHigh;
            m_DirSize.QuadPart += FileSize.QuadPart;
        }
        if (GetTickCount() - *ticks > (DWORD) 300)
        {
            /* FIXME Using IsWindow is generally ill advised */
            if (IsWindow(hwndDlg))
            {
                WCHAR wszBuf[MAX_PATH];

                if (SH_FormatFileSizeWithBytes(&m_DirSize, wszBuf, _countof(wszBuf)))
                    SetDlgItemTextW(hwndDlg, 14011, wszBuf);

                /* Display files and folders count */
                WCHAR wszFormat[256];
                LoadStringW(shell32_hInstance, IDS_FILE_FOLDER, wszFormat, _countof(wszFormat));
                StringCchPrintfW(wszBuf, _countof(wszBuf), wszFormat, m_cFiles, m_cFolders);
                SetDlgItemTextW(hwndDlg, 14027, wszBuf);
                *ticks = GetTickCount();
            }
            else
                break;
        }
    } while(FindNextFileW(hFind, &wfd));

    if (root && IsWindow(hwndDlg))
    {
        WCHAR wszBuf[MAX_PATH];

        if (SH_FormatFileSizeWithBytes(&m_DirSize, wszBuf, _countof(wszBuf)))
            SetDlgItemTextW(hwndDlg, 14011, wszBuf);

        /* Display files and folders count */
        WCHAR wszFormat[256];
        LoadStringW(shell32_hInstance, IDS_FILE_FOLDER, wszFormat, _countof(wszFormat));
        StringCchPrintfW(wszBuf, _countof(wszBuf), wszFormat, m_cFiles, m_cFolders);
        SetDlgItemTextW(hwndDlg, 14027, wszBuf);
    }

    FindClose(hFind);
    return TRUE;
}
