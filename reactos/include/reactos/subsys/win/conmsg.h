/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            include/reactos/subsys/win/conmsg.h
 * PURPOSE:         Public definitions for communication
 *                  between Console API Clients and Servers
 * PROGRAMMERS:     Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#ifndef _CONMSG_H
#define _CONMSG_H

#pragma once

#define CONSRV_SERVERDLL_INDEX      2
#define CONSRV_FIRST_API_NUMBER     512

// Windows Server 2003 table from http://j00ru.vexillium.org/csrss_list/api_list.html#Windows_2k3
typedef enum _CONSRV_API_NUMBER
{
    ConsolepOpenConsole = CONSRV_FIRST_API_NUMBER,
    ConsolepGetConsoleInput,
    ConsolepWriteConsoleInput,
    ConsolepReadConsoleOutput,
    ConsolepWriteConsoleOutput,
    ConsolepReadConsoleOutputString,
    ConsolepWriteConsoleOutputString,
    ConsolepFillConsoleOutput,
    ConsolepGetMode,
    ConsolepGetNumberOfFonts,
    ConsolepGetNumberOfInputEvents,
    ConsolepGetScreenBufferInfo,
    ConsolepGetCursorInfo,
    ConsolepGetMouseInfo,
    ConsolepGetFontInfo,
    ConsolepGetFontSize,
    ConsolepGetCurrentFont,
    ConsolepSetMode,
    ConsolepSetActiveScreenBuffer,
    ConsolepFlushInputBuffer,
    ConsolepGetLargestWindowSize,
    ConsolepSetScreenBufferSize,
    ConsolepSetCursorPosition,
    ConsolepSetCursorInfo,
    ConsolepSetWindowInfo,
    ConsolepScrollScreenBuffer,
    ConsolepSetTextAttribute,
    ConsolepSetFont,
    ConsolepSetIcon,
    ConsolepReadConsole,
    ConsolepWriteConsole,
    ConsolepDuplicateHandle,
    ConsolepGetHandleInformation,
    ConsolepSetHandleInformation,
    ConsolepCloseHandle,
    ConsolepVerifyIoHandle,
    ConsolepAlloc,                          // Not present in Win7
    ConsolepFree,                           // Not present in Win7
    ConsolepGetTitle,
    ConsolepSetTitle,
    ConsolepCreateScreenBuffer,
    ConsolepInvalidateBitMapRect,
    ConsolepVDMOperation,
    ConsolepSetCursor,
    ConsolepShowCursor,
    ConsolepMenuControl,
    ConsolepSetPalette,
    ConsolepSetDisplayMode,
    ConsolepRegisterVDM,
    ConsolepGetHardwareState,
    ConsolepSetHardwareState,
    ConsolepGetDisplayMode,
    ConsolepAddAlias,
    ConsolepGetAlias,
    ConsolepGetAliasesLength,
    ConsolepGetAliasExesLength,
    ConsolepGetAliases,
    ConsolepGetAliasExes,
    ConsolepExpungeCommandHistory,
    ConsolepSetNumberOfCommands,
    ConsolepGetCommandHistoryLength,
    ConsolepGetCommandHistory,
    ConsolepSetCommandHistoryMode,          // Not present in Vista+
    ConsolepGetCP,
    ConsolepSetCP,
    ConsolepSetKeyShortcuts,
    ConsolepSetMenuClose,
    ConsolepNotifyLastClose,
    ConsolepGenerateCtrlEvent,
    ConsolepGetKeyboardLayoutName,
    ConsolepGetConsoleWindow,
    ConsolepCharType,
    ConsolepSetLocalEUDC,
    ConsolepSetCursorMode,
    ConsolepGetCursorMode,
    ConsolepRegisterOS2,
    ConsolepSetOS2OemFormat,
    ConsolepGetNlsMode,
    ConsolepSetNlsMode,
    ConsolepRegisterConsoleIME,             // Not present in Win7
    ConsolepUnregisterConsoleIME,           // Not present in Win7
    // ConsolepQueryConsoleIME,                // Added only in Vista and Win2k8, not present in Win7
    ConsolepGetLangId,
    ConsolepAttach,                         // Not present in Win7
    ConsolepGetSelectionInfo,
    ConsolepGetProcessList,

    ConsolepGetHistory,                     // Added in Vista+
    ConsolepSetHistory,                     // Added in Vista+
    // ConsolepSetCurrentFont,                 // Added in Vista+
    // ConsolepSetScreenBufferInfo,            // Added in Vista+
    // ConsolepClientConnect,                  // Added in Win7

    ConsolepMaxApiNumber
} CONSRV_API_NUMBER, *PCONSRV_API_NUMBER;

