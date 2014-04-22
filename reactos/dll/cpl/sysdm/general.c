/*
 * PROJECT:     ReactOS System Control Panel Applet
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        dll/cpl/sysdm/general.c
 * PURPOSE:     General System Information
 * COPYRIGHT:   Copyright Thomas Weidenmueller <w3seek@reactos.org>
 *              Copyright 2006 Ged Murphy <gedmurphy@gmail.com>
 *              Copyright 2006-2007 Colin Finck <mail@colinfinck.de>
 *
 */

#include "precomp.h"

#include <winnls.h>
#include <powrprof.h>

#define ANIM_STEP 2
#define ANIM_TIME 50

typedef struct _IMGINFO
{
    HBITMAP hBitmap;
    INT cxSource;
    INT cySource;
    INT iPLanes;
    INT iBits;
} IMGINFO, *PIMGINFO;

PIMGINFO pImgInfo = NULL;
BLENDFUNCTION BlendFunc = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

VOID ShowLastWin32Error(HWND hWndOwner)
{
    LPTSTR lpMsg;
    DWORD LastError;

    LastError = GetLastError();

    if ((LastError == 0) ||
         !FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        LastError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&lpMsg,
                        0,
                        NULL))
    {
        return;
    }

    MessageBox(hWndOwner, lpMsg, NULL, MB_OK | MB_ICONERROR);

    LocalFree((LPVOID)lpMsg);
}


