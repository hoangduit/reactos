/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS uxtheme.dll
 * FILE:            dll/win32/uxtheme/nonclient.c
 * PURPOSE:         uxtheme non client area management
 * PROGRAMMER:      Giannis Adamopoulos
 */
 
#include "uxthemep.h"

HFONT hMenuFont = NULL;
HFONT hMenuFontBold = NULL;

void InitMenuFont(VOID)
{
  NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(NONCLIENTMETRICS); 

    if(!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
    {
      return ;
    }

    hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);

    ncm.lfMenuFont.lfWeight = max(ncm.lfMenuFont.lfWeight + 300, 1000);
    hMenuFontBold = CreateFontIndirect(&ncm.lfMenuFont);

}

static BOOL 
IsWindowActive(HWND hWnd, DWORD ExStyle)
{
    BOOL ret;

    if (ExStyle & WS_EX_MDICHILD)
    {
        ret = IsChild(GetForegroundWindow(), hWnd);
        if (ret)
            ret = (hWnd == (HWND)SendMessageW(GetParent(hWnd), WM_MDIGETACTIVE, 0, 0));
    }
    else
    {
        ret = (GetForegroundWindow() == hWnd);
    }

    return ret;
}

static BOOL 
UserHasWindowEdge(DWORD Style, DWORD ExStyle)
{
    if (Style & WS_MINIMIZE)
        return TRUE;
    if (ExStyle & WS_EX_DLGMODALFRAME)
        return TRUE;
    if (ExStyle & WS_EX_STATICEDGE)
        return FALSE;
    if (Style & WS_THICKFRAME)
        return TRUE;
    if (Style == WS_DLGFRAME || Style == WS_CAPTION)
        return TRUE;
   return FALSE;
}

HICON
UserGetWindowIcon(HWND hwnd)
{
    HICON hIcon = 0;

    SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL2, 0, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);

    if (!hIcon)
        SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);

    if (!hIcon)
        SendMessageTimeout(hwnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);

    if (!hIcon)
        hIcon = (HICON)GetClassLong(hwnd, GCL_HICONSM);

    if (!hIcon)
        hIcon = (HICON)GetClassLong(hwnd, GCL_HICON);

    if(!hIcon)
        hIcon = LoadIconW(NULL, (LPCWSTR)IDI_WINLOGO);

    return hIcon;
}

WCHAR *UserGetWindowCaption(HWND hwnd)
{
    INT len = 512;
    WCHAR *text;
    text = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len  * sizeof(WCHAR));
    if (text) InternalGetWindowText(hwnd, text, len);
    return text;
}

static void 
ThemeDrawTitle(PDRAW_CONTEXT context, RECT* prcCurrent)
{

}

HRESULT WINAPI ThemeDrawCaptionText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                             LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,
                             DWORD dwTextFlags2, const RECT *pRect, BOOL Active)
{
    HRESULT hr;
    HFONT hFont = NULL;
    HGDIOBJ oldFont = NULL;
    LOGFONTW logfont;
    COLORREF textColor;
    COLORREF oldTextColor;
    int oldBkMode;
    RECT rt;
    
    hr = GetThemeSysFont(0,TMT_CAPTIONFONT,&logfont);

    if(SUCCEEDED(hr)) {
        hFont = CreateFontIndirectW(&logfont);
    }
    CopyRect(&rt, pRect);
    if(hFont)
        oldFont = SelectObject(hdc, hFont);
        
    if(dwTextFlags2 & DTT_GRAYED)
        textColor = GetSysColor(COLOR_GRAYTEXT);
    else if (!Active)
        textColor = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
    else
        textColor = GetSysColor(COLOR_CAPTIONTEXT);
    
    oldTextColor = SetTextColor(hdc, textColor);
    oldBkMode = SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, pszText, iCharCount, &rt, dwTextFlags);
    SetBkMode(hdc, oldBkMode);
    SetTextColor(hdc, oldTextColor);

    if(hFont) {
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
    return S_OK;
}

void
ThemeInitDrawContext(PDRAW_CONTEXT pcontext,
                     HWND hWnd,
                     HRGN hRgn)
{
    pcontext->wi.cbSize = sizeof(pcontext->wi);
    GetWindowInfo(hWnd, &pcontext->wi);
    pcontext->hWnd = hWnd;
    pcontext->Active = IsWindowActive(hWnd, pcontext->wi.dwExStyle);
    pcontext->theme = MSSTYLES_OpenThemeClass(ActiveThemeFile, NULL, L"WINDOW");
    pcontext->scrolltheme = MSSTYLES_OpenThemeClass(ActiveThemeFile, NULL, L"SCROLLBAR");

    pcontext->CaptionHeight = pcontext->wi.cyWindowBorders;
    pcontext->CaptionHeight += GetSystemMetrics(pcontext->wi.dwExStyle & WS_EX_TOOLWINDOW ? SM_CYSMCAPTION : SM_CYCAPTION );

    if(hRgn <= (HRGN)1)
    {
        hRgn = CreateRectRgnIndirect(&pcontext->wi.rcWindow);
    }
    pcontext->hRgn = hRgn;

    pcontext->hDC = GetDCEx(hWnd, hRgn, DCX_WINDOW | DCX_INTERSECTRGN | DCX_USESTYLE | DCX_KEEPCLIPRGN);
}

void
ThemeCleanupDrawContext(PDRAW_CONTEXT pcontext)
{
    ReleaseDC(pcontext->hWnd ,pcontext->hDC);

    CloseThemeData (pcontext->theme);
    CloseThemeData (pcontext->scrolltheme);

    if(pcontext->hRgn != NULL)
    {
        DeleteObject(pcontext->hRgn);
    }
}

