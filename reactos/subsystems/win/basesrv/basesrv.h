/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Base API Server DLL
 * FILE:            subsystems/win/basesrv/basesrv.h
 * PURPOSE:         Main header - Definitions
 * PROGRAMMERS:     Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#ifndef __BASESRV_H__
#define __BASESRV_H__

#pragma once

/* PSDK/NDK Headers */
#include <stdarg.h>
#define WIN32_NO_STATUS
#define _INC_WINDOWS
#define COM_NO_WINDOWS_H
#include <windef.h>
#include <winbase.h>
#include <winreg.h>
#define NTOS_MODE_USER
#include <ndk/ntndk.h>

/* PSEH for SEH Support */
#include <pseh/pseh2.h>

/* CSRSS Header */
#include <csr/csrsrv.h>

/* BASE Headers */
#include <win/basemsg.h>
#include <win/base.h>

typedef struct _BASESRV_KERNEL_IMPORTS
{
    PCHAR FunctionName;
    PVOID* FunctionPointer;
} BASESRV_KERNEL_IMPORTS, *PBASESRV_KERNEL_IMPORTS;

/* FIXME: BASENLS.H */
typedef NTSTATUS(*WINAPI POPEN_DATA_FILE)(HANDLE hFile,
                                          PWCHAR FileName);

typedef BOOL(*WINAPI PGET_CP_FILE_NAME_FROM_REGISTRY)(UINT   CodePage,
                                                      LPWSTR FileName,
                                                      ULONG  FileNameSize);

typedef BOOL(*WINAPI PGET_NLS_SECTION_NAME)(UINT   CodePage,
                                            UINT   Base,
                                            ULONG  Unknown,
                                            LPWSTR BaseName,
                                            LPWSTR Result,
                                            ULONG  ResultSize);

typedef BOOL(*WINAPI PVALIDATE_LOCALE)(IN ULONG LocaleId);
typedef NTSTATUS(*WINAPI PCREATE_NLS_SECURTY_DESCRIPTOR)(IN PVOID Buffer,
                                                         IN ULONG BufferSize,
                                                         IN ULONG AceType);

/* Globals */
extern HANDLE BaseSrvHeap;
extern HANDLE BaseSrvSharedHeap;
extern PBASE_STATIC_SERVER_DATA BaseStaticServerData;

#endif // __BASESRV_H__

/* EOF */
