/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            win32ss/user/winsrv/consrv/frontends/gui/guiterm.h
 * PURPOSE:         GUI Terminal Front-End
 * PROGRAMMERS:     G� van Geldorp
 *                  Johannes Anderwald
 *                  Jeffrey Morlan
 *                  Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#pragma once

#include "guisettings.h"
#include "conwnd.h"

NTSTATUS GuiInitConsole(PCONSOLE Console,
                                 /*IN*/ PCONSOLE_START_INFO ConsoleStartInfo,
                                 PCONSOLE_INFO ConsoleInfo,
                                 DWORD ProcessId,
                                 LPCWSTR IconPath,
                                 INT IconIndex);
