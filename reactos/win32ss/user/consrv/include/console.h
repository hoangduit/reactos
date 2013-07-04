/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            win32ss/user/consrv/include/console.h
 * PURPOSE:         Public Console Management Interface
 * PROGRAMMERS:     Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#pragma once

BOOLEAN NTAPI
ConDrvValidateConsolePointer(IN PCONSOLE Console);

BOOLEAN NTAPI
ConDrvValidateConsoleState(IN PCONSOLE Console,
                           IN CONSOLE_STATE ExpectedState);

BOOLEAN NTAPI
ConDrvValidateConsoleUnsafe(IN PCONSOLE Console,
                            IN CONSOLE_STATE ExpectedState,
                            IN BOOLEAN LockConsole);

BOOLEAN NTAPI
ConDrvValidateConsole(IN PCONSOLE Console,
                      IN CONSOLE_STATE ExpectedState,
                      IN BOOLEAN LockConsole);

/* EOF */