//
// See http://msdn.microsoft.com/en-us/library/windows/desktop/bb773359(v=vs.85).aspx
//
typedef struct _CONSOLE_PROPERTIES
{
    WORD wFillAttribute;
    WORD wPopupFillAttribute;

    //
    // Not on MSDN, but show up in binary
    //
    WORD wShowWindow;
    WORD wUnknown;

    COORD dwScreenBufferSize;
    COORD dwWindowSize;
    COORD dwWindowOrigin;
    DWORD nFont;
    DWORD nInputBufferSize;
    COORD dwFontSize;
    UINT uFontFamily;
    UINT uFontWeight;
    WCHAR FaceName[LF_FACESIZE];
    UINT uCursorSize;
    BOOL bFullScreen;
    BOOL bQuickEdit;
    BOOL bInsertMode;
    BOOL bAutoPosition;
    UINT uHistoryBufferSize;
    UINT uNumberOfHistoryBuffers;
    BOOL bHistoryNoDup;
    COLORREF ColorTable[16];

    //NT_FE_CONSOLE_PROPS
    UINT uCodePage;
} CONSOLE_PROPERTIES;

//
// To minimize code changes, some fields were put here even though they really only belong in
// CONSRV_API_CONNECTINFO. Do not change the ordering however, as it's required for Windows
// compatibility.
//
typedef struct _CONSOLE_START_INFO
{
    INT IconIndex;
    HICON IconHandle1;
    HICON IconHandle2;
    DWORD dwHotKey;
    DWORD dwStartupFlags;
    CONSOLE_PROPERTIES;
    BOOLEAN ConsoleNeeded; // Used for GUI apps only.
    LPTHREAD_START_ROUTINE CtrlDispatcher;
    LPTHREAD_START_ROUTINE ImeDispatcher;
    LPTHREAD_START_ROUTINE PropDispatcher;
    ULONG TitleLength;
    WCHAR ConsoleTitle[MAX_PATH + 1];   // Console title or full path to the startup shortcut
    ULONG DesktopLength;
    PWCHAR DesktopPath;
    ULONG AppNameLength;
    WCHAR AppPath[128];        // Full path of the launched app
    ULONG IconPathLength;
    WCHAR IconPath[MAX_PATH + 1];       // Path to the file containing the icon
} CONSOLE_START_INFO, *PCONSOLE_START_INFO;

typedef struct _CONSRV_API_CONNECTINFO
{
    HANDLE ConsoleHandle;
    HANDLE InputWaitHandle;
    HANDLE InputHandle;
    HANDLE OutputHandle;
    HANDLE ErrorHandle;
    HANDLE Event1;
    HANDLE Event2;
    /* Adapted from CONSOLE_ALLOCCONSOLE */
    CONSOLE_START_INFO ConsoleStartInfo;
} CONSRV_API_CONNECTINFO, *PCONSRV_API_CONNECTINFO;

#if defined(_M_IX86)
C_ASSERT(sizeof(CONSRV_API_CONNECTINFO) == 0x638);
#endif

typedef struct
{
    ULONG  nMaxIds;
    ULONG  nProcessIdsTotal;
    PDWORD pProcessIds;
} CONSOLE_GETPROCESSLIST, *PCONSOLE_GETPROCESSLIST;

typedef struct
{
    HANDLE OutputHandle;

    BOOL Unicode;
    ULONG NrCharactersToWrite;
    ULONG NrCharactersWritten;

    ULONG BufferSize;
    PVOID Buffer;
} CONSOLE_WRITECONSOLE, *PCONSOLE_WRITECONSOLE;

