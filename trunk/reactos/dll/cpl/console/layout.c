/*
 * PROJECT:         ReactOS Console Configuration DLL
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            dll/win32/console/layout.c
 * PURPOSE:         displays layout dialog
 * PROGRAMMERS:     Johannes Anderwald (johannes.anderwald@reactos.org)
 */

#include "console.h"

#define NDEBUG
#include <debug.h>

const TCHAR szPreviewText[] =
    _T("C:\\ReactOS> dir                       \n") \
    _T("SYSTEM       <DIR>      13-04-15  5:00a\n") \
    _T("SYSTEM32     <DIR>      13-04-15  5:00a\n") \
    _T("readme   txt       1739 13-04-15  5:00a\n") \
    _T("explorer exe    3329536 13-04-15  5:00a\n") \
    _T("vgafonts cab      18736 13-04-15  5:00a\n") \
    _T("setuplog txt        313 13-04-15  5:00a\n") \
    _T("win      ini       7005 13-04-15  5:00a\n");


VOID
PaintConsole(LPDRAWITEMSTRUCT drawItem,
             PCONSOLE_PROPS pConInfo)
{
    PGUI_CONSOLE_INFO GuiInfo = pConInfo->TerminalInfo.TermInfo;
    HBRUSH hBrush;
    RECT cRect, fRect;
    DWORD startx, starty;
    DWORD endx, endy;
    DWORD sizex, sizey;

    FillRect(drawItem->hDC, &drawItem->rcItem, GetSysColorBrush(COLOR_BACKGROUND));

    sizex = drawItem->rcItem.right - drawItem->rcItem.left;
    sizey = drawItem->rcItem.bottom - drawItem->rcItem.top;

    if ( GuiInfo->WindowOrigin.x == MAXDWORD &&
         GuiInfo->WindowOrigin.y == MAXDWORD )
    {
        startx = sizex / 3;
        starty = sizey / 3;
    }
    else
    {
        // TODO:
        // Calculate pos correctly when console centered
        startx = sizex / 3;
        starty = sizey / 3;
    }

    // TODO:
    // Strech console when bold fonts are selected
    endx = drawItem->rcItem.right - startx + 15;
    endy = starty + sizey / 3;

    /* Draw console size */
    SetRect(&cRect, startx, starty, endx, endy);
    FillRect(drawItem->hDC, &cRect, GetSysColorBrush(COLOR_WINDOWFRAME));

    /* Draw console border */
    SetRect(&fRect, startx + 1, starty + 1, cRect.right - 1, cRect.bottom - 1);
    FrameRect(drawItem->hDC, &fRect, GetSysColorBrush(COLOR_ACTIVEBORDER));

    /* Draw left box */
    SetRect(&fRect, startx + 3, starty + 3, startx + 5, starty + 5);
    FillRect(drawItem->hDC, &fRect, GetSysColorBrush(COLOR_ACTIVEBORDER));

    /* Draw window title */
    SetRect(&fRect, startx + 7, starty + 3, cRect.right - 9, starty + 5);
    FillRect(drawItem->hDC, &fRect, GetSysColorBrush(COLOR_ACTIVECAPTION));

    /* Draw first right box */
    SetRect(&fRect, fRect.right + 1, starty + 3, fRect.right + 3, starty + 5);
    FillRect(drawItem->hDC, &fRect, GetSysColorBrush(COLOR_ACTIVEBORDER));

    /* Draw second right box */
    SetRect(&fRect, fRect.right + 1, starty + 3, fRect.right + 3, starty + 5);
    FillRect(drawItem->hDC, &fRect, GetSysColorBrush(COLOR_ACTIVEBORDER));

    /* Draw scrollbar */
    SetRect(&fRect, cRect.right - 5, fRect.bottom + 1, cRect.right - 3, cRect.bottom - 3);
    FillRect(drawItem->hDC, &fRect, GetSysColorBrush(COLOR_SCROLLBAR));

    /* Draw console background */
    hBrush = CreateSolidBrush(pConInfo->ci.Colors[BkgdAttribFromAttrib(pConInfo->ci.ScreenAttrib)]);
    SetRect(&fRect, startx + 3, starty + 6, cRect.right - 6, cRect.bottom - 3);
    FillRect(drawItem->hDC, &fRect, hBrush);
    DeleteObject((HGDIOBJ)hBrush);
}

