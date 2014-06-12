/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            win32ss/user/winsrv/consrv/frontends/gui/guiterm.c
 * PURPOSE:         GUI Terminal Front-End
 * PROGRAMMERS:     G� van Geldorp
 *                  Johannes Anderwald
 *                  Jeffrey Morlan
 *                  Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

/* INCLUDES *******************************************************************/

#include <consrv.h>

#define COBJMACROS
#include <shlobj.h>

#define NDEBUG
#include <debug.h>

#include "guiterm.h"
#include "resource.h"

// HACK!! Remove it when the hack in GuiWriteStream is fixed
#define CONGUI_UPDATE_TIME    0
#define CONGUI_UPDATE_TIMER   1

#define PM_CREATE_CONSOLE       (WM_APP + 1)
#define PM_DESTROY_CONSOLE      (WM_APP + 2)


/* Not defined in any header file */
extern VOID NTAPI PrivateCsrssManualGuiCheck(LONG Check);
// See winsrv/usersrv/init.c line 234


/* GLOBALS ********************************************************************/

typedef struct _GUI_INIT_INFO
{
    PCONSOLE_INFO ConsoleInfo;
    PCONSOLE_START_INFO ConsoleStartInfo;
    ULONG ProcessId;
} GUI_INIT_INFO, *PGUI_INIT_INFO;

static BOOL    ConsInitialized = FALSE;
static HWND    NotifyWnd = NULL;

extern HICON   ghDefaultIcon;
extern HICON   ghDefaultIconSm;
extern HCURSOR ghDefaultCursor;

VOID
SetConWndConsoleLeaderCID(IN PGUI_CONSOLE_DATA GuiData);
BOOLEAN
RegisterConWndClass(IN HINSTANCE hInstance);
BOOLEAN
UnRegisterConWndClass(HINSTANCE hInstance);

/* FUNCTIONS ******************************************************************/

static VOID
GetScreenBufferSizeUnits(IN PCONSOLE_SCREEN_BUFFER Buffer,
                         IN PGUI_CONSOLE_DATA GuiData,
                         OUT PUINT WidthUnit,
                         OUT PUINT HeightUnit)
{
    if (Buffer == NULL || GuiData == NULL ||
        WidthUnit == NULL || HeightUnit == NULL)
    {
        return;
    }

    if (GetType(Buffer) == TEXTMODE_BUFFER)
    {
        *WidthUnit  = GuiData->CharWidth ;
        *HeightUnit = GuiData->CharHeight;
    }
    else /* if (GetType(Buffer) == GRAPHICS_BUFFER) */
    {
        *WidthUnit  = 1;
        *HeightUnit = 1;
    }
}