static void 
ThemeStartBufferedPaint(PDRAW_CONTEXT pcontext, int cx, int cy)
{
    HBITMAP hbmp;

    pcontext->hDCScreen = pcontext->hDC;
    pcontext->hDC = CreateCompatibleDC(pcontext->hDCScreen);
    hbmp = CreateCompatibleBitmap(pcontext->hDCScreen, cx, cy); 
    pcontext->hbmpOld = (HBITMAP)SelectObject(pcontext->hDC, hbmp);
}

static void
ThemeEndBufferedPaint(PDRAW_CONTEXT pcontext, int x, int y, int cx, int cy)
{
    HBITMAP hbmp;
    BitBlt(pcontext->hDCScreen, 0, 0, cx, cy, pcontext->hDC, x, y, SRCCOPY);
    hbmp = (HBITMAP) SelectObject(pcontext->hDC, pcontext->hbmpOld);
    DeleteObject(pcontext->hDC);
    DeleteObject(hbmp);

    pcontext->hDC = pcontext->hDCScreen;
}

static void 
ThemeDrawCaptionButton(PDRAW_CONTEXT pcontext, 
                       RECT* prcCurrent, 
                       CAPTIONBUTTON buttonId, 
                       INT iStateId)
{
    RECT rcPart;
    INT ButtonWidth, ButtonHeight, iPartId;
    
    ButtonHeight = GetSystemMetrics( pcontext->wi.dwExStyle & WS_EX_TOOLWINDOW ? SM_CYSMSIZE : SM_CYSIZE);
    ButtonWidth = GetSystemMetrics( pcontext->wi.dwExStyle & WS_EX_TOOLWINDOW ? SM_CXSMSIZE : SM_CXSIZE);

    switch(buttonId)
    {
    case CLOSEBUTTON:
        iPartId = pcontext->wi.dwExStyle & WS_EX_TOOLWINDOW ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON;
        break;

    case MAXBUTTON:
        if (!(pcontext->wi.dwStyle & WS_MAXIMIZEBOX))
        {
            if (!(pcontext->wi.dwStyle & WS_MINIMIZEBOX))
                return;
            else
                iStateId = BUTTON_DISABLED;
        }

        iPartId = pcontext->wi.dwStyle & WS_MAXIMIZE ? WP_RESTOREBUTTON : WP_MAXBUTTON;
        break;

    case MINBUTTON:
        if (!(pcontext->wi.dwStyle & WS_MINIMIZEBOX))
        {
            if (!(pcontext->wi.dwStyle & WS_MAXIMIZEBOX))
                return;
            else
                iStateId = BUTTON_DISABLED;
        }
 
        iPartId = WP_MINBUTTON;
        break;

    default:
        //FIXME: Implement Help Button 
        return;
    }

    ButtonHeight -= 4;
    ButtonWidth -= 4;

    /* Calculate the position */
    rcPart.top = prcCurrent->top;
    rcPart.right = prcCurrent->right;
    rcPart.bottom = rcPart.top + ButtonHeight ;
    rcPart.left = rcPart.right - ButtonWidth ;
    prcCurrent->right -= ButtonWidth + BUTTON_GAP_SIZE;

    DrawThemeBackground(pcontext->theme, pcontext->hDC, iPartId, iStateId, &rcPart, NULL);
}

static DWORD
ThemeGetButtonState(DWORD htCurrect, DWORD htHot, DWORD htDown, BOOL Active)
{
    if (htHot == htCurrect)
        return BUTTON_HOT;
    if (!Active)
        return BUTTON_INACTIVE;
    if (htDown == htCurrect)
        return BUTTON_PRESSED;

    return BUTTON_NORMAL;
}

/* Used only from mouse event handlers */
static void 
ThemeDrawCaptionButtons(PDRAW_CONTEXT pcontext, DWORD htHot, DWORD htDown)
{
    RECT rcCurrent;

    /* Calculate the area of the caption */
    rcCurrent.top = rcCurrent.left = 0;
    rcCurrent.right = pcontext->wi.rcWindow.right - pcontext->wi.rcWindow.left;
    rcCurrent.bottom = pcontext->CaptionHeight;
    
    /* Add a padding around the objects of the caption */
    InflateRect(&rcCurrent, -(int)pcontext->wi.cyWindowBorders-BUTTON_GAP_SIZE, 
                            -(int)pcontext->wi.cyWindowBorders-BUTTON_GAP_SIZE);

    /* Draw the buttons */
    ThemeDrawCaptionButton(pcontext, &rcCurrent, CLOSEBUTTON, 
                           ThemeGetButtonState(HTCLOSE, htHot, htDown, pcontext->Active));
    ThemeDrawCaptionButton(pcontext, &rcCurrent, MAXBUTTON,  
                           ThemeGetButtonState(HTMAXBUTTON, htHot, htDown, pcontext->Active));
    ThemeDrawCaptionButton(pcontext, &rcCurrent, MINBUTTON,
                           ThemeGetButtonState(HTMINBUTTON, htHot, htDown, pcontext->Active));
    ThemeDrawCaptionButton(pcontext, &rcCurrent, HELPBUTTON,
                           ThemeGetButtonState(HTHELP, htHot, htDown, pcontext->Active));
}