typedef struct
{
    HANDLE InputHandle;

    BOOL Unicode;
    ULONG NrCharactersToRead;
    ULONG NrCharactersRead;

    UNICODE_STRING ExeName;
    DWORD CtrlWakeupMask;
    DWORD ControlKeyState;

    ULONG BufferSize;
    PVOID Buffer;
} CONSOLE_READCONSOLE, *PCONSOLE_READCONSOLE;

typedef struct
{
    PCONSOLE_START_INFO ConsoleStartInfo;

    HANDLE ConsoleHandle;
    HANDLE InputHandle;
    HANDLE OutputHandle;
    HANDLE ErrorHandle;
    HANDLE InputWaitHandle;
    LPTHREAD_START_ROUTINE CtrlDispatcher;
    LPTHREAD_START_ROUTINE PropDispatcher;
} CONSOLE_ALLOCCONSOLE, *PCONSOLE_ALLOCCONSOLE;

typedef struct
{
    DWORD ProcessId; // If ProcessId == ATTACH_PARENT_PROCESS == -1, then attach the current process to its parent process console.
    HANDLE ConsoleHandle;
    HANDLE InputHandle;
    HANDLE OutputHandle;
    HANDLE ErrorHandle;
    HANDLE InputWaitHandle;
    LPTHREAD_START_ROUTINE CtrlDispatcher;
    LPTHREAD_START_ROUTINE PropDispatcher;
} CONSOLE_ATTACHCONSOLE, *PCONSOLE_ATTACHCONSOLE;

typedef struct
{
    ULONG Dummy;
} CONSOLE_FREECONSOLE, *PCONSOLE_FREECONSOLE;

typedef struct
{
    HANDLE OutputHandle;
    CONSOLE_SCREEN_BUFFER_INFO Info;
} CONSOLE_GETSCREENBUFFERINFO, *PCONSOLE_GETSCREENBUFFERINFO;

typedef struct
{
    HANDLE OutputHandle;
    COORD Position;
} CONSOLE_SETCURSORPOSITION, *PCONSOLE_SETCURSORPOSITION;

typedef struct
{
    HANDLE OutputHandle;
    BOOL   Show;
    INT    RefCount;
} CONSOLE_SHOWCURSOR, *PCONSOLE_SHOWCURSOR;

typedef struct
{
    HANDLE  OutputHandle;
    HCURSOR hCursor;
} CONSOLE_SETCURSOR, *PCONSOLE_SETCURSOR;

typedef struct
{
    HANDLE OutputHandle;
    CONSOLE_CURSOR_INFO Info;
} CONSOLE_GETSETCURSORINFO, *PCONSOLE_GETSETCURSORINFO;

typedef struct
{
    HANDLE OutputHandle;
    WORD Attrib;
} CONSOLE_SETTEXTATTRIB, *PCONSOLE_SETTEXTATTRIB;

typedef struct
{
    HANDLE ConsoleHandle;   /* A valid input or output console handle */
    DWORD ConsoleMode;
} CONSOLE_GETSETCONSOLEMODE, *PCONSOLE_GETSETCONSOLEMODE;

typedef struct
{
    // HANDLE OutputHandle;
    DWORD  DisplayMode;
} CONSOLE_GETDISPLAYMODE, *PCONSOLE_GETDISPLAYMODE;

typedef struct
{
    HANDLE OutputHandle;
    DWORD  DisplayMode;
    COORD  NewSBDim;
} CONSOLE_SETDISPLAYMODE, *PCONSOLE_SETDISPLAYMODE;

/*
 * Console hardware states.
 */
#define CONSOLE_HARDWARE_STATE_GDI_MANAGED 0
#define CONSOLE_HARDWARE_STATE_DIRECT      1

typedef struct
{
    HANDLE OutputHandle;
    DWORD  State;
} CONSOLE_GETSETHWSTATE, *PCONSOLE_GETSETHWSTATE;


