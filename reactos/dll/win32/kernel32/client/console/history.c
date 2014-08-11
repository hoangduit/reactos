/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS system libraries
 * FILE:            dll/win32/kernel32/client/console/history.c
 * PURPOSE:         Win32 Console Client history functions
 * PROGRAMMERS:     Jeffrey Morlan
 */

/* INCLUDES *******************************************************************/

#include <k32.h>

#define NDEBUG
#include <debug.h>


/* PRIVATE FUNCTIONS **********************************************************/

#if 0
/* Get the size needed to copy a string to a capture buffer, including alignment */
static ULONG
IntStringSize(LPCVOID String,
              BOOL Unicode)
{
    ULONG Size = (Unicode ? wcslen(String) : strlen(String)) * sizeof(WCHAR);
    return (Size + 3) & ~3;
}


/* Copy a string to a capture buffer */
static VOID
IntCaptureMessageString(PCSR_CAPTURE_BUFFER CaptureBuffer,
                        LPCVOID String,
                        BOOL Unicode,
                        PUNICODE_STRING RequestString)
{
    ULONG Size;
    if (Unicode)
    {
        Size = wcslen(String) * sizeof(WCHAR);
        CsrCaptureMessageBuffer(CaptureBuffer, (PVOID)String, Size, (PVOID *)&RequestString->Buffer);
    }
    else
    {
        Size = strlen(String);
        CsrAllocateMessagePointer(CaptureBuffer, Size * sizeof(WCHAR), (PVOID *)&RequestString->Buffer);
        Size = MultiByteToWideChar(CP_ACP, 0, String, Size, RequestString->Buffer, Size * sizeof(WCHAR))
               * sizeof(WCHAR);
    }
    RequestString->Length = RequestString->MaximumLength = (USHORT)Size;
}
#endif