static VOID InitLogo(HWND hwndDlg)
{
    BITMAP logoBitmap;
    BITMAP maskBitmap;
    BITMAPINFO bmpi;
    HDC hDC = GetDC(hwndDlg);
    HDC hDCLogo = CreateCompatibleDC(NULL);
    HDC hDCMask = CreateCompatibleDC(NULL);
    HBITMAP hMask, hLogo, hAlphaLogo = NULL;
    COLORREF *pBits;
    INT line, column;

    ZeroMemory(pImgInfo, sizeof(*pImgInfo));
    ZeroMemory(&bmpi, sizeof(bmpi));

    hLogo = (HBITMAP)LoadImage(hApplet, MAKEINTRESOURCE(IDB_ROSBMP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    hMask = (HBITMAP)LoadImage(hApplet, MAKEINTRESOURCE(IDB_ROSMASK), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    if (hLogo != NULL && hMask != NULL)
    {
        GetObject(hLogo, sizeof(BITMAP), &logoBitmap);
        GetObject(hMask, sizeof(BITMAP), &maskBitmap);

        if(logoBitmap.bmHeight != maskBitmap.bmHeight || logoBitmap.bmWidth != maskBitmap.bmWidth)
            return;

        pImgInfo->cxSource = logoBitmap.bmWidth;
        pImgInfo->cySource = logoBitmap.bmHeight;

        bmpi.bmiHeader.biSize = sizeof(BITMAPINFO);
        bmpi.bmiHeader.biWidth = logoBitmap.bmWidth;
        bmpi.bmiHeader.biHeight = logoBitmap.bmHeight;
        bmpi.bmiHeader.biPlanes = 1;
        bmpi.bmiHeader.biBitCount = 32;
        bmpi.bmiHeader.biCompression = BI_RGB;
        bmpi.bmiHeader.biSizeImage = 4 * logoBitmap.bmWidth * logoBitmap.bmHeight;

        hAlphaLogo = CreateDIBSection(hDC, &bmpi, DIB_RGB_COLORS, (PVOID*)&pBits, 0, 0);

        if(!hAlphaLogo)
            return;

        SelectObject(hDCLogo, hLogo);
        SelectObject(hDCMask, hMask);

        for(line = logoBitmap.bmHeight - 1; line >= 0; line--)
        {
            for(column = 0; column < logoBitmap.bmWidth; column++)
            {
                COLORREF alpha = GetPixel(hDCMask, column, line) & 0xFF;
                COLORREF Color = GetPixel(hDCLogo, column, line);
                DWORD r, g, b;

                r = GetRValue(Color) * alpha / 255;
                g = GetGValue(Color) * alpha / 255;
                b = GetBValue(Color) * alpha / 255;

                *pBits++ =  b | g << 8 | r << 16 | alpha << 24;
            }
          }
    }

    pImgInfo->hBitmap = hAlphaLogo;
    pImgInfo->cxSource = logoBitmap.bmWidth;
    pImgInfo->cySource = logoBitmap.bmHeight;
    pImgInfo->iBits = logoBitmap.bmBitsPixel;
    pImgInfo->iPLanes = logoBitmap.bmPlanes;

    DeleteObject(hLogo);
    DeleteObject(hMask);
    DeleteDC(hDCLogo);
    DeleteDC(hDCMask);

}

LRESULT CALLBACK RosImageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT timerid = 0, top = 0, offset;
    static HBITMAP hCreditsBitmap;

    switch (uMsg)
    {
        case WM_LBUTTONDBLCLK:
            if (wParam & (MK_CONTROL | MK_SHIFT))
            {
                if (timerid == 0)
                {
                    HDC hCreditsDC, hLogoDC;
                    HFONT hFont;
                    NONCLIENTMETRICS ncm;
                    RECT rcCredits;
                    TCHAR szCredits[2048];
                    INT iDevsHeight;

                    top = 0;
                    offset = 0;
                    hCreditsDC = CreateCompatibleDC(GetDC(NULL));
                    hLogoDC = CreateCompatibleDC(hCreditsDC);

                    if (hCreditsDC == NULL || hLogoDC == NULL)
                        break;

                    SetRect(&rcCredits, 0, 0, 0, 0);

                    ncm.cbSize = sizeof(NONCLIENTMETRICS);
                    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

                    hFont = CreateFontIndirect(&ncm.lfMessageFont);
                    SelectObject(hCreditsDC, hFont);

                    LoadString(hApplet, IDS_DEVS, szCredits, sizeof(szCredits) / sizeof(TCHAR));
                    DrawText(hCreditsDC, szCredits, -1, &rcCredits, DT_CALCRECT);

                    iDevsHeight = rcCredits.bottom - rcCredits.top;

                    hCreditsBitmap = CreateBitmap(pImgInfo->cxSource, (2 * pImgInfo->cySource) + iDevsHeight + 1, pImgInfo->iPLanes, pImgInfo->iBits, NULL);

                    if(!hCreditsBitmap)
                        break;

                    SelectObject(hLogoDC, pImgInfo->hBitmap);
                    SelectObject(hCreditsDC, hCreditsBitmap);

                    offset += pImgInfo->cySource;

                    SetRect(&rcCredits, 0, 0, pImgInfo->cxSource, (2 * pImgInfo->cySource) + iDevsHeight + 1);
                    FillRect(hCreditsDC, &rcCredits, GetSysColorBrush(COLOR_3DFACE));

                    SetRect(&rcCredits, 0, offset, pImgInfo->cxSource, offset + iDevsHeight + 1);
                    SetBkMode(hCreditsDC, TRANSPARENT);

                    OffsetRect(&rcCredits, 1, 1);
                    SetTextColor(hCreditsDC, GetSysColor(COLOR_BTNSHADOW));
                    DrawText(hCreditsDC, szCredits, -1, &rcCredits, DT_CENTER);

                    OffsetRect(&rcCredits, -1, -1);
                    SetTextColor(hCreditsDC, GetSysColor(COLOR_WINDOWTEXT));
                    DrawText(hCreditsDC, szCredits, -1, &rcCredits, DT_CENTER);

                    offset += iDevsHeight;

                    AlphaBlend(hCreditsDC, 0, 0, pImgInfo->cxSource, pImgInfo->cySource, hLogoDC, 0,  0, pImgInfo->cxSource, pImgInfo->cySource, BlendFunc);
                    AlphaBlend(hCreditsDC, 0, offset, pImgInfo->cxSource, pImgInfo->cySource, hLogoDC, 0,  0, pImgInfo->cxSource, pImgInfo->cySource, BlendFunc);

                    DeleteDC(hLogoDC);
                    DeleteDC(hCreditsDC);

                    timerid = SetTimer(hwnd, 1, ANIM_TIME, NULL);
                }
            }
            break;
        case WM_LBUTTONDOWN:
            if (timerid)
            {
                RECT rcCredits;
                HDC hDC = GetDC(hwnd);

                GetClientRect(hwnd, &rcCredits);
                SetRect(&rcCredits, 0, 0, rcCredits.right, pImgInfo->cySource);
                FillRect(hDC, &rcCredits, GetSysColorBrush(COLOR_3DFACE));

                KillTimer(hwnd, timerid);
                DeleteObject(hCreditsBitmap);
                InvalidateRect(hwnd, NULL, FALSE);

                top = 0;
                timerid = 0;
            }
            break;
        case WM_TIMER:
            top += ANIM_STEP;

            if (top > offset)
            {
                RECT rcCredits;
                HDC hDC = GetDC(hwnd);

                GetClientRect(hwnd, &rcCredits);
                SetRect(&rcCredits, 0, 0, rcCredits.right, pImgInfo->cySource);
                FillRect(hDC, &rcCredits, GetSysColorBrush(COLOR_3DFACE));

                KillTimer(hwnd, timerid);
                DeleteObject(hCreditsBitmap);

                top = 0;
                timerid = 0;
            }

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT PS;
            HDC hdcMem, hdc;
            LONG left;

            hdc = wParam != 0 ? (HDC)wParam : BeginPaint(hwnd, &PS);

            GetClientRect(hwnd, &PS.rcPaint);

            /* Position image in center of dialog */
            left = (PS.rcPaint.right - pImgInfo->cxSource) / 2;
            hdcMem = CreateCompatibleDC(hdc);

            if (hdcMem != NULL)
            {
                if(timerid != 0)
                {
                    SelectObject(hdcMem, hCreditsBitmap);
                    BitBlt(hdc, left, PS.rcPaint.top, PS.rcPaint.right - PS.rcPaint.left, PS.rcPaint.top + pImgInfo->cySource, hdcMem, 0, top, SRCCOPY);
                }
                else
                {
                    SelectObject(hdcMem, pImgInfo->hBitmap);
                    AlphaBlend(hdc, left, PS.rcPaint.top, pImgInfo->cxSource, pImgInfo->cySource, hdcMem, 0, 0, pImgInfo->cxSource, pImgInfo->cySource, BlendFunc);
                }

                DeleteDC(hdcMem);
            }

            if (wParam == 0)
                EndPaint(hwnd,&PS);
            break;
        }
    }
    return TRUE;
}

static VOID SetRegTextData(HWND hwnd, HKEY hKey, LPTSTR Value, UINT uID)
{
    LPTSTR lpBuf = NULL;
    DWORD BufSize = 0;
    DWORD Type;

    if (RegQueryValueEx(hKey, Value, NULL, &Type, NULL, &BufSize) == ERROR_SUCCESS)
    {
        lpBuf = HeapAlloc(GetProcessHeap(), 0, BufSize);

        if (!lpBuf)
            return;

        if (RegQueryValueEx(hKey, Value, NULL, &Type, (PBYTE)lpBuf, &BufSize) == ERROR_SUCCESS)
            SetDlgItemText(hwnd, uID, lpBuf);

        HeapFree(GetProcessHeap(), 0, lpBuf);
    }
}

static INT SetProcNameString(HWND hwnd, HKEY hKey, LPTSTR Value, UINT uID1, UINT uID2)
{
    LPTSTR lpBuf = NULL;
    DWORD BufSize = 0;
    DWORD Type;
    INT Ret = 0;
    TCHAR szBuf[31];
    TCHAR* szLastSpace;
    INT LastSpace = 0;

    if (RegQueryValueEx(hKey, Value, NULL, &Type, NULL, &BufSize) == ERROR_SUCCESS)
    {
        lpBuf = HeapAlloc(GetProcessHeap(), 0, BufSize);

        if (!lpBuf)
            return 0;

        if (RegQueryValueEx(hKey, Value, NULL, &Type, (PBYTE)lpBuf, &BufSize) == ERROR_SUCCESS)
        {
            if (BufSize > ((30 + 1) * sizeof(TCHAR)))
            {
                /* Wrap the Processor Name String like XP does:                           *
                *   - Take the first 30 characters and look for the last space.          *
                *     Then wrap the string after this space.                             *
                *   - If no space is found, wrap the string after character 30.          *
                *                                                                        *
                * For example the Processor Name String of a Pentium 4 is right-aligned. *
                * With this wrapping the first line looks centered.                      */

                _tcsncpy(szBuf, lpBuf, 30);
                szBuf[30] = 0;
                szLastSpace = _tcsrchr(szBuf, ' ');

                if (szLastSpace == 0)
                {
                    LastSpace = 30;
                }
                else
                {
                    LastSpace = (szLastSpace - szBuf);
                    szBuf[LastSpace] = 0;
                }

                _tcsncpy(szBuf, lpBuf, LastSpace);

                SetDlgItemText(hwnd, uID1, szBuf);
                SetDlgItemText(hwnd, uID2, lpBuf+LastSpace+1);

                /* Return the number of used lines */
                Ret = 2;
            }
            else
            {
                SetDlgItemText(hwnd, uID1, lpBuf);
                Ret = 1;
            }
        }

        HeapFree(GetProcessHeap(), 0, lpBuf);
    }

    return Ret;
}

static VOID MakeFloatValueString(DOUBLE* dFloatValue, LPTSTR szOutput, LPTSTR szAppend)
{
    TCHAR szDecimalSeparator[4];

    /* Get the decimal separator for the current locale */
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSeparator, sizeof(szDecimalSeparator) / sizeof(TCHAR)) > 0)
    {
        UCHAR uDecimals;
        UINT uIntegral;

        /* Show the value with two decimals */
        uIntegral = (UINT)*dFloatValue;
        uDecimals = (UCHAR)((UINT)(*dFloatValue * 100) - uIntegral * 100);

        wsprintf(szOutput, _T("%u%s%02u %s"), uIntegral, szDecimalSeparator, uDecimals, szAppend);
    }
}