typedef struct
{
    HANDLE OutputHandle;     /* Handle to newly created screen buffer */
    DWORD  ScreenBufferType; /* Type of the screen buffer: CONSOLE_TEXTMODE_BUFFER or CONSOLE_GRAPHICS_BUFFER */
    /*
     * If we are creating a graphics screen buffer,
     * this structure holds the initialization information.
     */
    CONSOLE_GRAPHICS_BUFFER_INFO GraphicsBufferInfo;

    DWORD Access;
    DWORD ShareMode;
    BOOL  Inheritable;
} CONSOLE_CREATESCREENBUFFER, *PCONSOLE_CREATESCREENBUFFER;

typedef struct
{
    HANDLE OutputHandle;  /* Handle to screen buffer to switch to */
} CONSOLE_SETACTIVESCREENBUFFER, *PCONSOLE_SETACTIVESCREENBUFFER;

typedef struct
{
    HANDLE OutputHandle;
    SMALL_RECT Region;
} CONSOLE_INVALIDATEDIBITS, *PCONSOLE_INVALIDATEDIBITS;

typedef struct
{
    HANDLE   OutputHandle;
    HPALETTE PaletteHandle;
    UINT     Usage;
} CONSOLE_SETPALETTE, *PCONSOLE_SETPALETTE;

typedef struct
{
    DWORD Length;
    PWCHAR Title;
} CONSOLE_GETSETCONSOLETITLE, *PCONSOLE_GETSETCONSOLETITLE;

typedef struct
{
    HANDLE OutputHandle;

    BOOL Unicode;
    COORD BufferSize;
    COORD BufferCoord;
    SMALL_RECT WriteRegion;
    PCHAR_INFO CharInfo;
} CONSOLE_WRITEOUTPUT, *PCONSOLE_WRITEOUTPUT;

typedef struct
{
    HANDLE InputHandle;
} CONSOLE_FLUSHINPUTBUFFER, *PCONSOLE_FLUSHINPUTBUFFER;

typedef struct
{
    HANDLE OutputHandle;

    BOOL Unicode;
    SMALL_RECT ScrollRectangle;
    BOOL UseClipRectangle;
    SMALL_RECT ClipRectangle;
    COORD DestinationOrigin;
    CHAR_INFO Fill;
} CONSOLE_SCROLLSCREENBUFFER, *PCONSOLE_SCROLLSCREENBUFFER;


/*
 * An attribute or a character are instances of the same entity, namely
 * a "code" (what would be called an (ANSI) escape sequence). Therefore
 * encode them inside the same structure.
 */
typedef enum _CODE_TYPE
{
    CODE_ASCII      = 0x01,
    CODE_UNICODE    = 0x02,
    CODE_ATTRIBUTE  = 0x03
} CODE_TYPE;

typedef struct
{
    HANDLE OutputHandle;

    DWORD NumCodesToRead;
    COORD ReadCoord;
    COORD EndCoord;

    DWORD CodesRead;

    CODE_TYPE CodeType;
    union
    {
        PVOID  pCode;
        PCHAR  AsciiChar;
        PWCHAR UnicodeChar;
        PWORD  Attribute;
    } pCode;    // Either a pointer to a character or to an attribute.
} CONSOLE_READOUTPUTCODE, *PCONSOLE_READOUTPUTCODE;

typedef struct
{
    HANDLE OutputHandle;

    ULONG BufferSize; // Seems unusued
    WORD Length;
    COORD Coord;
    COORD EndCoord;

    ULONG NrCharactersWritten; // Seems unusued

    CODE_TYPE CodeType;
    union
    {
        PVOID  pCode;
        PCHAR  AsciiChar;
        PWCHAR UnicodeChar;
        PWORD  Attribute;
    } pCode;    // Either a pointer to a character or to an attribute.
} CONSOLE_WRITEOUTPUTCODE, *PCONSOLE_WRITEOUTPUTCODE;

typedef struct
{
    HANDLE OutputHandle;

    CODE_TYPE CodeType;
    union
    {
        CHAR  AsciiChar;
        WCHAR UnicodeChar;
        WORD  Attribute;
    } Code; // Either a character or an attribute.

    COORD Coord;
    ULONG Length;

    ULONG NrCharactersWritten; // FIXME: Only for chars, is it removable ?
} CONSOLE_FILLOUTPUTCODE, *PCONSOLE_FILLOUTPUTCODE;