VOID
GuiConsoleMoveWindow(PGUI_CONSOLE_DATA GuiData)
{
    /* Move the window if needed (not positioned by the system) */
    if (!GuiData->GuiInfo.AutoPosition)
    {
        SetWindowPos(GuiData->hWindow,
                     NULL,
                     GuiData->GuiInfo.WindowOrigin.x,
                     GuiData->GuiInfo.WindowOrigin.y,
                     0, 0,
                     SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

static VOID
SmallRectToRect(PGUI_CONSOLE_DATA GuiData, PRECT Rect, PSMALL_RECT SmallRect)
{
    PCONSOLE_SCREEN_BUFFER Buffer = GuiData->ActiveBuffer;
    UINT WidthUnit, HeightUnit;

    GetScreenBufferSizeUnits(Buffer, GuiData, &WidthUnit, &HeightUnit);

    Rect->left   = (SmallRect->Left       - Buffer->ViewOrigin.X) * WidthUnit ;
    Rect->top    = (SmallRect->Top        - Buffer->ViewOrigin.Y) * HeightUnit;
    Rect->right  = (SmallRect->Right  + 1 - Buffer->ViewOrigin.X) * WidthUnit ;
    Rect->bottom = (SmallRect->Bottom + 1 - Buffer->ViewOrigin.Y) * HeightUnit;
}

static VOID
DrawRegion(PGUI_CONSOLE_DATA GuiData,
           SMALL_RECT* Region)
{
    RECT RegionRect;

    SmallRectToRect(GuiData, &RegionRect, Region);
    /* Do not erase the background: it speeds up redrawing and reduce flickering */
    InvalidateRect(GuiData->hWindow, &RegionRect, FALSE);
    /**UpdateWindow(GuiData->hWindow);**/
}

VOID
InvalidateCell(PGUI_CONSOLE_DATA GuiData,
               SHORT x, SHORT y)
{
    SMALL_RECT CellRect = { x, y, x, y };
    DrawRegion(GuiData, &CellRect);
}


/******************************************************************************
 *                        GUI Terminal Initialization                         *
 ******************************************************************************/

VOID
SwitchFullScreen(PGUI_CONSOLE_DATA GuiData, BOOL FullScreen);

static LRESULT CALLBACK
GuiConsoleNotifyWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND NewWindow;
    LONG WindowCount;
    MSG Msg;

    switch (msg)
    {
        case WM_CREATE:
        {
            SetWindowLongW(hWnd, GWL_USERDATA, 0);
            return 0;
        }
    
        case PM_CREATE_CONSOLE:
        {
            PGUI_CONSOLE_DATA GuiData = (PGUI_CONSOLE_DATA)lParam;
            PCONSOLE Console = GuiData->Console;
            RECT rcWnd;

            DPRINT("PM_CREATE_CONSOLE -- creating window\n");

            NewWindow = CreateWindowExW(WS_EX_CLIENTEDGE,
                                        GUI_CONWND_CLASS,
                                        Console->Title.Buffer,
                                        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        NULL,
                                        NULL,
                                        ConSrvDllInstance,
                                        (PVOID)GuiData);
            if (NULL != NewWindow)
            {
                ASSERT(NewWindow == GuiData->hWindow);

                WindowCount = GetWindowLongW(hWnd, GWL_USERDATA);
                WindowCount++;
                SetWindowLongW(hWnd, GWL_USERDATA, WindowCount);

                //
                // FIXME: TODO: Move everything there into conwnd.c!OnNcCreate()
                //

                /* Retrieve our real position */
                // See conwnd.c!OnMove()
                GetWindowRect(GuiData->hWindow, &rcWnd);
                GuiData->GuiInfo.WindowOrigin.x = rcWnd.left;
                GuiData->GuiInfo.WindowOrigin.y = rcWnd.top;

                /* Move and resize the window to the user's values */
                /* CAN WE DEADLOCK ?? */
                GuiConsoleMoveWindow(GuiData); // FIXME: This MUST be done via the CreateWindowExW call.
                SendMessageW(GuiData->hWindow, PM_RESIZE_TERMINAL, 0, 0);

                /* Switch to full-screen mode if necessary */
                // FIXME: Move elsewhere, it cause misdrawings of the window.
                if (GuiData->GuiInfo.FullScreen) SwitchFullScreen(GuiData, TRUE);

                DPRINT("PM_CREATE_CONSOLE -- showing window\n");
                // ShowWindow(NewWindow, (int)wParam);
                ShowWindowAsync(NewWindow, (int)wParam);
                DPRINT("Window showed\n");
            }

            return (LRESULT)NewWindow;
        }

        case PM_DESTROY_CONSOLE:
        {
            PGUI_CONSOLE_DATA GuiData = (PGUI_CONSOLE_DATA)lParam;

            /* Exit the full screen mode if it was already set */
            // LeaveFullScreen(GuiData);

            /*
             * Window creation is done using a PostMessage(), so it's possible
             * that the window that we want to destroy doesn't exist yet.
             * So first empty the message queue.
             */
            /*
            while (PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Msg);
                DispatchMessageW(&Msg);
            }*/
            while (PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE)) ;

            if (GuiData->hWindow != NULL) /* && DestroyWindow(GuiData->hWindow) */
            {
                DestroyWindow(GuiData->hWindow);

                WindowCount = GetWindowLongW(hWnd, GWL_USERDATA);
                WindowCount--;
                SetWindowLongW(hWnd, GWL_USERDATA, WindowCount);
                if (0 == WindowCount)
                {
                    NotifyWnd = NULL;
                    DestroyWindow(hWnd);
                    DPRINT("CONSRV: Going to quit the Gui Thread!!\n");
                    PostQuitMessage(0);
                }
            }

            return 0;
        }

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

static DWORD NTAPI
GuiConsoleGuiThread(PVOID Data)
{
    MSG msg;
    PHANDLE GraphicsStartupEvent = (PHANDLE)Data;

    /*
     * This thread dispatches all the console notifications to the notify window.
     * It is common for all the console windows.
     */

    PrivateCsrssManualGuiCheck(+1);

    NotifyWnd = CreateWindowW(L"ConSrvCreateNotify",
                              L"",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              NULL,
                              ConSrvDllInstance,
                              NULL);
    if (NULL == NotifyWnd)
    {
        PrivateCsrssManualGuiCheck(-1);
        SetEvent(*GraphicsStartupEvent);
        return 1;
    }

    SetEvent(*GraphicsStartupEvent);

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DPRINT("CONSRV: Quit the Gui Thread!!\n");
    PrivateCsrssManualGuiCheck(-1);

    return 1;
}

static BOOL
GuiInit(VOID)
{
    WNDCLASSEXW wc;

    /* Exit if we were already initialized */
    // if (ConsInitialized) return TRUE;

    /*
     * Initialize and register the different window classes, if needed.
     */
    if (!ConsInitialized)
    {
        /* Initialize the notification window class */
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpszClassName = L"ConSrvCreateNotify";
        wc.lpfnWndProc = GuiConsoleNotifyWndProc;
        wc.style = 0;
        wc.hInstance = ConSrvDllInstance;
        wc.hIcon = NULL;
        wc.hIconSm = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        if (RegisterClassExW(&wc) == 0)
        {
            DPRINT1("Failed to register GUI notify wndproc\n");
            return FALSE;
        }

        /* Initialize the console window class */
        if (!RegisterConWndClass(ConSrvDllInstance))
            return FALSE;

        ConsInitialized = TRUE;
    }

    /*
     * Set-up the notification window
     */
    if (NULL == NotifyWnd)
    {
        HANDLE ThreadHandle;
        HANDLE GraphicsStartupEvent;

        GraphicsStartupEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (NULL == GraphicsStartupEvent) return FALSE;

        ThreadHandle = CreateThread(NULL,
                                    0,
                                    GuiConsoleGuiThread,
                                    (PVOID)&GraphicsStartupEvent,
                                    0,
                                    NULL);
        if (NULL == ThreadHandle)
        {
            CloseHandle(GraphicsStartupEvent);
            DPRINT1("CONSRV: Failed to create graphics console thread. Expect problems\n");
            return FALSE;
        }
        SetThreadPriority(ThreadHandle, THREAD_PRIORITY_HIGHEST);
        CloseHandle(ThreadHandle);

        WaitForSingleObject(GraphicsStartupEvent, INFINITE);
        CloseHandle(GraphicsStartupEvent);

        if (NULL == NotifyWnd)
        {
            DPRINT1("CONSRV: Failed to create notification window.\n");
            return FALSE;
        }
    }

    // ConsInitialized = TRUE;

    return TRUE;
}



/******************************************************************************
 *                             GUI Console Driver                             *
 ******************************************************************************/

static VOID NTAPI
GuiDeinitFrontEnd(IN OUT PFRONTEND This);

NTSTATUS NTAPI
GuiInitFrontEnd(IN OUT PFRONTEND This,
                IN PCONSOLE Console)
{
    PGUI_INIT_INFO GuiInitInfo;
    PCONSOLE_INFO  ConsoleInfo;
    PCONSOLE_START_INFO ConsoleStartInfo;

    PGUI_CONSOLE_DATA GuiData;
    GUI_CONSOLE_INFO  TermInfo;

    SIZE_T Length    = 0;
    LPWSTR IconPath  = NULL;
    INT    IconIndex = 0;

    if (This == NULL || Console == NULL || This->OldData == NULL)
        return STATUS_INVALID_PARAMETER;

    ASSERT(This->Console == Console);

    GuiInitInfo = This->OldData;

    if (GuiInitInfo->ConsoleInfo == NULL || GuiInitInfo->ConsoleStartInfo == NULL)
        return STATUS_INVALID_PARAMETER;

    ConsoleInfo      = GuiInitInfo->ConsoleInfo;
    ConsoleStartInfo = GuiInitInfo->ConsoleStartInfo;

    IconPath  = ConsoleStartInfo->IconPath;
    IconIndex = ConsoleStartInfo->IconIndex;


    /* Terminal data allocation */
    GuiData = ConsoleAllocHeap(HEAP_ZERO_MEMORY, sizeof(GUI_CONSOLE_DATA));
    if (!GuiData)
    {
        DPRINT1("CONSRV: Failed to create GUI_CONSOLE_DATA\n");
        return STATUS_UNSUCCESSFUL;
    }
    ///// /* HACK */ Console->TermIFace.Data = (PVOID)GuiData; /* HACK */
    GuiData->Console      = Console;
    GuiData->ActiveBuffer = Console->ActiveBuffer;
    GuiData->hWindow = NULL;

    /* The console can be resized */
    Console->FixedSize = FALSE;

    InitializeCriticalSection(&GuiData->Lock);


    /*
     * Load terminal settings
     */

    /* 1. Load the default settings */
    GuiConsoleGetDefaultSettings(&TermInfo, GuiInitInfo->ProcessId);

    /* 3. Load the remaining console settings via the registry. */
    if ((ConsoleStartInfo->dwStartupFlags & STARTF_TITLEISLINKNAME) == 0)
    {
        /* Load the terminal infos from the registry. */
        GuiConsoleReadUserSettings(&TermInfo,
                                   ConsoleInfo->ConsoleTitle,
                                   GuiInitInfo->ProcessId);

        /*
         * Now, update them with the properties the user might gave to us
         * via the STARTUPINFO structure before calling CreateProcess
         * (and which was transmitted via the ConsoleStartInfo structure).
         * We therefore overwrite the values read in the registry.
         */
        if (ConsoleStartInfo->dwStartupFlags & STARTF_USESHOWWINDOW)
        {
            TermInfo.ShowWindow = ConsoleStartInfo->wShowWindow;
        }
        if (ConsoleStartInfo->dwStartupFlags & STARTF_USEPOSITION)
        {
            TermInfo.AutoPosition = FALSE;
            TermInfo.WindowOrigin.x = ConsoleStartInfo->dwWindowOrigin.X;
            TermInfo.WindowOrigin.y = ConsoleStartInfo->dwWindowOrigin.Y;
        }
        if (ConsoleStartInfo->dwStartupFlags & STARTF_RUNFULLSCREEN)
        {
            TermInfo.FullScreen = TRUE;
        }
    }


    /*
     * Set up GUI data
     */

    Length = min(wcslen(TermInfo.FaceName) + 1, LF_FACESIZE); // wcsnlen
    wcsncpy(GuiData->GuiInfo.FaceName, TermInfo.FaceName, LF_FACESIZE);
    GuiData->GuiInfo.FaceName[Length] = L'\0';
    GuiData->GuiInfo.FontFamily     = TermInfo.FontFamily;
    GuiData->GuiInfo.FontSize       = TermInfo.FontSize;
    GuiData->GuiInfo.FontWeight     = TermInfo.FontWeight;
    GuiData->GuiInfo.UseRasterFonts = TermInfo.UseRasterFonts;
    GuiData->GuiInfo.FullScreen     = TermInfo.FullScreen;
    GuiData->GuiInfo.ShowWindow     = TermInfo.ShowWindow;
    GuiData->GuiInfo.AutoPosition   = TermInfo.AutoPosition;
    GuiData->GuiInfo.WindowOrigin   = TermInfo.WindowOrigin;

    /* Initialize the icon handles to their default values */
    GuiData->hIcon   = ghDefaultIcon;
    GuiData->hIconSm = ghDefaultIconSm;

    /* Get the associated icon, if any */
    if (IconPath == NULL || IconPath[0] == L'\0')
    {
        IconPath  = ConsoleStartInfo->AppPath;
        IconIndex = 0;
    }
    DPRINT("IconPath = %S ; IconIndex = %lu\n", (IconPath ? IconPath : L"n/a"), IconIndex);
    if (IconPath && IconPath[0] != L'\0')
    {
        HICON hIcon = NULL, hIconSm = NULL;
        PrivateExtractIconExW(IconPath,
                              IconIndex,
                              &hIcon,
                              &hIconSm,
                              1);
        DPRINT("hIcon = 0x%p ; hIconSm = 0x%p\n", hIcon, hIconSm);
        if (hIcon   != NULL) GuiData->hIcon   = hIcon;
        if (hIconSm != NULL) GuiData->hIconSm = hIconSm;
    }
    ASSERT(GuiData->hIcon && GuiData->hIconSm);

    /* Mouse is shown by default with its default cursor shape */
    GuiData->hCursor = ghDefaultCursor;
    GuiData->MouseCursorRefCount = 0;

    /* A priori don't ignore mouse signals */
    GuiData->IgnoreNextMouseSignal = FALSE;

    /* Close button and the corresponding system menu item are enabled by default */
    GuiData->IsCloseButtonEnabled = TRUE;

    /* There is no user-reserved menu id range by default */
    GuiData->CmdIdLow = GuiData->CmdIdHigh = 0;

    /* Initialize the selection */
    RtlZeroMemory(&GuiData->Selection, sizeof(GuiData->Selection));
    GuiData->Selection.dwFlags = CONSOLE_NO_SELECTION;
    RtlZeroMemory(&GuiData->dwSelectionCursor, sizeof(GuiData->dwSelectionCursor));
    GuiData->LineSelection = FALSE; // Default to block selection
    // TODO: Retrieve the selection mode via the registry.

    /*
     * We need to wait until the GUI has been fully initialized
     * to retrieve custom settings i.e. WindowSize etc...
     * Ideally we could use SendNotifyMessage for this but its not
     * yet implemented.
     */
    GuiData->hGuiInitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

    DPRINT("GUI - Checkpoint\n");

    /* Create the terminal window */
    PostMessageW(NotifyWnd, PM_CREATE_CONSOLE, GuiData->GuiInfo.ShowWindow, (LPARAM)GuiData);

    /* Wait until initialization has finished */
    WaitForSingleObject(GuiData->hGuiInitEvent, INFINITE);
    DPRINT("OK we created the console window\n");
    CloseHandle(GuiData->hGuiInitEvent);
    GuiData->hGuiInitEvent = NULL;

    /* Check whether we really succeeded in initializing the terminal window */
    if (GuiData->hWindow == NULL)
    {
        DPRINT("GuiInitConsole - We failed at creating a new terminal window\n");
        GuiDeinitFrontEnd(This);
        return STATUS_UNSUCCESSFUL;
    }

    /* Finally, finish to initialize the frontend structure */
    This->Data = GuiData;
    if (This->OldData) ConsoleFreeHeap(This->OldData);
    This->OldData = NULL;

    return STATUS_SUCCESS;
}

static VOID NTAPI
GuiDeinitFrontEnd(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    SendMessageW(NotifyWnd, PM_DESTROY_CONSOLE, 0, (LPARAM)GuiData);

    DPRINT("Destroying icons !! - GuiData->hIcon = 0x%p ; ghDefaultIcon = 0x%p ; GuiData->hIconSm = 0x%p ; ghDefaultIconSm = 0x%p\n",
            GuiData->hIcon, ghDefaultIcon, GuiData->hIconSm, ghDefaultIconSm);
    if (GuiData->hIcon != NULL && GuiData->hIcon != ghDefaultIcon)
    {
        DPRINT("Destroy hIcon\n");
        DestroyIcon(GuiData->hIcon);
    }
    if (GuiData->hIconSm != NULL && GuiData->hIconSm != ghDefaultIconSm)
    {
        DPRINT("Destroy hIconSm\n");
        DestroyIcon(GuiData->hIconSm);
    }

    This->Data = NULL;
    DeleteCriticalSection(&GuiData->Lock);
    ConsoleFreeHeap(GuiData);

    DPRINT("Quit GuiDeinitFrontEnd\n");
}

static VOID NTAPI
GuiDrawRegion(IN OUT PFRONTEND This,
              SMALL_RECT* Region)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    DrawRegion(GuiData, Region);
}

static VOID NTAPI
GuiWriteStream(IN OUT PFRONTEND This,
               SMALL_RECT* Region,
               SHORT CursorStartX,
               SHORT CursorStartY,
               UINT ScrolledLines,
               PWCHAR Buffer,
               UINT Length)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    PCONSOLE_SCREEN_BUFFER Buff;
    SHORT CursorEndX, CursorEndY;
    RECT ScrollRect;

    if (NULL == GuiData || NULL == GuiData->hWindow) return;

    Buff = GuiData->ActiveBuffer;
    if (GetType(Buff) != TEXTMODE_BUFFER) return;

    if (0 != ScrolledLines)
    {
        ScrollRect.left = 0;
        ScrollRect.top = 0;
        ScrollRect.right = Buff->ViewSize.X * GuiData->CharWidth;
        ScrollRect.bottom = Region->Top * GuiData->CharHeight;

        ScrollWindowEx(GuiData->hWindow,
                       0,
                       -(int)(ScrolledLines * GuiData->CharHeight),
                       &ScrollRect,
                       NULL,
                       NULL,
                       NULL,
                       SW_INVALIDATE);
    }

    DrawRegion(GuiData, Region);

    if (CursorStartX < Region->Left || Region->Right < CursorStartX
            || CursorStartY < Region->Top || Region->Bottom < CursorStartY)
    {
        InvalidateCell(GuiData, CursorStartX, CursorStartY);
    }

    CursorEndX = Buff->CursorPosition.X;
    CursorEndY = Buff->CursorPosition.Y;
    if ((CursorEndX < Region->Left || Region->Right < CursorEndX
            || CursorEndY < Region->Top || Region->Bottom < CursorEndY)
            && (CursorEndX != CursorStartX || CursorEndY != CursorStartY))
    {
        InvalidateCell(GuiData, CursorEndX, CursorEndY);
    }

    // HACK!!
    // Set up the update timer (very short interval) - this is a "hack" for getting the OS to
    // repaint the window without having it just freeze up and stay on the screen permanently.
    Buff->CursorBlinkOn = TRUE;
    SetTimer(GuiData->hWindow, CONGUI_UPDATE_TIMER, CONGUI_UPDATE_TIME, NULL);
}

static BOOL NTAPI
GuiSetCursorInfo(IN OUT PFRONTEND This,
                 PCONSOLE_SCREEN_BUFFER Buff)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    if (GuiData->ActiveBuffer == Buff)
    {
        InvalidateCell(GuiData, Buff->CursorPosition.X, Buff->CursorPosition.Y);
    }

    return TRUE;
}