/* Used from WM_NCPAINT and WM_NCACTIVATE handlers*/
static void 
ThemeDrawCaption(PDRAW_CONTEXT pcontext, RECT* prcCurrent)
{
    RECT rcPart;
    int iPart, iState;
    HICON hIcon;
    WCHAR *CaptionText;

    hIcon = UserGetWindowIcon(pcontext->hWnd);
    CaptionText = UserGetWindowCaption(pcontext->hWnd);

    /* Get the caption part and state id */
    if (pcontext->wi.dwExStyle & WS_EX_TOOLWINDOW)
        iPart = WP_SMALLCAPTION;
    else if (pcontext->wi.dwStyle & WS_MAXIMIZE)
        iPart = WP_MAXCAPTION;
    else
        iPart = WP_CAPTION;

    iState = pcontext->Active ? FS_ACTIVE : FS_INACTIVE;

    /* Draw the caption background*/
    rcPart = *prcCurrent;
    rcPart.bottom = rcPart.top + pcontext->CaptionHeight;
    prcCurrent->top = rcPart.bottom;
    DrawThemeBackground(pcontext->theme, pcontext->hDC,iPart,iState,&rcPart,NULL);

    /* Add a padding around the objects of the caption */
    InflateRect(&rcPart, -(int)pcontext->wi.cyWindowBorders-BUTTON_GAP_SIZE, 
                         -(int)pcontext->wi.cyWindowBorders-BUTTON_GAP_SIZE);

    /* Draw the caption buttons */
    if (pcontext->wi.dwStyle & WS_SYSMENU)
    {
        iState = pcontext->Active ? BUTTON_NORMAL : BUTTON_INACTIVE;

        ThemeDrawCaptionButton(pcontext, &rcPart, CLOSEBUTTON, iState);
        ThemeDrawCaptionButton(pcontext, &rcPart, MAXBUTTON, iState);
        ThemeDrawCaptionButton(pcontext, &rcPart, MINBUTTON, iState);
        ThemeDrawCaptionButton(pcontext, &rcPart, HELPBUTTON, iState);
    }
    
    rcPart.top += 3 ;

    /* Draw the icon */
    if(hIcon && !(pcontext->wi.dwExStyle & WS_EX_TOOLWINDOW))
    {
        int IconHeight = GetSystemMetrics(SM_CYSMICON);
        int IconWidth = GetSystemMetrics(SM_CXSMICON);
        DrawIconEx(pcontext->hDC, rcPart.left, rcPart.top , hIcon, IconWidth, IconHeight, 0, NULL, DI_NORMAL);
        rcPart.left += IconWidth + 4;
    }

    rcPart.right -= 4;

    /* Draw the caption */
    if(CaptionText)
    {
        /*FIXME: Use DrawThemeTextEx*/
        ThemeDrawCaptionText(pcontext->theme, 
                             pcontext->hDC, 
                             iPart,
                             iState, 
                             CaptionText, 
                             lstrlenW(CaptionText), 
                             DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, 
                             0, 
                             &rcPart , 
                             pcontext->Active);
        HeapFree(GetProcessHeap(), 0, CaptionText);
    }
}

static void 
ThemeDrawBorders(PDRAW_CONTEXT pcontext, RECT* prcCurrent)
{
    RECT rcPart;
    int iState = pcontext->Active ? FS_ACTIVE : FS_INACTIVE;

    /* Draw the bottom border */
    rcPart = *prcCurrent;
    rcPart.top = rcPart.bottom - pcontext->wi.cyWindowBorders;
    prcCurrent->bottom = rcPart.top;
    DrawThemeBackground(pcontext->theme, pcontext->hDC, WP_FRAMEBOTTOM, iState, &rcPart, NULL);

    /* Draw the left border */
    rcPart = *prcCurrent;
    rcPart.right = rcPart.left + pcontext->wi.cxWindowBorders ;
    prcCurrent->left = rcPart.right;
    DrawThemeBackground(pcontext->theme, pcontext->hDC,WP_FRAMELEFT, iState, &rcPart, NULL);

    /* Draw the right border */
    rcPart = *prcCurrent;
    rcPart.left = rcPart.right - pcontext->wi.cxWindowBorders;
    prcCurrent->right = rcPart.left;
    DrawThemeBackground(pcontext->theme, pcontext->hDC,WP_FRAMERIGHT, iState, &rcPart, NULL);
}

static void 
DrawClassicFrame(PDRAW_CONTEXT context, RECT* prcCurrent)
{
    /* Draw outer edge */
    if (UserHasWindowEdge(context->wi.dwStyle, context->wi.dwExStyle))
    {
        DrawEdge(context->hDC, prcCurrent, EDGE_RAISED, BF_RECT | BF_ADJUST);
    } 
    else if (context->wi.dwExStyle & WS_EX_STATICEDGE)
    {
        DrawEdge(context->hDC, prcCurrent, BDR_SUNKENINNER, BF_RECT | BF_ADJUST | BF_FLAT);
    }

    /* Firstly the "thick" frame */
    if ((context->wi.dwStyle & WS_THICKFRAME) && !(context->wi.dwStyle & WS_MINIMIZE))
    {
        INT Width =
            (GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXDLGFRAME)) *
            GetSystemMetrics(SM_CXBORDER);
        INT Height =
            (GetSystemMetrics(SM_CYFRAME) - GetSystemMetrics(SM_CYDLGFRAME)) *
            GetSystemMetrics(SM_CYBORDER);

        SelectObject(context->hDC, GetSysColorBrush(
                     context->Active ? COLOR_ACTIVEBORDER : COLOR_INACTIVEBORDER));

        /* Draw frame */
        PatBlt(context->hDC, prcCurrent->left, prcCurrent->top, 
               prcCurrent->right - prcCurrent->left, Height, PATCOPY);
        PatBlt(context->hDC, prcCurrent->left, prcCurrent->top, 
               Width, prcCurrent->bottom - prcCurrent->top, PATCOPY);
        PatBlt(context->hDC, prcCurrent->left, prcCurrent->bottom, 
               prcCurrent->right - prcCurrent->left, -Height, PATCOPY);
        PatBlt(context->hDC, prcCurrent->right, prcCurrent->top, 
               -Width, prcCurrent->bottom - prcCurrent->top, PATCOPY);

        InflateRect(prcCurrent, -Width, -Height);
    }

    /* Now the other bit of the frame */
    if (context->wi.dwStyle & (WS_DLGFRAME | WS_BORDER) || context->wi.dwExStyle & WS_EX_DLGMODALFRAME)
    {
        INT Width = GetSystemMetrics(SM_CXBORDER);
        INT Height = GetSystemMetrics(SM_CYBORDER);

        SelectObject(context->hDC, GetSysColorBrush(
            (context->wi.dwExStyle & (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE)) ? COLOR_3DFACE :
            (context->wi.dwExStyle & WS_EX_STATICEDGE) ? COLOR_WINDOWFRAME :
            (context->wi.dwStyle & (WS_DLGFRAME | WS_THICKFRAME)) ? COLOR_3DFACE :
            COLOR_WINDOWFRAME));

        /* Draw frame */
        PatBlt(context->hDC, prcCurrent->left, prcCurrent->top, 
               prcCurrent->right - prcCurrent->left, Height, PATCOPY);
        PatBlt(context->hDC, prcCurrent->left, prcCurrent->top, 
               Width, prcCurrent->bottom - prcCurrent->top, PATCOPY);
        PatBlt(context->hDC, prcCurrent->left, prcCurrent->bottom, 
               prcCurrent->right - prcCurrent->left, -Height, PATCOPY);
        PatBlt(context->hDC, prcCurrent->right, prcCurrent->top, 
              -Width, prcCurrent->bottom - prcCurrent->top, PATCOPY);

        InflateRect(prcCurrent, -Width, -Height);
    }

    if (context->wi.dwExStyle & WS_EX_CLIENTEDGE)
    {
        DrawEdge(context->hDC, prcCurrent, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
    }
}