typedef struct
{
    HANDLE InputHandle;
    ULONG InputsRead;
    PINPUT_RECORD InputRecord;
    ULONG Length;
    WORD wFlags;
    BOOLEAN Unicode;
} CONSOLE_GETINPUT, *PCONSOLE_GETINPUT;

typedef struct
{
    HANDLE OutputHandle;

    BOOL Unicode;
    COORD BufferSize;
    COORD BufferCoord;
    SMALL_RECT ReadRegion;
    PCHAR_INFO CharInfo;
} CONSOLE_READOUTPUT, *PCONSOLE_READOUTPUT;

typedef struct
{
    HANDLE InputHandle;
    DWORD Length;
    INPUT_RECORD* InputRecord;
    BOOL Unicode;
    BOOL AppendToEnd;
} CONSOLE_WRITEINPUT, *PCONSOLE_WRITEINPUT;

typedef struct
{
    HANDLE ConsoleHandle;
} CONSOLE_CLOSEHANDLE, *PCONSOLE_CLOSEHANDLE;

typedef struct
{
    HANDLE ConsoleHandle;
} CONSOLE_VERIFYHANDLE, *PCONSOLE_VERIFYHANDLE;

typedef struct
{
    HANDLE ConsoleHandle;
    DWORD Access;
    BOOL Inheritable;
    DWORD Options;
} CONSOLE_DUPLICATEHANDLE, *PCONSOLE_DUPLICATEHANDLE;

/*
 * Type of handles.
 */
typedef enum _CONSOLE_HANDLE_TYPE
{
    HANDLE_INPUT    = 0x01,
    HANDLE_OUTPUT   = 0x02
} CONSOLE_HANDLE_TYPE;

typedef struct
{
    HANDLE ConsoleHandle;
    CONSOLE_HANDLE_TYPE HandleType;
    DWORD Access;
    BOOL Inheritable;
    DWORD ShareMode;
} CONSOLE_OPENCONSOLE, *PCONSOLE_OPENCONSOLE;


typedef struct
{
    HANDLE OutputHandle;
    COORD  Size;
} CONSOLE_GETLARGESTWINDOWSIZE, *PCONSOLE_GETLARGESTWINDOWSIZE;

typedef struct
{
    HANDLE OutputHandle;
    DWORD  dwCmdIdLow;
    DWORD  dwCmdIdHigh;
    HMENU  hMenu;
} CONSOLE_MENUCONTROL, *PCONSOLE_MENUCONTROL;

typedef struct
{
    BOOL Enable;
} CONSOLE_SETMENUCLOSE, *PCONSOLE_SETMENUCLOSE;

typedef struct
{
    HANDLE OutputHandle;
    BOOL   Absolute;
    SMALL_RECT WindowRect; // New console window position in the screen-buffer frame (Absolute == TRUE)
                           // or in the old window position frame (Absolute == FALSE).
} CONSOLE_SETWINDOWINFO, *PCONSOLE_SETWINDOWINFO;

typedef struct
{
    HWND WindowHandle;
} CONSOLE_GETWINDOW, *PCONSOLE_GETWINDOW;

typedef struct
{
    HICON WindowIcon;
} CONSOLE_SETICON, *PCONSOLE_SETICON;



typedef struct
{
    ULONG  SourceLength;
    ULONG  TargetLength; // Also used for storing the number of bytes written.
    ULONG  ExeLength;
    LPWSTR Source;
    LPWSTR Target;
    LPWSTR Exe;
} CONSOLE_ADDGETALIAS, *PCONSOLE_ADDGETALIAS;

typedef struct
{
    DWORD ExeLength;
    DWORD AliasesBufferLength;
    LPWSTR ExeName;
    LPWSTR AliasesBuffer;
} CONSOLE_GETALLALIASES, *PCONSOLE_GETALLALIASES;

typedef struct
{
    DWORD Length;
    DWORD ExeLength;
    LPWSTR ExeName;
} CONSOLE_GETALLALIASESLENGTH, *PCONSOLE_GETALLALIASESLENGTH;