static BOOL NTAPI
GuiSetScreenInfo(IN OUT PFRONTEND This,
                 PCONSOLE_SCREEN_BUFFER Buff,
                 SHORT OldCursorX,
                 SHORT OldCursorY)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    if (GuiData->ActiveBuffer == Buff)
    {
        /* Redraw char at old position (remove cursor) */
        InvalidateCell(GuiData, OldCursorX, OldCursorY);
        /* Redraw char at new position (show cursor) */
        InvalidateCell(GuiData, Buff->CursorPosition.X, Buff->CursorPosition.Y);
    }

    return TRUE;
}

static VOID NTAPI
GuiResizeTerminal(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    /* Resize the window to the user's values */
    PostMessageW(GuiData->hWindow, PM_RESIZE_TERMINAL, 0, 0);
}

static VOID NTAPI
GuiSetActiveScreenBuffer(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    PCONSOLE_SCREEN_BUFFER ActiveBuffer;
    HPALETTE hPalette;

    EnterCriticalSection(&GuiData->Lock);
    GuiData->WindowSizeLock = TRUE;

    InterlockedExchangePointer(&GuiData->ActiveBuffer,
                               ConDrvGetActiveScreenBuffer(GuiData->Console));

    GuiData->WindowSizeLock = FALSE;
    LeaveCriticalSection(&GuiData->Lock);

    ActiveBuffer = GuiData->ActiveBuffer;

    /* Change the current palette */
    if (ActiveBuffer->PaletteHandle == NULL)
    {
        hPalette = GuiData->hSysPalette;
    }
    else
    {
        hPalette = ActiveBuffer->PaletteHandle;
    }

    DPRINT1("GuiSetActiveScreenBuffer using palette 0x%p\n", hPalette);

    /* Set the new palette for the framebuffer */
    SelectPalette(GuiData->hMemDC, hPalette, FALSE);

    /* Specify the use of the system palette for the framebuffer */
    SetSystemPaletteUse(GuiData->hMemDC, ActiveBuffer->PaletteUsage);

    /* Realize the (logical) palette */
    RealizePalette(GuiData->hMemDC);

    GuiResizeTerminal(This);
    // ConioDrawConsole(Console);
}