static void
ThemeDrawMenuItem(PDRAW_CONTEXT pcontext, HMENU Menu, int imenu)
{
    PWCHAR Text;
    BOOL flat_menu = FALSE;
    MENUITEMINFOW Item;
    RECT Rect,rcCalc;
    WCHAR wstrItemText[20];
    register int i = 0;
    HFONT FontOld = NULL;
    UINT uFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;

    Item.cbSize = sizeof(MENUITEMINFOW);
    Item.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_STRING;
    Item.dwTypeData = wstrItemText;
    Item.cch = 20;
    if (!GetMenuItemInfoW(Menu, imenu, TRUE, &Item))
        return;

    if(Item.fType & MF_SEPARATOR)
        return;

    if(Item.cch >= 20)
    {
        Item.cch++;
        Item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, Item.cch  * sizeof(WCHAR));
        Item.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_STRING;
        GetMenuItemInfoW(Menu, imenu, TRUE, &Item);
    }

    if(Item.cch == 0)
        return;

    flat_menu = GetThemeSysBool(pcontext->theme, TMT_FLATMENUS);

    GetMenuItemRect(pcontext->hWnd, Menu, imenu, &Rect);

#ifdef __REACTOS__
    OffsetRect(&Rect, -pcontext->wi.rcClient.left, -pcontext->wi.rcClient.top);
#else
    OffsetRect(&Rect, -pcontext->wi.rcWindow.left, -pcontext->wi.rcWindow.top);
#endif
    
    SetBkColor(pcontext->hDC, GetSysColor(flat_menu ? COLOR_MENUBAR : COLOR_MENU));
    SetTextColor(pcontext->hDC, GetSysColor(Item.fState & MF_GRAYED ? COLOR_GRAYTEXT : COLOR_MENUTEXT));

    if (0 != (Item.fState & MFS_DEFAULT))
    {
        FontOld = (HFONT)SelectObject(pcontext->hDC, hMenuFontBold);
    }

    Rect.left += MENU_BAR_ITEMS_SPACE / 2;
    Rect.right -= MENU_BAR_ITEMS_SPACE / 2;

    Text = (PWCHAR) Item.dwTypeData;
    if(Text)
    {
        for (i = 0; L'\0' != Text[i]; i++)
        {
            if (L'\t' == Text[i] || L'\b' == Text[i])
            {
                break;
            }
        }
    }

    SetBkMode(pcontext->hDC, OPAQUE);

    if (0 != (Item.fState & MF_GRAYED))
    {
        if (0 == (Item.fState & MF_HILITE))
        {
            ++Rect.left; ++Rect.top; ++Rect.right; ++Rect.bottom;
            SetTextColor(pcontext->hDC, RGB(0xff, 0xff, 0xff));
            DrawTextW(pcontext->hDC, Text, i, &Rect, uFormat);
            --Rect.left; --Rect.top; --Rect.right; --Rect.bottom;
        }
        SetTextColor(pcontext->hDC, RGB(0x80, 0x80, 0x80));
        SetBkMode(pcontext->hDC, TRANSPARENT);
    }
    
    DrawTextW(pcontext->hDC, Text, i, &Rect, uFormat);

    /* Exclude from the clip region the area drawn by DrawText */
    SetRect(&rcCalc, 0,0,0,0);
    DrawTextW(pcontext->hDC, Text, i, &rcCalc, uFormat | DT_CALCRECT);
    InflateRect( &Rect, 0, -(rcCalc.bottom+1)/2);
    ExcludeClipRect(pcontext->hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);

    if (NULL != FontOld)
    {
        SelectObject(pcontext->hDC, FontOld);
    }
}