typedef struct
{
    DWORD Length;
    LPWSTR ExeNames;
} CONSOLE_GETALIASESEXES, *PCONSOLE_GETALIASESEXES;

typedef struct
{
    DWORD Length;
} CONSOLE_GETALIASESEXESLENGTH, *PCONSOLE_GETALIASESEXESLENGTH;



typedef struct
{
    UNICODE_STRING ExeName;
    PWCHAR History;
    DWORD Length;
} CONSOLE_GETCOMMANDHISTORY, *PCONSOLE_GETCOMMANDHISTORY;

typedef struct
{
    UNICODE_STRING ExeName;
    DWORD Length;
} CONSOLE_GETCOMMANDHISTORYLENGTH, *PCONSOLE_GETCOMMANDHISTORYLENGTH;

typedef struct
{
    UNICODE_STRING ExeName;
} CONSOLE_EXPUNGECOMMANDHISTORY, *PCONSOLE_EXPUNGECOMMANDHISTORY;

typedef struct
{
    UNICODE_STRING ExeName;
    DWORD NumCommands;
} CONSOLE_SETHISTORYNUMBERCOMMANDS, *PCONSOLE_SETHISTORYNUMBERCOMMANDS;

typedef struct
{
    UINT HistoryBufferSize;
    UINT NumberOfHistoryBuffers;
    DWORD dwFlags;
} CONSOLE_GETSETHISTORYINFO, *PCONSOLE_GETSETHISTORYINFO;



typedef struct
{
    DWORD Event;
    DWORD ProcessGroup;
} CONSOLE_GENERATECTRLEVENT, *PCONSOLE_GENERATECTRLEVENT;

typedef struct
{
    HANDLE InputHandle;
    DWORD NumInputEvents;
} CONSOLE_GETNUMINPUTEVENTS, *PCONSOLE_GETNUMINPUTEVENTS;

typedef struct
{
    HANDLE OutputHandle;
    COORD Size;
} CONSOLE_SETSCREENBUFFERSIZE, *PCONSOLE_SETSCREENBUFFERSIZE;

typedef struct
{
    CONSOLE_SELECTION_INFO Info;
} CONSOLE_GETSELECTIONINFO, *PCONSOLE_GETSELECTIONINFO;

typedef struct
{
    BOOL InputCP;   // TRUE : Input Code Page ; FALSE : Output Code Page
    UINT CodePage;
} CONSOLE_GETSETINPUTOUTPUTCP, *PCONSOLE_GETSETINPUTOUTPUTCP;