static VOID NTAPI
GuiReleaseScreenBuffer(IN OUT PFRONTEND This,
                       IN PCONSOLE_SCREEN_BUFFER ScreenBuffer)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    /*
     * If we were notified to release a screen buffer that is not actually
     * ours, then just ignore the notification...
     */
    if (ScreenBuffer != GuiData->ActiveBuffer) return;

    /*
     * ... else, we must release our active buffer. Two cases are present:
     * - If ScreenBuffer (== GuiData->ActiveBuffer) IS NOT the console
     *   active screen buffer, then we can safely switch to it.
     * - If ScreenBuffer IS the console active screen buffer, we must release
     *   it ONLY.
     */

    /* Release the old active palette and set the default one */
    if (GetCurrentObject(GuiData->hMemDC, OBJ_PAL) == ScreenBuffer->PaletteHandle)
    {
        /* Set the new palette */
        SelectPalette(GuiData->hMemDC, GuiData->hSysPalette, FALSE);
    }

    /* Set the adequate active screen buffer */
    if (ScreenBuffer != GuiData->Console->ActiveBuffer)
    {
        GuiSetActiveScreenBuffer(This);
    }
    else
    {
        EnterCriticalSection(&GuiData->Lock);
        GuiData->WindowSizeLock = TRUE;

        InterlockedExchangePointer(&GuiData->ActiveBuffer, NULL);

        GuiData->WindowSizeLock = FALSE;
        LeaveCriticalSection(&GuiData->Lock);
    }
}