void WINAPI
ThemeDrawMenuBar(PDRAW_CONTEXT pcontext, PRECT prcCurrent)
{
    HMENU Menu;
    MENUBARINFO MenuBarInfo;
    int i;
    HFONT FontOld = NULL;
    BOOL flat_menu;
    RECT Rect;
    HPEN oldPen ;

    if (!hMenuFont)
        InitMenuFont();

    flat_menu = GetThemeSysBool(pcontext->theme, TMT_FLATMENUS);

    MenuBarInfo.cbSize = sizeof(MENUBARINFO);
    if (! GetMenuBarInfo(pcontext->hWnd, OBJID_MENU, 0, &MenuBarInfo))
        return;

    Menu = GetMenu(pcontext->hWnd);
    if (GetMenuItemCount(Menu) == 0)
        return;

    Rect = MenuBarInfo.rcBar;
    OffsetRect(&Rect, -pcontext->wi.rcWindow.left, -pcontext->wi.rcWindow.top);

    /* Draw a line under the menu*/
    oldPen = (HPEN)SelectObject(pcontext->hDC, GetStockObject(DC_PEN));
    SetDCPenColor(pcontext->hDC, GetSysColor(COLOR_3DFACE));
    MoveToEx(pcontext->hDC, Rect.left, Rect.bottom, NULL);
    LineTo(pcontext->hDC, Rect.right, Rect.bottom);
    SelectObject(pcontext->hDC, oldPen);

    /* Draw menu items */
    FontOld = (HFONT)SelectObject(pcontext->hDC, hMenuFont);

    for (i = 0; i < GetMenuItemCount(Menu); i++)
    {
        ThemeDrawMenuItem(pcontext, Menu, i);
    }

    SelectObject(pcontext->hDC, FontOld);

    /* Fill the menu background area that isn't painted yet*/
    FillRect(pcontext->hDC, &Rect, GetSysColorBrush(flat_menu ? COLOR_MENUBAR : COLOR_MENU));
}

static void 
ThemePaintWindow(PDRAW_CONTEXT pcontext, RECT* prcCurrent, BOOL bDoDoubleBuffering)
{
    if(!(pcontext->wi.dwStyle & WS_VISIBLE))
        return;

    if(pcontext->wi.dwStyle & WS_MINIMIZE)
    {
        ThemeDrawTitle(pcontext, prcCurrent);
        return;
    }

    if((pcontext->wi.dwStyle & WS_CAPTION)==WS_CAPTION)
    {
        if (bDoDoubleBuffering)
            ThemeStartBufferedPaint(pcontext, prcCurrent->right, pcontext->CaptionHeight);
        ThemeDrawCaption(pcontext, prcCurrent);
        if (bDoDoubleBuffering)
            ThemeEndBufferedPaint(pcontext, 0, 0, prcCurrent->right, pcontext->CaptionHeight);
        ThemeDrawBorders(pcontext, prcCurrent);
    }
    else
    {
        DrawClassicFrame(pcontext, prcCurrent);
    }

    if(HAS_MENU(pcontext->hWnd, pcontext->wi.dwStyle))
        ThemeDrawMenuBar(pcontext, prcCurrent);
    
    if(pcontext->wi.dwStyle & WS_HSCROLL)
        ThemeDrawScrollBar(pcontext, SB_HORZ , NULL);

    if(pcontext->wi.dwStyle & WS_VSCROLL)
        ThemeDrawScrollBar(pcontext, SB_VERT, NULL);
}

/*
    Message handlers
 */

static LRESULT 
ThemeHandleNCPaint(HWND hWnd, HRGN hRgn)
{
    DRAW_CONTEXT context;
    RECT rcCurrent;

    ThemeInitDrawContext(&context, hWnd, hRgn);

    rcCurrent = context.wi.rcWindow;
    OffsetRect( &rcCurrent, -context.wi.rcWindow.left, -context.wi.rcWindow.top);

    ThemePaintWindow(&context, &rcCurrent, TRUE);
    ThemeCleanupDrawContext(&context);

    return 0;
}

static LRESULT 
ThemeHandleNcMouseMove(HWND hWnd, DWORD ht, POINT* pt)
{
    DRAW_CONTEXT context;
    TRACKMOUSEEVENT tme;
    DWORD style;
    PWND_CONTEXT pcontext;

    /* First of all check if we have something to do here */
    style = GetWindowLongW(hWnd, GWL_STYLE);
    if((style & (WS_CAPTION|WS_HSCROLL|WS_VSCROLL))==0)
        return 0;

    /* Get theme data for this window */
    pcontext = ThemeGetWndContext(hWnd);
    if (pcontext == NULL)
        return 0;

    /* Begin tracking in the non client area if we are not tracking yet */
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_QUERY;
    tme.hwndTrack  = hWnd;
    TrackMouseEvent(&tme);
    if (tme.dwFlags != (TME_LEAVE | TME_NONCLIENT))
    {
        tme.hwndTrack  = hWnd;
        tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
        TrackMouseEvent(&tme);
    }

    /* Dont do any drawing if the hit test wasn't changed */
    if (ht == pcontext->lastHitTest)
        return 0;

    ThemeInitDrawContext(&context, hWnd, 0);
    if (context.wi.dwStyle & WS_SYSMENU)
    {
        if (HT_ISBUTTON(ht) || HT_ISBUTTON(pcontext->lastHitTest))
            ThemeDrawCaptionButtons(&context, ht, 0);
    }

   if (context.wi.dwStyle & WS_HSCROLL)
   {
       if (ht == HTHSCROLL || pcontext->lastHitTest == HTHSCROLL)
           ThemeDrawScrollBar(&context, SB_HORZ , ht == HTHSCROLL ? pt : NULL);
   }

    if (context.wi.dwStyle & WS_VSCROLL)
    {
        if (ht == HTVSCROLL || pcontext->lastHitTest == HTVSCROLL)
            ThemeDrawScrollBar(&context, SB_VERT, ht == HTVSCROLL ? pt : NULL);
    }
    ThemeCleanupDrawContext(&context);

    pcontext->lastHitTest = ht;

    return 0;
}