static VOID
IntExpungeConsoleCommandHistory(LPCVOID lpExeName, BOOLEAN bUnicode)
{
    CONSOLE_API_MESSAGE ApiMessage;
    PCONSOLE_EXPUNGECOMMANDHISTORY ExpungeCommandHistoryRequest = &ApiMessage.Data.ExpungeCommandHistoryRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;

    USHORT NumChars = (USHORT)(lpExeName ? (bUnicode ? wcslen(lpExeName) : strlen(lpExeName)) : 0);

    if (lpExeName == NULL || NumChars == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return;
    }

    ExpungeCommandHistoryRequest->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    ExpungeCommandHistoryRequest->ExeLength     = NumChars * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
    ExpungeCommandHistoryRequest->Unicode  =
    ExpungeCommandHistoryRequest->Unicode2 = bUnicode;

    // CaptureBuffer = CsrAllocateCaptureBuffer(1, IntStringSize(lpExeName, bUnicode));
    CaptureBuffer = CsrAllocateCaptureBuffer(1, ExpungeCommandHistoryRequest->ExeLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    // IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
    //                         &ExpungeCommandHistoryRequest->ExeName);
    CsrCaptureMessageBuffer(CaptureBuffer,
                            (PVOID)lpExeName,
                            ExpungeCommandHistoryRequest->ExeLength,
                            (PVOID)&ExpungeCommandHistoryRequest->ExeName);

    CsrClientCallServer((PCSR_API_MESSAGE)&ApiMessage,
                        CaptureBuffer,
                        CSR_CREATE_API_NUMBER(CONSRV_SERVERDLL_INDEX, ConsolepExpungeCommandHistory),
                        sizeof(*ExpungeCommandHistoryRequest));

    CsrFreeCaptureBuffer(CaptureBuffer);

    if (!NT_SUCCESS(ApiMessage.Status))
        BaseSetLastNTError(ApiMessage.Status);
}


static DWORD
IntGetConsoleCommandHistory(LPVOID lpHistory, DWORD cbHistory, LPCVOID lpExeName, BOOLEAN bUnicode)
{
    CONSOLE_API_MESSAGE ApiMessage;
    PCONSOLE_GETCOMMANDHISTORY GetCommandHistoryRequest = &ApiMessage.Data.GetCommandHistoryRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;

    USHORT NumChars = (USHORT)(lpExeName ? (bUnicode ? wcslen(lpExeName) : strlen(lpExeName)) : 0);

    if (lpExeName == NULL || NumChars == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    GetCommandHistoryRequest->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    GetCommandHistoryRequest->HistoryLength = cbHistory;
    GetCommandHistoryRequest->ExeLength     = NumChars * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
    GetCommandHistoryRequest->Unicode  =
    GetCommandHistoryRequest->Unicode2 = bUnicode;

    // CaptureBuffer = CsrAllocateCaptureBuffer(2, IntStringSize(lpExeName, bUnicode) +
    //                                             HistoryLength);
    CaptureBuffer = CsrAllocateCaptureBuffer(2, GetCommandHistoryRequest->ExeLength +
                                                GetCommandHistoryRequest->HistoryLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    // IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
    //                         &GetCommandHistoryRequest->ExeName);
    CsrCaptureMessageBuffer(CaptureBuffer,
                            (PVOID)lpExeName,
                            GetCommandHistoryRequest->ExeLength,
                            (PVOID)&GetCommandHistoryRequest->ExeName);

    // CsrAllocateMessagePointer(CaptureBuffer, HistoryLength,
    CsrAllocateMessagePointer(CaptureBuffer, GetCommandHistoryRequest->HistoryLength,
                              (PVOID*)&GetCommandHistoryRequest->History);

    CsrClientCallServer((PCSR_API_MESSAGE)&ApiMessage,
                        CaptureBuffer,
                        CSR_CREATE_API_NUMBER(CONSRV_SERVERDLL_INDEX, ConsolepGetCommandHistory),
                        sizeof(*GetCommandHistoryRequest));
    if (!NT_SUCCESS(ApiMessage.Status))
    {
        CsrFreeCaptureBuffer(CaptureBuffer);
        BaseSetLastNTError(ApiMessage.Status);
        return 0;
    }

    RtlCopyMemory(lpHistory,
                  GetCommandHistoryRequest->History,
                  GetCommandHistoryRequest->HistoryLength);

    CsrFreeCaptureBuffer(CaptureBuffer);

    return GetCommandHistoryRequest->HistoryLength;
}


static DWORD
IntGetConsoleCommandHistoryLength(LPCVOID lpExeName, BOOL bUnicode)
{
    CONSOLE_API_MESSAGE ApiMessage;
    PCONSOLE_GETCOMMANDHISTORYLENGTH GetCommandHistoryLengthRequest = &ApiMessage.Data.GetCommandHistoryLengthRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;

    USHORT NumChars = (USHORT)(lpExeName ? (bUnicode ? wcslen(lpExeName) : strlen(lpExeName)) : 0);

    if (lpExeName == NULL || NumChars == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    GetCommandHistoryLengthRequest->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    GetCommandHistoryLengthRequest->ExeLength     = NumChars * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
    GetCommandHistoryLengthRequest->Unicode  =
    GetCommandHistoryLengthRequest->Unicode2 = bUnicode;

    // CaptureBuffer = CsrAllocateCaptureBuffer(1, IntStringSize(lpExeName, bUnicode));
    CaptureBuffer = CsrAllocateCaptureBuffer(1, GetCommandHistoryLengthRequest->ExeLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    // IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
    //                         &GetCommandHistoryLengthRequest->ExeName);
    CsrCaptureMessageBuffer(CaptureBuffer,
                            (PVOID)lpExeName,
                            GetCommandHistoryLengthRequest->ExeLength,
                            (PVOID)&GetCommandHistoryLengthRequest->ExeName);

    CsrClientCallServer((PCSR_API_MESSAGE)&ApiMessage,
                        CaptureBuffer,
                        CSR_CREATE_API_NUMBER(CONSRV_SERVERDLL_INDEX, ConsolepGetCommandHistoryLength),
                        sizeof(*GetCommandHistoryLengthRequest));

    CsrFreeCaptureBuffer(CaptureBuffer);

    if (!NT_SUCCESS(ApiMessage.Status))
    {
        BaseSetLastNTError(ApiMessage.Status);
        return 0;
    }

    return GetCommandHistoryLengthRequest->HistoryLength;
}


static BOOL
IntSetConsoleNumberOfCommands(DWORD dwNumCommands,
                              LPCVOID lpExeName,
                              BOOLEAN bUnicode)
{
    CONSOLE_API_MESSAGE ApiMessage;
    PCONSOLE_SETHISTORYNUMBERCOMMANDS SetHistoryNumberCommandsRequest = &ApiMessage.Data.SetHistoryNumberCommandsRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;

    USHORT NumChars = (USHORT)(lpExeName ? (bUnicode ? wcslen(lpExeName) : strlen(lpExeName)) : 0);

    if (lpExeName == NULL || NumChars == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SetHistoryNumberCommandsRequest->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    SetHistoryNumberCommandsRequest->NumCommands   = dwNumCommands;
    SetHistoryNumberCommandsRequest->ExeLength     = NumChars * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
    SetHistoryNumberCommandsRequest->Unicode  =
    SetHistoryNumberCommandsRequest->Unicode2 = bUnicode;

    // CaptureBuffer = CsrAllocateCaptureBuffer(1, IntStringSize(lpExeName, bUnicode));
    CaptureBuffer = CsrAllocateCaptureBuffer(1, SetHistoryNumberCommandsRequest->ExeLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    // IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
    //                         &SetHistoryNumberCommandsRequest->ExeName);
    CsrCaptureMessageBuffer(CaptureBuffer,
                            (PVOID)lpExeName,
                            SetHistoryNumberCommandsRequest->ExeLength,
                            (PVOID)&SetHistoryNumberCommandsRequest->ExeName);

    CsrClientCallServer((PCSR_API_MESSAGE)&ApiMessage,
                        CaptureBuffer,
                        CSR_CREATE_API_NUMBER(CONSRV_SERVERDLL_INDEX, ConsolepSetNumberOfCommands),
                        sizeof(*SetHistoryNumberCommandsRequest));

    CsrFreeCaptureBuffer(CaptureBuffer);

    if (!NT_SUCCESS(ApiMessage.Status))
    {
        BaseSetLastNTError(ApiMessage.Status);
        return FALSE;
    }

    return TRUE;
}


/* FUNCTIONS ******************************************************************/

/*
 * @implemented (Undocumented)
 */
VOID
WINAPI
ExpungeConsoleCommandHistoryW(LPCWSTR lpExeName)
{
    IntExpungeConsoleCommandHistory(lpExeName, TRUE);
}


/*
 * @implemented (Undocumented)
 */
VOID
WINAPI
ExpungeConsoleCommandHistoryA(LPCSTR lpExeName)
{
    IntExpungeConsoleCommandHistory(lpExeName, FALSE);
}


/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryW(LPWSTR lpHistory,
                          DWORD cbHistory,
                          LPCWSTR lpExeName)
{
    return IntGetConsoleCommandHistory(lpHistory, cbHistory, lpExeName, TRUE);
}


/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryA(LPSTR lpHistory,
                          DWORD cbHistory,
                          LPCSTR lpExeName)
{
    return IntGetConsoleCommandHistory(lpHistory, cbHistory, lpExeName, FALSE);
}


/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryLengthW(LPCWSTR lpExeName)
{
    return IntGetConsoleCommandHistoryLength(lpExeName, TRUE);
}


/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryLengthA(LPCSTR lpExeName)
{
    return IntGetConsoleCommandHistoryLength(lpExeName, FALSE);
}


/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleNumberOfCommandsW(DWORD dwNumCommands,
                            LPCWSTR lpExeName)
{
    return IntSetConsoleNumberOfCommands(dwNumCommands, lpExeName, TRUE);
}


/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleNumberOfCommandsA(DWORD dwNumCommands,
                            LPCSTR lpExeName)
{
    return IntSetConsoleNumberOfCommands(dwNumCommands, lpExeName, FALSE);
}


/*
 * @implemented
 */
BOOL
WINAPI
SetConsoleCommandHistoryMode(IN DWORD dwMode)
{
    CONSOLE_API_MESSAGE ApiMessage;
    PCONSOLE_SETHISTORYMODE SetHistoryModeRequest = &ApiMessage.Data.SetHistoryModeRequest;

    SetHistoryModeRequest->ConsoleHandle = NtCurrentPeb()->ProcessParameters->ConsoleHandle;
    SetHistoryModeRequest->Mode          = dwMode;

    CsrClientCallServer((PCSR_API_MESSAGE)&ApiMessage,
                        NULL,
                        CSR_CREATE_API_NUMBER(CONSRV_SERVERDLL_INDEX, ConsolepSetCommandHistoryMode),
                        sizeof(*SetHistoryModeRequest));
    if (!NT_SUCCESS(ApiMessage.Status))
    {
        BaseSetLastNTError(ApiMessage.Status);
        return FALSE;
    }

    return TRUE;
}

/* EOF */