static BOOL NTAPI
GuiProcessKeyCallback(IN OUT PFRONTEND This,
                      MSG* msg,
                      BYTE KeyStateMenu,
                      DWORD ShiftState,
                      UINT VirtualKeyCode,
                      BOOL Down)
{
    if ((ShiftState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED) || KeyStateMenu & 0x80) &&
        (VirtualKeyCode == VK_ESCAPE || VirtualKeyCode == VK_TAB || VirtualKeyCode == VK_SPACE))
    {
        DPRINT1("GuiProcessKeyCallback\n");
        //DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        //return TRUE;
    }

    return FALSE;
}

static BOOL NTAPI
GuiSetMouseCursor(IN OUT PFRONTEND This,
                  HCURSOR CursorHandle);

static VOID NTAPI
GuiRefreshInternalInfo(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    /* Update the console leader information held by the window */
    SetConWndConsoleLeaderCID(GuiData);

    /*
     * HACK:
     * We reset the cursor here so that, when a console app quits, we reset
     * the cursor to the default one. It's quite a hack since it doesn't proceed
     * per - console process... This must be fixed.
     *
     * See GuiInitConsole(...) for more information.
     */

    /* Mouse is shown by default with its default cursor shape */
    GuiData->MouseCursorRefCount = 0; // Reinitialize the reference counter
    GuiSetMouseCursor(This, NULL);
}