static LRESULT 
ThemeHandleNcMouseLeave(HWND hWnd)
{
    DRAW_CONTEXT context;
    DWORD style;
    PWND_CONTEXT pWndContext;

    /* First of all check if we have something to do here */
    style = GetWindowLongW(hWnd, GWL_STYLE);
    if((style & (WS_CAPTION|WS_HSCROLL|WS_VSCROLL))==0)
        return 0;

    /* Get theme data for this window */
    pWndContext = ThemeGetWndContext(hWnd);
    if (pWndContext == NULL)
        return 0;

    ThemeInitDrawContext(&context, hWnd, 0);
    if (context.wi.dwStyle & WS_SYSMENU && HT_ISBUTTON(pWndContext->lastHitTest))
        ThemeDrawCaptionButtons(&context, 0, 0);

   if (context.wi.dwStyle & WS_HSCROLL && pWndContext->lastHitTest == HTHSCROLL)
        ThemeDrawScrollBar(&context, SB_HORZ,  NULL);

    if (context.wi.dwStyle & WS_VSCROLL && pWndContext->lastHitTest == HTVSCROLL)
        ThemeDrawScrollBar(&context, SB_VERT, NULL);

    ThemeCleanupDrawContext(&context);

    pWndContext->lastHitTest = HTNOWHERE;

    return 0;
}

static VOID
ThemeHandleButton(HWND hWnd, WPARAM wParam)
{
    MSG Msg;
    BOOL Pressed = TRUE;
    WPARAM SCMsg, ht;
    ULONG Style;
    DRAW_CONTEXT context;
    PWND_CONTEXT pWndContext;

    Style = GetWindowLongW(hWnd, GWL_STYLE);
    if (!((Style & WS_CAPTION) && (Style & WS_SYSMENU)))
        return ;

    switch (wParam)
    {
        case HTCLOSE:
            SCMsg = SC_CLOSE;
            break;
        case HTMINBUTTON:
            if (!(Style & WS_MINIMIZEBOX))
                return;
            SCMsg = ((Style & WS_MINIMIZE) ? SC_RESTORE : SC_MINIMIZE);
            break;
        case HTMAXBUTTON:
            if (!(Style & WS_MAXIMIZEBOX))
                return;
            SCMsg = ((Style & WS_MAXIMIZE) ? SC_RESTORE : SC_MAXIMIZE);
            break;
        default :
            return;
    }

    /* Get theme data for this window */
    pWndContext = ThemeGetWndContext(hWnd);
    if (pWndContext == NULL)
        return;

    ThemeInitDrawContext(&context, hWnd, 0);
    ThemeDrawCaptionButtons(&context, 0,  wParam);
    pWndContext->lastHitTest = wParam;

    SetCapture(hWnd);

    for (;;)
    {
        if (GetMessageW(&Msg, 0, WM_MOUSEFIRST, WM_MOUSELAST) <= 0)
            break;

        if (Msg.message == WM_LBUTTONUP)
            break;

        if (Msg.message != WM_MOUSEMOVE)
            continue;

        ht = SendMessage(hWnd, WM_NCHITTEST, 0, MAKELPARAM(Msg.pt.x, Msg.pt.y));
        Pressed = (ht == wParam);

        /* Only draw the buttons if the hit test changed */
        if (ht != pWndContext->lastHitTest &&
            (HT_ISBUTTON(ht) || HT_ISBUTTON(pWndContext->lastHitTest)))
        {
            ThemeDrawCaptionButtons(&context, 0, Pressed ? wParam: 0);
            pWndContext->lastHitTest = ht;
        }
    }

    ThemeDrawCaptionButtons(&context, ht, 0);
    ThemeCleanupDrawContext(&context);

    ReleaseCapture();

    if (Pressed)
        SendMessageW(hWnd, WM_SYSCOMMAND, SCMsg, 0);
}


