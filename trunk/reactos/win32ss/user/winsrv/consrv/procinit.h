/*
 * LICENSE:         GPL - See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            win32ss/user/winsrv/consrv/procinit.h
 * PURPOSE:         Functions for console processes initialization
 * PROGRAMMERS:     Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#pragma once

NTSTATUS ConSrvAllocateConsole(PCONSOLE_PROCESS_DATA ProcessData,
                                        PHANDLE pInputHandle,
                                        PHANDLE pOutputHandle,
                                        PHANDLE pErrorHandle,
                                        PCONSOLE_START_INFO ConsoleStartInfo);
NTSTATUS ConSrvInheritConsole(PCONSOLE_PROCESS_DATA ProcessData,
                                       HANDLE ConsoleHandle,
                                       BOOL CreateNewHandlesTable,
                                       PHANDLE pInputHandle,
                                       PHANDLE pOutputHandle,
                                       PHANDLE pErrorHandle);
VOID ConSrvRemoveConsole(PCONSOLE_PROCESS_DATA ProcessData);