static VOID NTAPI
GuiChangeTitle(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    // PostMessageW(GuiData->hWindow, PM_CONSOLE_SET_TITLE, 0, 0);
    SetWindowText(GuiData->hWindow, GuiData->Console->Title.Buffer);
}

static BOOL NTAPI
GuiChangeIcon(IN OUT PFRONTEND This,
              HICON IconHandle)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    HICON hIcon, hIconSm;

    if (IconHandle == NULL)
    {
        hIcon   = ghDefaultIcon;
        hIconSm = ghDefaultIconSm;
    }
    else
    {
        hIcon   = CopyIcon(IconHandle);
        hIconSm = CopyIcon(IconHandle);
    }

    if (hIcon == NULL)
    {
        return FALSE;
    }

    if (hIcon != GuiData->hIcon)
    {
        if (GuiData->hIcon != NULL && GuiData->hIcon != ghDefaultIcon)
        {
            DestroyIcon(GuiData->hIcon);
        }
        if (GuiData->hIconSm != NULL && GuiData->hIconSm != ghDefaultIconSm)
        {
            DestroyIcon(GuiData->hIconSm);
        }

        GuiData->hIcon   = hIcon;
        GuiData->hIconSm = hIconSm;

        DPRINT("Set icons in GuiChangeIcon\n");
        PostMessageW(GuiData->hWindow, WM_SETICON, ICON_BIG  , (LPARAM)GuiData->hIcon  );
        PostMessageW(GuiData->hWindow, WM_SETICON, ICON_SMALL, (LPARAM)GuiData->hIconSm);
    }

    return TRUE;
}

static HWND NTAPI
GuiGetConsoleWindowHandle(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    return GuiData->hWindow;
}

static VOID NTAPI
GuiGetLargestConsoleWindowSize(IN OUT PFRONTEND This,
                               PCOORD pSize)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    PCONSOLE_SCREEN_BUFFER ActiveBuffer;
    RECT WorkArea;
    LONG width, height;
    UINT WidthUnit, HeightUnit;

    if (!pSize) return;

    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &WorkArea, 0))
    {
        DPRINT1("SystemParametersInfoW failed - What to do ??\n");
        return;
    }

    ActiveBuffer = GuiData->ActiveBuffer;
    if (ActiveBuffer)
    {
        GetScreenBufferSizeUnits(ActiveBuffer, GuiData, &WidthUnit, &HeightUnit);
    }
    else
    {
        /* Default: text mode */
        WidthUnit  = GuiData->CharWidth ;
        HeightUnit = GuiData->CharHeight;
    }

    width  = WorkArea.right;
    height = WorkArea.bottom;

    width  -= (2 * (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE)));
    height -= (2 * (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION));

    if (width  < 0) width  = 0;
    if (height < 0) height = 0;

    pSize->X = (SHORT)(width  / (int)WidthUnit ) /* HACK */ + 2;
    pSize->Y = (SHORT)(height / (int)HeightUnit) /* HACK */ + 1;
}

static BOOL NTAPI
GuiGetSelectionInfo(IN OUT PFRONTEND This,
                    PCONSOLE_SELECTION_INFO pSelectionInfo)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    if (pSelectionInfo == NULL) return FALSE;

    ZeroMemory(pSelectionInfo, sizeof(CONSOLE_SELECTION_INFO));
    if (GuiData->Selection.dwFlags != CONSOLE_NO_SELECTION)
        RtlCopyMemory(pSelectionInfo, &GuiData->Selection, sizeof(CONSOLE_SELECTION_INFO));

    return TRUE;
}

static BOOL NTAPI
GuiSetPalette(IN OUT PFRONTEND This,
              HPALETTE PaletteHandle,
              UINT PaletteUsage)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    HPALETTE OldPalette;

    // if (GetType(GuiData->ActiveBuffer) != GRAPHICS_BUFFER) return FALSE;
    if (PaletteHandle == NULL) return FALSE;

    /* Set the new palette for the framebuffer */
    OldPalette = SelectPalette(GuiData->hMemDC, PaletteHandle, FALSE);
    if (OldPalette == NULL) return FALSE;

    /* Specify the use of the system palette for the framebuffer */
    SetSystemPaletteUse(GuiData->hMemDC, PaletteUsage);

    /* Realize the (logical) palette */
    RealizePalette(GuiData->hMemDC);

    /* Save the original system palette handle */
    if (GuiData->hSysPalette == NULL) GuiData->hSysPalette = OldPalette;

    return TRUE;
}