static LRESULT
DefWndNCHitTest(HWND hWnd, POINT Point)
{
    RECT WindowRect;
    POINT ClientPoint;
    WINDOWINFO wi;

    GetWindowInfo(hWnd, &wi);

    if (!PtInRect(&wi.rcWindow, Point))
    {
        return HTNOWHERE;
    }
    WindowRect = wi.rcWindow;

    if (UserHasWindowEdge(wi.dwStyle, wi.dwExStyle))
    {
        LONG XSize, YSize;

        InflateRect(&WindowRect, -(int)wi.cxWindowBorders, -(int)wi.cyWindowBorders);
        XSize = GetSystemMetrics(SM_CXSIZE) * GetSystemMetrics(SM_CXBORDER);
        YSize = GetSystemMetrics(SM_CYSIZE) * GetSystemMetrics(SM_CYBORDER);
        if (!PtInRect(&WindowRect, Point))
        {
            BOOL ThickFrame;

            ThickFrame = (wi.dwStyle & WS_THICKFRAME);
            if (Point.y < WindowRect.top)
            {
                if(wi.dwStyle & WS_MINIMIZE)
                    return HTCAPTION;
                if(!ThickFrame)
                    return HTBORDER;
                if (Point.x < (WindowRect.left + XSize))
                    return HTTOPLEFT;
                if (Point.x >= (WindowRect.right - XSize))
                    return HTTOPRIGHT;
                return HTTOP;
            }
            if (Point.y >= WindowRect.bottom)
            {
                if(wi.dwStyle & WS_MINIMIZE)
                    return HTCAPTION;
                if(!ThickFrame)
                    return HTBORDER;
                if (Point.x < (WindowRect.left + XSize))
                    return HTBOTTOMLEFT;
                if (Point.x >= (WindowRect.right - XSize))
                    return HTBOTTOMRIGHT;
                return HTBOTTOM;
            }
            if (Point.x < WindowRect.left)
            {
                if(wi.dwStyle & WS_MINIMIZE)
                    return HTCAPTION;
                if(!ThickFrame)
                    return HTBORDER;
                if (Point.y < (WindowRect.top + YSize))
                    return HTTOPLEFT;
                if (Point.y >= (WindowRect.bottom - YSize))
                    return HTBOTTOMLEFT;
                return HTLEFT;
            }
            if (Point.x >= WindowRect.right)
            {
                if(wi.dwStyle & WS_MINIMIZE)
                    return HTCAPTION;
                if(!ThickFrame)
                    return HTBORDER;
                if (Point.y < (WindowRect.top + YSize))
                    return HTTOPRIGHT;
                if (Point.y >= (WindowRect.bottom - YSize))
                    return HTBOTTOMRIGHT;
                return HTRIGHT;
            }
        }
    }
    else
    {
        if (wi.dwExStyle & WS_EX_STATICEDGE)
            InflateRect(&WindowRect, -GetSystemMetrics(SM_CXBORDER),
                                     -GetSystemMetrics(SM_CYBORDER));
        if (!PtInRect(&WindowRect, Point))
            return HTBORDER;
    }

    if ((wi.dwStyle & WS_CAPTION) == WS_CAPTION)
    {
        if (wi.dwExStyle & WS_EX_TOOLWINDOW)
            WindowRect.top += GetSystemMetrics(SM_CYSMCAPTION);
        else
            WindowRect.top += GetSystemMetrics(SM_CYCAPTION);

        if (!PtInRect(&WindowRect, Point))
        {

            INT ButtonWidth;

            if (wi.dwExStyle & WS_EX_TOOLWINDOW)
                ButtonWidth = GetSystemMetrics(SM_CXSMSIZE);
            else
                ButtonWidth = GetSystemMetrics(SM_CXSIZE);

            ButtonWidth -= 4;
            ButtonWidth+= BUTTON_GAP_SIZE;

            if (wi.dwStyle & WS_SYSMENU)
            {
                if (wi.dwExStyle & WS_EX_TOOLWINDOW)
                {
                    WindowRect.right -= ButtonWidth;
                }
                else
                {
                    if(!(wi.dwExStyle & WS_EX_DLGMODALFRAME))
                        WindowRect.left += ButtonWidth;
                    WindowRect.right -= ButtonWidth;
                }
            }
            if (Point.x < WindowRect.left)
                return HTSYSMENU;
            if (WindowRect.right <= Point.x)
                return HTCLOSE;
            if (wi.dwStyle & WS_MAXIMIZEBOX || wi.dwStyle & WS_MINIMIZEBOX)
                WindowRect.right -= ButtonWidth;
            if (Point.x >= WindowRect.right)
                return HTMAXBUTTON;
            if (wi.dwStyle & WS_MINIMIZEBOX)
                WindowRect.right -= ButtonWidth;
            if (Point.x >= WindowRect.right)
                return HTMINBUTTON;
            return HTCAPTION;
        }
    }

    if(!(wi.dwStyle & WS_MINIMIZE))
    {
        HMENU menu;

        ClientPoint = Point;
        ScreenToClient(hWnd, &ClientPoint);
        GetClientRect(hWnd, &wi.rcClient);

        if (PtInRect(&wi.rcClient, ClientPoint))
        {
            return HTCLIENT;
        }

        if ((menu = GetMenu(hWnd)) && !(wi.dwStyle & WS_CHILD))
        {
            if (Point.x > 0 && Point.x < WindowRect.right && ClientPoint.y < 0)
                return HTMENU;
        }

        if (wi.dwExStyle & WS_EX_CLIENTEDGE)
        {
            InflateRect(&WindowRect, -2 * GetSystemMetrics(SM_CXBORDER),
                        -2 * GetSystemMetrics(SM_CYBORDER));
        }

        if ((wi.dwStyle & WS_VSCROLL) && (wi.dwStyle & WS_HSCROLL) &&
            (WindowRect.bottom - WindowRect.top) > GetSystemMetrics(SM_CYHSCROLL))
        {
            RECT ParentRect, TempRect = WindowRect, TempRect2 = WindowRect;
            HWND Parent = GetParent(hWnd);

            TempRect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
            if ((wi.dwExStyle & WS_EX_LEFTSCROLLBAR) != 0)
                TempRect.right = TempRect.left + GetSystemMetrics(SM_CXVSCROLL);
            else
                TempRect.left = TempRect.right - GetSystemMetrics(SM_CXVSCROLL);
            if (PtInRect(&TempRect, Point))
                return HTVSCROLL;

            TempRect2.top = TempRect2.bottom - GetSystemMetrics(SM_CYHSCROLL);
            if ((wi.dwExStyle & WS_EX_LEFTSCROLLBAR) != 0)
                TempRect2.left += GetSystemMetrics(SM_CXVSCROLL);
            else
                TempRect2.right -= GetSystemMetrics(SM_CXVSCROLL);
            if (PtInRect(&TempRect2, Point))
                return HTHSCROLL;

            TempRect.top = TempRect2.top;
            TempRect.bottom = TempRect2.bottom;
            if(Parent)
                GetClientRect(Parent, &ParentRect);
            if (PtInRect(&TempRect, Point) && HASSIZEGRIP(wi.dwStyle, wi.dwExStyle,
                      GetWindowLongW(Parent, GWL_STYLE), wi.rcWindow, ParentRect))
            {
                if ((wi.dwExStyle & WS_EX_LEFTSCROLLBAR) != 0)
                    return HTBOTTOMLEFT;
                else
                    return HTBOTTOMRIGHT;
            }
        }
        else
        {
            if (wi.dwStyle & WS_VSCROLL)
            {
                RECT TempRect = WindowRect;

                if ((wi.dwExStyle & WS_EX_LEFTSCROLLBAR) != 0)
                    TempRect.right = TempRect.left + GetSystemMetrics(SM_CXVSCROLL);
                else
                    TempRect.left = TempRect.right - GetSystemMetrics(SM_CXVSCROLL);
                if (PtInRect(&TempRect, Point))
                    return HTVSCROLL;
            } 
            else if (wi.dwStyle & WS_HSCROLL)
            {
                RECT TempRect = WindowRect;
                TempRect.top = TempRect.bottom - GetSystemMetrics(SM_CYHSCROLL);
                if (PtInRect(&TempRect, Point))
                    return HTHSCROLL;
            }
        }
    }

    return HTNOWHERE;
}