VOID PaintText(LPDRAWITEMSTRUCT drawItem,
               PCONSOLE_PROPS pConInfo)
{
    COLORREF pbkColor, ptColor;
    COLORREF nbkColor, ntColor;
    HBRUSH hBrush = NULL;

    if (drawItem->CtlID == IDC_STATIC_SCREEN_COLOR)
    {
        nbkColor = pConInfo->ci.Colors[BkgdAttribFromAttrib(pConInfo->ci.ScreenAttrib)];
        hBrush = CreateSolidBrush(nbkColor);
        ntColor = pConInfo->ci.Colors[TextAttribFromAttrib(pConInfo->ci.ScreenAttrib)];
    }
    else if (drawItem->CtlID == IDC_STATIC_POPUP_COLOR)
    {
        nbkColor = pConInfo->ci.Colors[BkgdAttribFromAttrib(pConInfo->ci.PopupAttrib)];
        hBrush = CreateSolidBrush(nbkColor);
        ntColor = pConInfo->ci.Colors[TextAttribFromAttrib(pConInfo->ci.PopupAttrib)];
    }

    if (!hBrush)
    {
        return;
    }

    FillRect(drawItem->hDC, &drawItem->rcItem, hBrush);

    ptColor = SetTextColor(drawItem->hDC, ntColor);
    pbkColor = SetBkColor(drawItem->hDC, nbkColor);
    DrawText(drawItem->hDC, szPreviewText, _tcslen(szPreviewText), &drawItem->rcItem, 0);
    SetTextColor(drawItem->hDC, ptColor);
    SetBkColor(drawItem->hDC, pbkColor);
    DeleteObject((HGDIOBJ)hBrush);
}