static ULONG NTAPI
GuiGetDisplayMode(IN OUT PFRONTEND This)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    ULONG DisplayMode = 0;

    if (GuiData->GuiInfo.FullScreen)
        DisplayMode |= CONSOLE_FULLSCREEN_HARDWARE; // CONSOLE_FULLSCREEN
    else
        DisplayMode |= CONSOLE_WINDOWED;

    return DisplayMode;
}

static BOOL NTAPI
GuiSetDisplayMode(IN OUT PFRONTEND This,
                  ULONG NewMode)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;
    BOOL FullScreen;

    if (NewMode & ~(CONSOLE_FULLSCREEN_MODE | CONSOLE_WINDOWED_MODE))
        return FALSE;

    FullScreen = ((NewMode & CONSOLE_FULLSCREEN_MODE) != 0);

    if (FullScreen != GuiData->GuiInfo.FullScreen)
    {
        SwitchFullScreen(GuiData, FullScreen);
    }

    return TRUE;
}

static INT NTAPI
GuiShowMouseCursor(IN OUT PFRONTEND This,
                   BOOL Show)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    /* Set the reference count */
    if (Show) ++GuiData->MouseCursorRefCount;
    else      --GuiData->MouseCursorRefCount;

    /* Effectively show (or hide) the cursor (use special values for (w|l)Param) */
    PostMessageW(GuiData->hWindow, WM_SETCURSOR, -1, -1);

    return GuiData->MouseCursorRefCount;
}

static BOOL NTAPI
GuiSetMouseCursor(IN OUT PFRONTEND This,
                  HCURSOR CursorHandle)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    /*
     * Set the cursor's handle. If the given handle is NULL,
     * then restore the default cursor.
     */
    GuiData->hCursor = (CursorHandle ? CursorHandle : ghDefaultCursor);

    /* Effectively modify the shape of the cursor (use special values for (w|l)Param) */
    PostMessageW(GuiData->hWindow, WM_SETCURSOR, -1, -1);

    return TRUE;
}

static HMENU NTAPI
GuiMenuControl(IN OUT PFRONTEND This,
               UINT CmdIdLow,
               UINT CmdIdHigh)
{
    PGUI_CONSOLE_DATA GuiData = This->Data;

    GuiData->CmdIdLow  = CmdIdLow ;
    GuiData->CmdIdHigh = CmdIdHigh;

    return GetSystemMenu(GuiData->hWindow, FALSE);
}

static BOOL NTAPI
GuiSetMenuClose(IN OUT PFRONTEND This,
                BOOL Enable)
{
    /*
     * NOTE: See http://www.mail-archive.com/harbour@harbour-project.org/msg27509.html
     * or http://harbour-devel.1590103.n2.nabble.com/Question-about-hb-gt-win-CtrlHandler-usage-td4670862i20.html
     * for more information.
     */

    PGUI_CONSOLE_DATA GuiData = This->Data;
    HMENU hSysMenu = GetSystemMenu(GuiData->hWindow, FALSE);

    if (hSysMenu == NULL) return FALSE;

    GuiData->IsCloseButtonEnabled = Enable;
    EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | (Enable ? MF_ENABLED : MF_GRAYED));

    return TRUE;
}

static FRONTEND_VTBL GuiVtbl =
{
    GuiInitFrontEnd,
    GuiDeinitFrontEnd,
    GuiDrawRegion,
    GuiWriteStream,
    GuiSetCursorInfo,
    GuiSetScreenInfo,
    GuiResizeTerminal,
    GuiSetActiveScreenBuffer,
    GuiReleaseScreenBuffer,
    GuiProcessKeyCallback,
    GuiRefreshInternalInfo,
    GuiChangeTitle,
    GuiChangeIcon,
    GuiGetConsoleWindowHandle,
    GuiGetLargestConsoleWindowSize,
    GuiGetSelectionInfo,
    GuiSetPalette,
    GuiGetDisplayMode,
    GuiSetDisplayMode,
    GuiShowMouseCursor,
    GuiSetMouseCursor,
    GuiMenuControl,
    GuiSetMenuClose,
};


