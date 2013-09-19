/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS User API Server DLL
 * FILE:            include/reactos/subsys/win/winmsg.h
 * PURPOSE:         Public definitions for communication
 *                  between User-Mode API Clients and Servers
 * PROGRAMMERS:     Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

#ifndef _WINMSG_H
#define _WINMSG_H

#pragma once

#define USERSRV_SERVERDLL_INDEX     3
#define USERSRV_FIRST_API_NUMBER    1024

// Windows Server 2003 table from http://j00ru.vexillium.org/csrss_list/api_list.html#Windows_2k3
typedef enum _USERSRV_API_NUMBER
{
    UserpExitWindowsEx = USERSRV_FIRST_API_NUMBER,
    UserpEndTask,
    UserpLogon,
    UserpRegisterServicesProcess, // Not present in Win7
    UserpActivateDebugger,
    UserpGetThreadConsoleDesktop, // Not present in Win7
    UserpDeviceEvent,
    UserpRegisterLogonProcess,    // Not present in Win7
    UserpCreateSystemThreads,
    UserpRecordShutdownReason,
    // UserpCancelShutdown,              // Added in Vista
    // UserpConsoleHandleOperation,      // Added in Win7
    // UserpGetSetShutdownBlockReason,   // Added in Vista

    UserpMaxApiNumber
} USERSRV_API_NUMBER, *PUSERSRV_API_NUMBER;


typedef struct
{
    UINT Flags;
    DWORD Reserved;
} USER_EXIT_REACTOS, *PUSER_EXIT_REACTOS;

typedef struct
{
    ULONG_PTR ProcessId;
} USER_REGISTER_SERVICES_PROCESS, *PUSER_REGISTER_SERVICES_PROCESS;

typedef struct
{
    ULONG_PTR ProcessId;
    BOOL Register;
} USER_REGISTER_LOGON_PROCESS, *PUSER_REGISTER_LOGON_PROCESS;


typedef struct _USER_API_MESSAGE
{
    PORT_MESSAGE Header;

    PCSR_CAPTURE_BUFFER CsrCaptureData;
    CSR_API_NUMBER ApiNumber;
    NTSTATUS Status;
    ULONG Reserved;
    union
    {
        USER_EXIT_REACTOS ExitReactosRequest;
        USER_REGISTER_SERVICES_PROCESS RegisterServicesProcessRequest;
        USER_REGISTER_LOGON_PROCESS RegisterLogonProcessRequest;
    } Data;
} USER_API_MESSAGE, *PUSER_API_MESSAGE;

// Check that a USER_API_MESSAGE can hold in a CSR_API_MESSAGE.
CHECK_API_MSG_SIZE(USER_API_MESSAGE);

#endif // _WINMSG_H

/* EOF */