static VOID SetProcSpeed(HWND hwnd, HKEY hKey, LPTSTR Value, UINT uID)
{
    TCHAR szBuf[64];
    DWORD BufSize = sizeof(DWORD);
    DWORD Type = REG_SZ;
    PROCESSOR_POWER_INFORMATION ppi;

    ZeroMemory(&ppi, sizeof(ppi));

    if ((CallNtPowerInformation(ProcessorInformation,
                                NULL,
                                0,
                                (PVOID)&ppi,
                                sizeof(ppi)) == STATUS_SUCCESS &&
         ppi.CurrentMhz != 0) || RegQueryValueEx(hKey, Value, NULL, &Type, (PBYTE)&ppi.CurrentMhz, &BufSize) == ERROR_SUCCESS)
    {
        if (ppi.CurrentMhz < 1000)
        {
            wsprintf(szBuf, _T("%lu MHz"), ppi.CurrentMhz);
        }
        else
        {
            double flt = ppi.CurrentMhz / 1000.0;
            MakeFloatValueString(&flt, szBuf, _T("GHz"));
        }

        SetDlgItemText(hwnd, uID, szBuf);
    }
}

static VOID GetSystemInformation(HWND hwnd)
{
    HKEY hKey;
    TCHAR ProcKey[] = _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");
    MEMORYSTATUSEX MemStat;
    TCHAR Buf[32];
    INT CurMachineLine = IDC_MACHINELINE1;

    /*
     * Get Processor information
     * although undocumented, this information is being pulled
     * directly out of the registry instead of via setupapi as it
     * contains all the info we need, and should remain static
     */
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, ProcKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        SetRegTextData(hwnd, hKey, _T("VendorIdentifier"), CurMachineLine);
        CurMachineLine++;

        CurMachineLine += SetProcNameString(hwnd,
                                            hKey,
                                            _T("ProcessorNameString"),
                                            CurMachineLine,
                                            CurMachineLine + 1);

        SetProcSpeed(hwnd, hKey, _T("~MHz"), CurMachineLine);
        CurMachineLine++;
    }

    /* Get total physical RAM */
    MemStat.dwLength = sizeof(MemStat);

    if (GlobalMemoryStatusEx(&MemStat))
    {
        TCHAR szStr[32];
        double dTotalPhys;

        if (MemStat.ullTotalPhys > 1024 * 1024 * 1024)
        {
            UINT i = 0;
            static const UINT uStrId[] = { IDS_GIGABYTE, IDS_TERABYTE, IDS_PETABYTE};

            // We're dealing with GBs or more
            MemStat.ullTotalPhys /= 1024 * 1024;

            if (MemStat.ullTotalPhys > 1024 * 1024)
            {
                // We're dealing with TBs or more
                MemStat.ullTotalPhys /= 1024;
                i++;

                if (MemStat.ullTotalPhys > 1024 * 1024)
                {
                    // We're dealing with PBs or more
                    MemStat.ullTotalPhys /= 1024;
                    i++;

                    dTotalPhys = (double)MemStat.ullTotalPhys / 1024;
                }
                else
                {
                    dTotalPhys = (double)MemStat.ullTotalPhys / 1024;
                }
            }
            else
            {
                dTotalPhys = (double)MemStat.ullTotalPhys / 1024;
            }

            LoadString(hApplet, uStrId[i], szStr, sizeof(szStr) / sizeof(TCHAR));
            MakeFloatValueString(&dTotalPhys, Buf, szStr);
        }
        else
        {
            // We're dealing with MBs, don't show any decimals
            LoadString(hApplet, IDS_MEGABYTE, szStr, sizeof(szStr) / sizeof(TCHAR));
            wsprintf(Buf, _T("%u %s"), (UINT)MemStat.ullTotalPhys / 1024 / 1024, szStr);
        }

        SetDlgItemText(hwnd, CurMachineLine, Buf);
    }
}