static BOOL
LoadShellLinkConsoleInfo(IN OUT PCONSOLE_START_INFO ConsoleStartInfo,
                         IN OUT PCONSOLE_INFO ConsoleInfo)
{
#define PATH_SEPARATOR L'\\'

    BOOL    RetVal   = FALSE;
    HRESULT hRes     = S_OK;
    LPWSTR  LinkName = NULL;
    SIZE_T  Length   = 0;

    if ((ConsoleStartInfo->dwStartupFlags & STARTF_TITLEISLINKNAME) == 0)
        return FALSE;

    ConsoleStartInfo->IconPath[0] = L'\0';
    ConsoleStartInfo->IconIndex   = 0;

    /* 1- Find the last path separator if any */
    LinkName = wcsrchr(ConsoleStartInfo->ConsoleTitle, PATH_SEPARATOR);
    if (LinkName == NULL)
    {
        LinkName = ConsoleStartInfo->ConsoleTitle;
    }
    else
    {
        /* Skip the path separator */
        ++LinkName;
    }

    /* 2- Check for the link extension. The name ".lnk" is considered invalid. */
    Length = wcslen(LinkName);
    if ( (Length <= 4) || (wcsicmp(LinkName + (Length - 4), L".lnk") != 0) )
        return FALSE;

    /* 3- It may be a link. Try to retrieve some properties */
    hRes = CoInitialize(NULL);
    if (SUCCEEDED(hRes))
    {
        /* Get a pointer to the IShellLink interface */
        IShellLinkW* pshl = NULL;
        hRes = CoCreateInstance(&CLSID_ShellLink,
                                NULL, 
                                CLSCTX_INPROC_SERVER,
                                &IID_IShellLinkW,
                                (LPVOID*)&pshl);
        if (SUCCEEDED(hRes))
        {
            /* Get a pointer to the IPersistFile interface */
            IPersistFile* ppf = NULL;
            hRes = IPersistFile_QueryInterface(pshl, &IID_IPersistFile, (LPVOID*)&ppf);
            if (SUCCEEDED(hRes))
            {
                /* Load the shortcut */
                hRes = IPersistFile_Load(ppf, ConsoleStartInfo->ConsoleTitle, STGM_READ);
                if (SUCCEEDED(hRes))
                {
                    /*
                     * Finally we can get the properties !
                     * Update the old ones if needed.
                     */
                    INT ShowCmd = 0;
                    // WORD HotKey = 0;

                    /* Reset the name of the console with the name of the shortcut */
                    Length = min(/*Length*/ Length - 4, // 4 == len(".lnk")
                                 sizeof(ConsoleInfo->ConsoleTitle) / sizeof(ConsoleInfo->ConsoleTitle[0]) - 1);
                    wcsncpy(ConsoleInfo->ConsoleTitle, LinkName, Length);
                    ConsoleInfo->ConsoleTitle[Length] = L'\0';

                    /* Get the window showing command */
                    hRes = IShellLinkW_GetShowCmd(pshl, &ShowCmd);
                    if (SUCCEEDED(hRes)) ConsoleStartInfo->wShowWindow = (WORD)ShowCmd;

                    /* Get the hotkey */
                    // hRes = pshl->GetHotkey(&ShowCmd);
                    // if (SUCCEEDED(hRes)) ConsoleStartInfo->HotKey = HotKey;

                    /* Get the icon location, if any */

                    hRes = IShellLinkW_GetIconLocation(pshl,
                                                       ConsoleStartInfo->IconPath,
                                                       sizeof(ConsoleStartInfo->IconPath)/sizeof(ConsoleStartInfo->IconPath[0]) - 1, // == MAX_PATH
                                                       &ConsoleStartInfo->IconIndex);
                    if (!SUCCEEDED(hRes))
                    {
                        ConsoleStartInfo->IconPath[0] = L'\0';
                        ConsoleStartInfo->IconIndex   = 0;
                    }

                    // FIXME: Since we still don't load console properties from the shortcut,
                    // return false. When this will be done, we will return true instead.
                    RetVal = FALSE;
                }
                IPersistFile_Release(ppf);
            }
            IShellLinkW_Release(pshl);
        }
    }
    CoUninitialize();

    return RetVal;
}

NTSTATUS NTAPI
GuiLoadFrontEnd(IN OUT PFRONTEND FrontEnd,
                IN OUT PCONSOLE_INFO ConsoleInfo,
                IN OUT PVOID ExtraConsoleInfo,
                IN ULONG ProcessId)
{
    PCONSOLE_START_INFO ConsoleStartInfo = ExtraConsoleInfo;
    PGUI_INIT_INFO GuiInitInfo;

    if (FrontEnd == NULL || ConsoleInfo == NULL || ConsoleStartInfo == NULL)
        return STATUS_INVALID_PARAMETER;

    /* Initialize GUI terminal emulator common functionalities */
    if (!GuiInit()) return STATUS_UNSUCCESSFUL;

    /*
     * Load per-application terminal settings.
     *
     * Check whether the process creating the console was launched via
     * a shell-link. ConsoleInfo->ConsoleTitle may be updated with the
     * name of the shortcut, and ConsoleStartInfo->Icon[Path|Index] too.
     */
    if (ConsoleStartInfo->dwStartupFlags & STARTF_TITLEISLINKNAME)
    {
        if (!LoadShellLinkConsoleInfo(ConsoleStartInfo, ConsoleInfo))
        {
            ConsoleStartInfo->dwStartupFlags &= ~STARTF_TITLEISLINKNAME;
        }
    }

    /*
     * Initialize a private initialization info structure for later use.
     * It must be freed by a call to GuiUnloadFrontEnd or GuiInitFrontEnd.
     */
    GuiInitInfo = ConsoleAllocHeap(HEAP_ZERO_MEMORY, sizeof(GUI_INIT_INFO));
    if (GuiInitInfo == NULL) return STATUS_NO_MEMORY;

    // HACK: We suppose that the pointers will be valid in GuiInitFrontEnd...
    GuiInitInfo->ConsoleInfo      = ConsoleInfo;
    GuiInitInfo->ConsoleStartInfo = ConsoleStartInfo;
    GuiInitInfo->ProcessId        = ProcessId;

    /* Finally, initialize the frontend structure */
    FrontEnd->Vtbl    = &GuiVtbl;
    FrontEnd->Data    = NULL;
    FrontEnd->OldData = GuiInitInfo;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
GuiUnloadFrontEnd(IN OUT PFRONTEND FrontEnd)
{
    if (FrontEnd == NULL) return STATUS_INVALID_PARAMETER;

    if (FrontEnd->Data)    GuiDeinitFrontEnd(FrontEnd);
    if (FrontEnd->OldData) ConsoleFreeHeap(FrontEnd->OldData);

    return STATUS_SUCCESS;
}

/* EOF */