LRESULT CALLBACK 
ThemeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, WNDPROC DefWndProc)
{
    switch(Msg)
    {
    case WM_NCPAINT:
        return ThemeHandleNCPaint(hWnd, (HRGN)wParam);
    case WM_NCUAHDRAWCAPTION:
    case WM_NCUAHDRAWFRAME:
    case WM_NCACTIVATE:
        ThemeHandleNCPaint(hWnd, (HRGN)1);
        return TRUE;
    case WM_NCMOUSEMOVE:
    {
        POINT Point;
        Point.x = GET_X_LPARAM(lParam);
        Point.y = GET_Y_LPARAM(lParam);
        return ThemeHandleNcMouseMove(hWnd, wParam, &Point);
    }
    case WM_NCMOUSELEAVE:
        return ThemeHandleNcMouseLeave(hWnd);
    case WM_NCLBUTTONDOWN:
        switch (wParam)
        {
            case HTMINBUTTON:
            case HTMAXBUTTON:
            case HTCLOSE:
            {
                ThemeHandleButton(hWnd, wParam);
                return 0;
            }
            default:
                return DefWndProc(hWnd, Msg, wParam, lParam);
        }
    case WM_NCHITTEST:
    {
        POINT Point;
        Point.x = GET_X_LPARAM(lParam);
        Point.y = GET_Y_LPARAM(lParam);
        return DefWndNCHitTest(hWnd, Point);
    }
    case WM_SYSCOMMAND:
    {
        if((wParam & 0xfff0) == SC_VSCROLL ||
           (wParam & 0xfff0) == SC_HSCROLL)
        {
            POINT Pt;
            Pt.x = (short)LOWORD(lParam);
            Pt.y = (short)HIWORD(lParam);
            NC_TrackScrollBar(hWnd, wParam, Pt);
            return 0;
        }
        else
        {
            return DefWndProc(hWnd, Msg, wParam, lParam);
        }
    }
    default:
        return DefWndProc(hWnd, Msg, wParam, lParam);
    }
}

HRESULT WINAPI DrawNCPreview(HDC hDC, 
                             DWORD DNCP_Flag,
                             LPRECT prcPreview, 
                             LPCWSTR pszThemeFileName, 
                             LPCWSTR pszColorName,
                             LPCWSTR pszSizeName,
                             PNONCLIENTMETRICSW pncMetrics,
                             COLORREF* lpaRgbValues)
{
    WNDCLASSEXW DummyPreviewWindowClass;
    HWND hwndDummy;
    HRESULT hres;
    HTHEMEFILE hThemeFile;
    DRAW_CONTEXT context;
    RECT rcCurrent;

    /* FIXME: We also need to implement drawing the rest of the preview windows 
     *        and make use of the ncmetrics and colors passed as parameters */

    /* Create a dummy window that will be used to trick the paint funtions */
    memset(&DummyPreviewWindowClass, 0, sizeof(DummyPreviewWindowClass));
    DummyPreviewWindowClass.cbSize = sizeof(DummyPreviewWindowClass);
    DummyPreviewWindowClass.lpszClassName = L"DummyPreviewWindowClass";
    DummyPreviewWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    DummyPreviewWindowClass.hInstance = hDllInst;
    DummyPreviewWindowClass.lpfnWndProc = DefWindowProcW;
    if (!RegisterClassExW(&DummyPreviewWindowClass))
        return E_FAIL;

    hwndDummy = CreateWindowExW(0, L"DummyPreviewWindowClass", L"Active window", WS_OVERLAPPEDWINDOW,30,30,300,150,0,0,hDllInst,NULL);
    if (!hwndDummy)
        return E_FAIL;

    hres = OpenThemeFile(pszThemeFileName, pszColorName, pszSizeName, &hThemeFile,0);
    if (FAILED(hres))
        return hres;

    /* Initialize the special draw context for the preview */
    context.hDC = hDC;
    context.hWnd = hwndDummy;
    context.theme = OpenThemeDataFromFile(hThemeFile, hwndDummy, L"WINDOW", 0);
    if (!context.theme)
        return E_FAIL;
    context.scrolltheme = OpenThemeDataFromFile(hThemeFile, hwndDummy, L"SCROLLBAR", 0);
    if (!context.theme)
        return E_FAIL;
    context.Active = TRUE;
    if (!GetWindowInfo(hwndDummy, &context.wi))
        return E_FAIL;
    context.wi.dwStyle |= WS_VISIBLE;
    context.CaptionHeight = context.wi.cyWindowBorders;
    context.CaptionHeight += GetSystemMetrics(context.wi.dwExStyle & WS_EX_TOOLWINDOW ? SM_CYSMCAPTION : SM_CYCAPTION );
    context.hRgn = CreateRectRgnIndirect(&context.wi.rcWindow);

    /* Paint the window on the preview hDC */
    rcCurrent = context.wi.rcWindow;
    ThemePaintWindow(&context, &rcCurrent, FALSE);
    context.hDC = NULL;
    ThemeCleanupDrawContext(&context);

    /* Cleanup */
    DestroyWindow(hwndDummy);
    UnregisterClassW(L"DummyPreviewWindowClass", hDllInst);

    return S_OK;
}