/* Property page dialog callback */
INT_PTR CALLBACK GeneralPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);

    switch (uMsg)
    {
        case WM_INITDIALOG:
            pImgInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMGINFO));
            if (pImgInfo == NULL)
            {
                EndDialog(hwndDlg, 0);
                return FALSE;
            }

            InitLogo(hwndDlg);
            SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_ROSIMG), GWL_WNDPROC, (LONG)RosImageProc);
            GetSystemInformation(hwndDlg);
            break;

        case WM_DESTROY:
            HeapFree(GetProcessHeap(), 0, pImgInfo);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_LICENCE)
            {
                DialogBox(hApplet, MAKEINTRESOURCE(IDD_LICENCE), hwndDlg, LicenceDlgProc);

                return TRUE;
            }
            break;

        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;

            if (lpDrawItem->CtlID == IDC_ROSIMG)
            {
                HDC hdcMem;
                LONG left;

                /* Position image in centre of dialog */
                left = (lpDrawItem->rcItem.right - pImgInfo->cxSource) / 2;

                hdcMem = CreateCompatibleDC(lpDrawItem->hDC);
                if (hdcMem != NULL)
                {
                    SelectObject(hdcMem, pImgInfo->hBitmap);
                    BitBlt(lpDrawItem->hDC,
                           left,
                           lpDrawItem->rcItem.top,
                           lpDrawItem->rcItem.right - lpDrawItem->rcItem.left,
                           lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top,
                           hdcMem,
                           0,
                           0,
                           SRCCOPY);
                    DeleteDC(hdcMem);
                }
            }
            return TRUE;
        }

        case WM_NOTIFY:
        {
            NMHDR *nmhdr = (NMHDR *)lParam;

            if (nmhdr->idFrom == IDC_ROSHOMEPAGE_LINK && nmhdr->code == NM_CLICK)
            {
                PNMLINK nml = (PNMLINK)nmhdr;

                ShellExecuteW(hwndDlg, L"open", nml->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
            }
            break;
        }

    }

    return FALSE;
}