INT_PTR
CALLBACK
LayoutProc(HWND hwndDlg,
           UINT uMsg,
           WPARAM wParam,
           LPARAM lParam)
{
    LPNMUPDOWN lpnmud;
    LPPSHNOTIFY lppsn;
    PCONSOLE_PROPS pConInfo = (PCONSOLE_PROPS)GetWindowLongPtr(hwndDlg, DWLP_USER);
    PGUI_CONSOLE_INFO GuiInfo = (pConInfo ? pConInfo->TerminalInfo.TermInfo : NULL);

    UNREFERENCED_PARAMETER(hwndDlg);
    UNREFERENCED_PARAMETER(wParam);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            DWORD xres, yres;
            HDC hDC;
            pConInfo = (PCONSOLE_PROPS)((LPPROPSHEETPAGE)lParam)->lParam;
            GuiInfo  = pConInfo->TerminalInfo.TermInfo;
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pConInfo);

            SendMessage(GetDlgItem(hwndDlg, IDC_UPDOWN_SCREEN_BUFFER_HEIGHT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(9999, 1));
            SendMessage(GetDlgItem(hwndDlg, IDC_UPDOWN_SCREEN_BUFFER_WIDTH), UDM_SETRANGE, 0, (LPARAM)MAKELONG(9999, 1));
            SendMessage(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_SIZE_HEIGHT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(9999, 1));
            SendMessage(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_SIZE_WIDTH), UDM_SETRANGE, 0, (LPARAM)MAKELONG(9999, 1));

            SetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_HEIGHT, pConInfo->ci.ScreenBufferSize.Y, FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_WIDTH, pConInfo->ci.ScreenBufferSize.X, FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_HEIGHT, pConInfo->ci.ConsoleSize.Y, FALSE);
            SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_WIDTH, pConInfo->ci.ConsoleSize.X, FALSE);

            hDC = GetDC(NULL);
            xres = GetDeviceCaps(hDC, HORZRES);
            yres = GetDeviceCaps(hDC, VERTRES);
            SendMessage(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_LEFT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(xres, 0));
            SendMessage(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_TOP), UDM_SETRANGE, 0, (LPARAM)MAKELONG(yres, 0));

            if ( GuiInfo->WindowOrigin.x != MAXDWORD &&
                 GuiInfo->WindowOrigin.y != MAXDWORD )
            {
                SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT, GuiInfo->WindowOrigin.x, FALSE);
                SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_TOP, GuiInfo->WindowOrigin.y, FALSE);
            }
            else
            {
                // FIXME: Calculate window pos from xres, yres
                SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT, 88, FALSE);
                SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_TOP, 88, FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_WINDOW_POS_TOP), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_LEFT), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_TOP), FALSE);
                SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_SYSTEM_POS_WINDOW), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
            }

            return TRUE;
        }
        case WM_DRAWITEM:
        {
            PaintConsole((LPDRAWITEMSTRUCT)lParam, pConInfo);
            return TRUE;
        }
        case WM_NOTIFY:
        {
            lpnmud = (LPNMUPDOWN) lParam;
            lppsn = (LPPSHNOTIFY) lParam;

            if (lppsn->hdr.code == UDN_DELTAPOS)
            {
                DWORD wheight, wwidth;
                DWORD sheight, swidth;
                DWORD left, top;

                if (lppsn->hdr.idFrom == IDC_UPDOWN_WINDOW_SIZE_WIDTH)
                {
                    wwidth = lpnmud->iPos + lpnmud->iDelta;
                }
                else
                {
                    wwidth = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_WIDTH, NULL, FALSE);
                }

                if (lppsn->hdr.idFrom == IDC_UPDOWN_WINDOW_SIZE_HEIGHT)
                {
                    wheight = lpnmud->iPos + lpnmud->iDelta;
                }
                else
                {
                    wheight = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_HEIGHT, NULL, FALSE);
                }

                if (lppsn->hdr.idFrom == IDC_UPDOWN_SCREEN_BUFFER_WIDTH)
                {
                    swidth = lpnmud->iPos + lpnmud->iDelta;
                }
                else
                {
                    swidth = GetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_WIDTH, NULL, FALSE);
                }

                if (lppsn->hdr.idFrom == IDC_UPDOWN_SCREEN_BUFFER_HEIGHT)
                {
                    sheight = lpnmud->iPos + lpnmud->iDelta;
                }
                else
                {
                    sheight = GetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_HEIGHT, NULL, FALSE);
                }

                if (lppsn->hdr.idFrom == IDC_UPDOWN_WINDOW_POS_LEFT)
                {
                    left = lpnmud->iPos + lpnmud->iDelta;
                }
                else
                {
                    left = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT, NULL, FALSE);
                }

                if (lppsn->hdr.idFrom == IDC_UPDOWN_WINDOW_POS_TOP)
                {
                    top = lpnmud->iPos + lpnmud->iDelta;
                }
                else
                {
                    top = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_TOP, NULL, FALSE);
                }

                if (lppsn->hdr.idFrom == IDC_UPDOWN_WINDOW_SIZE_WIDTH || lppsn->hdr.idFrom == IDC_UPDOWN_WINDOW_SIZE_HEIGHT)
                {
                    /* Automatically adjust screen buffer size when window size enlarges */
                    if (wwidth >= swidth)
                    {
                        SetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_WIDTH, wwidth, TRUE);
                        swidth = wwidth;
                    }

                    if (wheight >= sheight)
                    {
                        SetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_HEIGHT, wheight, TRUE);
                        sheight = wheight;
                    }
                }
                swidth  = min(max(swidth , 1), 0xFFFF);
                sheight = min(max(sheight, 1), 0xFFFF);

                if (lppsn->hdr.idFrom == IDC_UPDOWN_SCREEN_BUFFER_WIDTH || lppsn->hdr.idFrom == IDC_UPDOWN_SCREEN_BUFFER_HEIGHT)
                {
                    /* Automatically adjust window size when screen buffer decreases */
                    if (wwidth > swidth)
                    {
                        SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_WIDTH, swidth, TRUE);
                        wwidth = swidth;
                    }

                    if (wheight > sheight)
                    {
                        SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_HEIGHT, sheight, TRUE);
                        wheight = sheight;
                    }
                }

                pConInfo->ci.ScreenBufferSize.X = (SHORT)swidth;
                pConInfo->ci.ScreenBufferSize.Y = (SHORT)sheight;
                pConInfo->ci.ConsoleSize.X = (SHORT)wwidth;
                pConInfo->ci.ConsoleSize.Y = (SHORT)wheight;
                GuiInfo->WindowOrigin.x = left;
                GuiInfo->WindowOrigin.y = top;
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_EDIT_SCREEN_BUFFER_WIDTH:
                case IDC_EDIT_SCREEN_BUFFER_HEIGHT:
                case IDC_EDIT_WINDOW_SIZE_WIDTH:
                case IDC_UPDOWN_WINDOW_SIZE_HEIGHT:
                case IDC_EDIT_WINDOW_POS_LEFT:
                case IDC_EDIT_WINDOW_POS_TOP:
                {
                    if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        DWORD wheight, wwidth;
                        DWORD sheight, swidth;
                        DWORD left, top;

                        wwidth  = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_WIDTH, NULL, FALSE);
                        wheight = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_HEIGHT, NULL, FALSE);
                        swidth  = GetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_WIDTH, NULL, FALSE);
                        sheight = GetDlgItemInt(hwndDlg, IDC_EDIT_SCREEN_BUFFER_HEIGHT, NULL, FALSE);
                        left    = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT, NULL, FALSE);
                        top     = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_TOP, NULL, FALSE);

                        swidth  = min(max(swidth , 1), 0xFFFF);
                        sheight = min(max(sheight, 1), 0xFFFF);

                        /* Automatically adjust window size when screen buffer decreases */
                        if (wwidth > swidth)
                        {
                            SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_WIDTH, swidth, TRUE);
                            wwidth = swidth;
                        }

                        if (wheight > sheight)
                        {
                            SetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_SIZE_HEIGHT, sheight, TRUE);
                            wheight = sheight;
                        }

                        pConInfo->ci.ScreenBufferSize.X = (SHORT)swidth;
                        pConInfo->ci.ScreenBufferSize.Y = (SHORT)sheight;
                        pConInfo->ci.ConsoleSize.X = (SHORT)wwidth;
                        pConInfo->ci.ConsoleSize.Y = (SHORT)wheight;
                        GuiInfo->WindowOrigin.x = left;
                        GuiInfo->WindowOrigin.y = top;
                        PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                    }
                    break;
                }

                case IDC_CHECK_SYSTEM_POS_WINDOW:
                {
                    LONG res = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
                    if (res == BST_CHECKED)
                    {
                        ULONG left, top;

                        left = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT, NULL, FALSE);
                        top = GetDlgItemInt(hwndDlg, IDC_EDIT_WINDOW_POS_TOP, NULL, FALSE);
                        GuiInfo->WindowOrigin.x = left;
                        GuiInfo->WindowOrigin.y = top;
                        SendMessage((HWND)lParam, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT), TRUE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_WINDOW_POS_TOP), TRUE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_LEFT), TRUE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_TOP), TRUE);
                    }
                    else if (res == BST_UNCHECKED)
                    {
                        GuiInfo->WindowOrigin.x = UINT_MAX;
                        GuiInfo->WindowOrigin.y = UINT_MAX;
                        SendMessage((HWND)lParam, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_WINDOW_POS_LEFT), FALSE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_WINDOW_POS_TOP), FALSE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_LEFT), FALSE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_UPDOWN_WINDOW_POS_TOP), FALSE);
                    }
                }
            }
        }
        default:
            break;
    }

    return FALSE;
}
