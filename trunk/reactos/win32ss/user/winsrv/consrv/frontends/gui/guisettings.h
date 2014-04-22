/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            win32ss/user/winsrv/consrv/guisettings.h
 * PURPOSE:         GUI front-end settings management
 * PROGRAMMERS:     Johannes Anderwald
 *                  Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 *
 * NOTE: Also used by console.dll
 */

#pragma once

#ifndef WM_APP
    #define WM_APP 0x8000
#endif
#define PM_APPLY_CONSOLE_INFO (WM_APP + 100)

/* STRUCTURES *****************************************************************/

typedef struct _GUI_CONSOLE_INFO
{
    // FONTSIGNATURE FontSignature;
    WCHAR FaceName[LF_FACESIZE];
    UINT  FontFamily;
    DWORD FontSize;
    DWORD FontWeight;
    BOOL  UseRasterFonts;

    BOOL  FullScreen;       /* Whether the console is displayed in full-screen or windowed mode */
//  ULONG HardwareState;    /* _GDI_MANAGED, _DIRECT */

    WORD  ShowWindow;
    BOOL  AutoPosition;
    POINT WindowOrigin;
} GUI_CONSOLE_INFO, *PGUI_CONSOLE_INFO;

#ifndef CONSOLE_H__ // If we aren't included by console.dll

#include "conwnd.h"

/* FUNCTIONS ******************************************************************/

BOOL
GuiConsoleReadUserSettings(IN OUT PGUI_CONSOLE_INFO TermInfo,
                           IN LPCWSTR ConsoleTitle,
                           IN DWORD ProcessId);
BOOL
GuiConsoleWriteUserSettings(IN OUT PGUI_CONSOLE_INFO TermInfo,
                            IN LPCWSTR ConsoleTitle,
                            IN DWORD ProcessId);
VOID
GuiConsoleGetDefaultSettings(IN OUT PGUI_CONSOLE_INFO TermInfo,
                             IN DWORD ProcessId);
VOID
GuiConsoleShowConsoleProperties(PGUI_CONSOLE_DATA GuiData,
                                BOOL Defaults);
VOID
GuiApplyUserSettings(PGUI_CONSOLE_DATA GuiData,
                     HANDLE hClientSection,
                     BOOL SaveSettings);

#endif

/* EOF */