typedef struct _CONSOLE_API_MESSAGE
{
    PORT_MESSAGE Header;

    PCSR_CAPTURE_BUFFER CsrCaptureData;
    CSR_API_NUMBER ApiNumber;
    NTSTATUS Status;
    ULONG Reserved;
    union
    {
        /* Console initialization and uninitialization */
        CONSOLE_ALLOCCONSOLE AllocConsoleRequest;
        CONSOLE_ATTACHCONSOLE AttachConsoleRequest;
        CONSOLE_FREECONSOLE FreeConsoleRequest;

        /* Process list */
        CONSOLE_GETPROCESSLIST GetProcessListRequest;

        /* Handles */
        CONSOLE_OPENCONSOLE OpenConsoleRequest;
        CONSOLE_CLOSEHANDLE CloseHandleRequest;
        CONSOLE_VERIFYHANDLE VerifyHandleRequest;
        CONSOLE_DUPLICATEHANDLE DuplicateHandleRequest;

        /* Cursor */
        CONSOLE_SHOWCURSOR ShowCursorRequest;
        CONSOLE_SETCURSOR SetCursorRequest;
        CONSOLE_GETSETCURSORINFO CursorInfoRequest;
        CONSOLE_SETCURSORPOSITION SetCursorPositionRequest;

        /* Screen-buffer */
        CONSOLE_CREATESCREENBUFFER CreateScreenBufferRequest;
        CONSOLE_SETACTIVESCREENBUFFER SetScreenBufferRequest;
        CONSOLE_GETSCREENBUFFERINFO ScreenBufferInfoRequest;
        CONSOLE_SETSCREENBUFFERSIZE SetScreenBufferSizeRequest;
        CONSOLE_SCROLLSCREENBUFFER ScrollScreenBufferRequest;

        CONSOLE_GETSELECTIONINFO GetSelectionInfoRequest;
        CONSOLE_FLUSHINPUTBUFFER FlushInputBufferRequest;

        /* Console mode */
        CONSOLE_GETSETCONSOLEMODE ConsoleModeRequest;
        CONSOLE_GETDISPLAYMODE GetDisplayModeRequest;
        CONSOLE_SETDISPLAYMODE SetDisplayModeRequest;
        CONSOLE_GETSETHWSTATE HardwareStateRequest;

        /* Console window */
        CONSOLE_INVALIDATEDIBITS InvalidateDIBitsRequest;
        CONSOLE_SETPALETTE SetPaletteRequest;
        CONSOLE_GETSETCONSOLETITLE TitleRequest;
        CONSOLE_GETLARGESTWINDOWSIZE GetLargestWindowSizeRequest;
        CONSOLE_MENUCONTROL MenuControlRequest;
        CONSOLE_SETMENUCLOSE SetMenuCloseRequest;
        CONSOLE_SETWINDOWINFO SetWindowInfoRequest;
        CONSOLE_GETWINDOW GetWindowRequest;
        CONSOLE_SETICON SetIconRequest;

        /* Read */
        CONSOLE_READCONSOLE ReadConsoleRequest;         // SrvReadConsole / ReadConsole
        CONSOLE_GETINPUT GetInputRequest;               // SrvGetConsoleInput / PeekConsoleInput & ReadConsoleInput
        CONSOLE_READOUTPUT ReadOutputRequest;           // SrvReadConsoleOutput / ReadConsoleOutput
        CONSOLE_READOUTPUTCODE ReadOutputCodeRequest;   // SrvReadConsoleOutputString / ReadConsoleOutputAttribute & ReadConsoleOutputCharacter

        /* Write */
        CONSOLE_WRITECONSOLE WriteConsoleRequest;       // SrvWriteConsole / WriteConsole
        CONSOLE_WRITEINPUT WriteInputRequest;
        CONSOLE_WRITEOUTPUT WriteOutputRequest;
        CONSOLE_WRITEOUTPUTCODE WriteOutputCodeRequest;

        CONSOLE_FILLOUTPUTCODE FillOutputRequest;
        CONSOLE_SETTEXTATTRIB SetTextAttribRequest;

        /* Aliases */
        CONSOLE_ADDGETALIAS ConsoleAliasRequest;
        CONSOLE_GETALLALIASES GetAllAliasesRequest;
        CONSOLE_GETALLALIASESLENGTH GetAllAliasesLengthRequest;
        CONSOLE_GETALIASESEXES GetAliasesExesRequest;
        CONSOLE_GETALIASESEXESLENGTH GetAliasesExesLengthRequest;

        /* History */
        CONSOLE_GETCOMMANDHISTORY GetCommandHistoryRequest;
        CONSOLE_GETCOMMANDHISTORYLENGTH GetCommandHistoryLengthRequest;
        CONSOLE_EXPUNGECOMMANDHISTORY ExpungeCommandHistoryRequest;
        CONSOLE_SETHISTORYNUMBERCOMMANDS SetHistoryNumberCommandsRequest;
        CONSOLE_GETSETHISTORYINFO HistoryInfoRequest;

        CONSOLE_GENERATECTRLEVENT GenerateCtrlEventRequest;
        CONSOLE_GETNUMINPUTEVENTS GetNumInputEventsRequest;

        /* Input and Output Code Pages */
        CONSOLE_GETSETINPUTOUTPUTCP ConsoleCPRequest;
    } Data;
} CONSOLE_API_MESSAGE, *PCONSOLE_API_MESSAGE;

// Check that a CONSOLE_API_MESSAGE can hold in a CSR_API_MESSAGE.
CHECK_API_MSG_SIZE(CONSOLE_API_MESSAGE);

#endif // _CONMSG_H

/* EOF */
