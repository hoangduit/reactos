/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Console Server DLL
 * FILE:            win32ss/user/winsrv/consrv/coninput.c
 * PURPOSE:         Console Input functions
 * PROGRAMMERS:     Jeffrey Morlan
 *                  Hermes Belusca-Maito (hermes.belusca@sfr.fr)
 */

/* INCLUDES *******************************************************************/

#include "consrv.h"

#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

#define ConSrvGetInputBuffer(ProcessData, Handle, Ptr, Access, LockConsole)     \
    ConSrvGetObject((ProcessData), (Handle), (PCONSOLE_IO_OBJECT*)(Ptr), NULL,  \
                    (Access), (LockConsole), INPUT_BUFFER)
#define ConSrvGetInputBufferAndHandleEntry(ProcessData, Handle, Ptr, Entry, Access, LockConsole)    \
    ConSrvGetObject((ProcessData), (Handle), (PCONSOLE_IO_OBJECT*)(Ptr), (Entry),                   \
                    (Access), (LockConsole), INPUT_BUFFER)
#define ConSrvReleaseInputBuffer(Buff, IsConsoleLocked) \
    ConSrvReleaseObject(&(Buff)->Header, (IsConsoleLocked))


typedef struct _GET_INPUT_INFO
{
    PCSR_THREAD           CallingThread;    // The thread which called the input API.
    PVOID                 HandleEntry;      // The handle data associated with the wait thread.
    PCONSOLE_INPUT_BUFFER InputBuffer;      // The input buffer corresponding to the handle.
} GET_INPUT_INFO, *PGET_INPUT_INFO;


/* PRIVATE FUNCTIONS **********************************************************/

/*
 * This pre-processing code MUST be IN consrv ONLY
 */
/* static */ ULONG
PreprocessInput(PCONSOLE Console,
                PINPUT_RECORD InputEvent,
                ULONG NumEventsToWrite)
{
    ULONG NumEvents;

    /*
     * Loop each event, and for each, check for pause or unpause
     * and perform adequate behaviour.
     */
    for (NumEvents = NumEventsToWrite; NumEvents > 0; --NumEvents)
    {
        /* Check for pause or unpause */
        if (InputEvent->EventType == KEY_EVENT && InputEvent->Event.KeyEvent.bKeyDown)
        {
            WORD vk = InputEvent->Event.KeyEvent.wVirtualKeyCode;
            if (!(Console->PauseFlags & PAUSED_FROM_KEYBOARD))
            {
                DWORD cks = InputEvent->Event.KeyEvent.dwControlKeyState;
                if (Console->InputBuffer.Mode & ENABLE_LINE_INPUT &&
                    (vk == VK_PAUSE ||
                    (vk == 'S' && (cks & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) &&
                                 !(cks & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED)))))
                {
                    ConioPause(Console, PAUSED_FROM_KEYBOARD);

                    /* Skip the event */
                    RtlMoveMemory(InputEvent,
                                  InputEvent + 1,
                                  (NumEvents - 1) * sizeof(INPUT_RECORD));
                    --NumEventsToWrite;
                    continue;
                }
            }
            else
            {
                if ((vk < VK_SHIFT || vk > VK_CAPITAL) && vk != VK_LWIN &&
                    vk != VK_RWIN && vk != VK_NUMLOCK && vk != VK_SCROLL)
                {
                    ConioUnpause(Console, PAUSED_FROM_KEYBOARD);

                    /* Skip the event */
                    RtlMoveMemory(InputEvent,
                                  InputEvent + 1,
                                  (NumEvents - 1) * sizeof(INPUT_RECORD));
                    --NumEventsToWrite;
                    continue;
                }
            }
        }

        /* Go to the next event */
        ++InputEvent;
    }

    return NumEventsToWrite;
}

/*
 * This post-processing code MUST be IN consrv ONLY
 */
/* static */ VOID
PostprocessInput(PCONSOLE Console)
{
    CsrNotifyWait(&Console->ReadWaitQueue,
                  FALSE,
                  NULL,
                  NULL);
    if (!IsListEmpty(&Console->ReadWaitQueue))
    {
        CsrDereferenceWait(&Console->ReadWaitQueue);
    }
}





static NTSTATUS
WaitBeforeReading(IN PGET_INPUT_INFO InputInfo,
                  IN PCSR_API_MESSAGE ApiMessage,
                  IN CSR_WAIT_FUNCTION WaitFunction OPTIONAL,
                  IN BOOLEAN CreateWaitBlock OPTIONAL)
{
    if (CreateWaitBlock)
    {
        PGET_INPUT_INFO CapturedInputInfo;

        CapturedInputInfo = ConsoleAllocHeap(0, sizeof(GET_INPUT_INFO));
        if (!CapturedInputInfo) return STATUS_NO_MEMORY;

        RtlMoveMemory(CapturedInputInfo, InputInfo, sizeof(GET_INPUT_INFO));

        if (!CsrCreateWait(&InputInfo->InputBuffer->Header.Console->ReadWaitQueue,
                           WaitFunction,
                           InputInfo->CallingThread,
                           ApiMessage,
                           CapturedInputInfo))
        {
            ConsoleFreeHeap(CapturedInputInfo);
            return STATUS_NO_MEMORY;
        }
    }

    /* Wait for input */
    return STATUS_PENDING;
}

static NTSTATUS
ReadChars(IN PGET_INPUT_INFO InputInfo,
          IN PCSR_API_MESSAGE ApiMessage,
          IN BOOLEAN CreateWaitBlock OPTIONAL);

// Wait function CSR_WAIT_FUNCTION
static BOOLEAN
NTAPI
ReadCharsThread(IN PLIST_ENTRY WaitList,
                IN PCSR_THREAD WaitThread,
                IN PCSR_API_MESSAGE WaitApiMessage,
                IN PVOID WaitContext,
                IN PVOID WaitArgument1,
                IN PVOID WaitArgument2,
                IN ULONG WaitFlags)
{
    NTSTATUS Status;
    PGET_INPUT_INFO InputInfo = (PGET_INPUT_INFO)WaitContext;

    PVOID InputHandle = WaitArgument2;

    DPRINT("ReadCharsThread - WaitContext = 0x%p, WaitArgument1 = 0x%p, WaitArgument2 = 0x%p, WaitFlags = %lu\n", WaitContext, WaitArgument1, WaitArgument2, WaitFlags);

    /*
     * If we are notified of the process termination via a call
     * to CsrNotifyWaitBlock triggered by CsrDestroyProcess or
     * CsrDestroyThread, just return.
     */
    if (WaitFlags & CsrProcessTerminating)
    {
        Status = STATUS_THREAD_IS_TERMINATING;
        goto Quit;
    }

    /*
     * Somebody is closing a handle to this input buffer,
     * by calling ConSrvCloseHandleEntry.
     * See whether we are linked to that handle (ie. we
     * are a waiter for this handle), and if so, return.
     * Otherwise, ignore the call and continue waiting.
     */
    if (InputHandle != NULL)
    {
        Status = (InputHandle == InputInfo->HandleEntry ? STATUS_ALERTED
                                                        : STATUS_PENDING);
        goto Quit;
    }

    /*
     * If we go there, that means we are notified for some new input.
     * The console is therefore already locked.
     */
    Status = ReadChars(InputInfo, WaitApiMessage, FALSE);

Quit:
    if (Status != STATUS_PENDING)
    {
        WaitApiMessage->Status = Status;
        ConsoleFreeHeap(InputInfo);
    }

    return (Status == STATUS_PENDING ? FALSE : TRUE);
}

NTSTATUS NTAPI
ConDrvReadConsole(IN PCONSOLE Console,
                  IN PCONSOLE_INPUT_BUFFER InputBuffer,
                  /**/IN PUNICODE_STRING ExeName /**/OPTIONAL/**/,/**/
                  IN BOOLEAN Unicode,
                  OUT PVOID Buffer,
                  IN OUT PCONSOLE_READCONSOLE_CONTROL ReadControl,
                  IN ULONG NumCharsToRead,
                  OUT PULONG NumCharsRead OPTIONAL);
static NTSTATUS
ReadChars(IN PGET_INPUT_INFO InputInfo,
          IN PCSR_API_MESSAGE ApiMessage,
          IN BOOLEAN CreateWaitBlock OPTIONAL)
{
    NTSTATUS Status;
    PCONSOLE_READCONSOLE ReadConsoleRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.ReadConsoleRequest;
    PCONSOLE_INPUT_BUFFER InputBuffer = InputInfo->InputBuffer;
    CONSOLE_READCONSOLE_CONTROL ReadControl;

    UNICODE_STRING ExeName;

    PVOID Buffer;
    ULONG NrCharactersRead = 0;
    ULONG CharSize = (ReadConsoleRequest->Unicode ? sizeof(WCHAR) : sizeof(CHAR));

    /* Compute the executable name, if needed */
    if (ReadConsoleRequest->InitialNumBytes == 0 &&
        ReadConsoleRequest->ExeLength <= sizeof(ReadConsoleRequest->StaticBuffer))
    {
        ExeName.Length = ExeName.MaximumLength = ReadConsoleRequest->ExeLength;
        ExeName.Buffer = (PWCHAR)ReadConsoleRequest->StaticBuffer;
    }
    else
    {
        ExeName.Length = ExeName.MaximumLength = 0;
        ExeName.Buffer = NULL;
    }

    /* Build the ReadControl structure */
    ReadControl.nLength           = sizeof(CONSOLE_READCONSOLE_CONTROL);
    ReadControl.nInitialChars     = ReadConsoleRequest->InitialNumBytes / CharSize;
    ReadControl.dwCtrlWakeupMask  = ReadConsoleRequest->CtrlWakeupMask;
    ReadControl.dwControlKeyState = ReadConsoleRequest->ControlKeyState;

    /*
     * For optimization purposes, Windows (and hence ReactOS, too, for
     * compatibility reasons) uses a static buffer if no more than eighty
     * bytes are read. Otherwise a new buffer is used.
     * The client-side expects that we know this behaviour.
     */
    if (ReadConsoleRequest->CaptureBufferSize <= sizeof(ReadConsoleRequest->StaticBuffer))
    {
        /*
         * Adjust the internal pointer, because its old value points to
         * the static buffer in the original ApiMessage structure.
         */
        // ReadConsoleRequest->Buffer = ReadConsoleRequest->StaticBuffer;
        Buffer = ReadConsoleRequest->StaticBuffer;
    }
    else
    {
        Buffer = ReadConsoleRequest->Buffer;
    }

    DPRINT1("Calling ConDrvReadConsole(%wZ)\n", &ExeName);
    Status = ConDrvReadConsole(InputBuffer->Header.Console,
                               InputBuffer,
                               &ExeName,
                               ReadConsoleRequest->Unicode,
                               Buffer,
                               &ReadControl,
                               ReadConsoleRequest->NumBytes / CharSize, // NrCharactersToRead
                               &NrCharactersRead);
    DPRINT1("ConDrvReadConsole returned (%d ; Status = 0x%08x)\n",
           NrCharactersRead, Status);

    // ReadConsoleRequest->ControlKeyState = ReadControl.dwControlKeyState;

    if (Status == STATUS_PENDING)
    {
        /* We haven't completed a read, so start a wait */
        return WaitBeforeReading(InputInfo,
                                 ApiMessage,
                                 ReadCharsThread,
                                 CreateWaitBlock);
    }
    else
    {
        /*
         * We read all what we wanted. Set the number of bytes read and
         * return the error code we were given.
         */
        ReadConsoleRequest->NumBytes = NrCharactersRead * CharSize;
        ReadConsoleRequest->ControlKeyState = ReadControl.dwControlKeyState;

        return Status;
        // return STATUS_SUCCESS;
    }
}

static NTSTATUS
ReadInputBuffer(IN PGET_INPUT_INFO InputInfo,
                IN PCSR_API_MESSAGE ApiMessage,
                IN BOOLEAN CreateWaitBlock OPTIONAL);

// Wait function CSR_WAIT_FUNCTION
static BOOLEAN
NTAPI
ReadInputBufferThread(IN PLIST_ENTRY WaitList,
                      IN PCSR_THREAD WaitThread,
                      IN PCSR_API_MESSAGE WaitApiMessage,
                      IN PVOID WaitContext,
                      IN PVOID WaitArgument1,
                      IN PVOID WaitArgument2,
                      IN ULONG WaitFlags)
{
    NTSTATUS Status;
    PGET_INPUT_INFO InputInfo = (PGET_INPUT_INFO)WaitContext;

    PVOID InputHandle = WaitArgument2;

    DPRINT("ReadInputBufferThread - WaitContext = 0x%p, WaitArgument1 = 0x%p, WaitArgument2 = 0x%p, WaitFlags = %lu\n", WaitContext, WaitArgument1, WaitArgument2, WaitFlags);

    /*
     * If we are notified of the process termination via a call
     * to CsrNotifyWaitBlock triggered by CsrDestroyProcess or
     * CsrDestroyThread, just return.
     */
    if (WaitFlags & CsrProcessTerminating)
    {
        Status = STATUS_THREAD_IS_TERMINATING;
        goto Quit;
    }

    /*
     * Somebody is closing a handle to this input buffer,
     * by calling ConSrvCloseHandleEntry.
     * See whether we are linked to that handle (ie. we
     * are a waiter for this handle), and if so, return.
     * Otherwise, ignore the call and continue waiting.
     */
    if (InputHandle != NULL)
    {
        Status = (InputHandle == InputInfo->HandleEntry ? STATUS_ALERTED
                                                        : STATUS_PENDING);
        goto Quit;
    }

    /*
     * If we go there, that means we are notified for some new input.
     * The console is therefore already locked.
     */
    Status = ReadInputBuffer(InputInfo, WaitApiMessage, FALSE);

Quit:
    if (Status != STATUS_PENDING)
    {
        WaitApiMessage->Status = Status;
        ConsoleFreeHeap(InputInfo);
    }

    return (Status == STATUS_PENDING ? FALSE : TRUE);
}

NTSTATUS NTAPI
ConDrvGetConsoleInput(IN PCONSOLE Console,
                      IN PCONSOLE_INPUT_BUFFER InputBuffer,
                      IN BOOLEAN KeepEvents,
                      IN BOOLEAN WaitForMoreEvents,
                      IN BOOLEAN Unicode,
                      OUT PINPUT_RECORD InputRecord,
                      IN ULONG NumEventsToRead,
                      OUT PULONG NumEventsRead OPTIONAL);
static NTSTATUS
ReadInputBuffer(IN PGET_INPUT_INFO InputInfo,
                IN PCSR_API_MESSAGE ApiMessage,
                IN BOOLEAN CreateWaitBlock OPTIONAL)
{
    NTSTATUS Status;
    PCONSOLE_GETINPUT GetInputRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.GetInputRequest;
    PCONSOLE_INPUT_BUFFER InputBuffer = InputInfo->InputBuffer;
    ULONG NumEventsRead;

    PINPUT_RECORD InputRecord;

    /*
     * For optimization purposes, Windows (and hence ReactOS, too, for
     * compatibility reasons) uses a static buffer if no more than five
     * input records are read. Otherwise a new buffer is used.
     * The client-side expects that we know this behaviour.
     */
    if (GetInputRequest->NumRecords <= sizeof(GetInputRequest->RecordStaticBuffer)/sizeof(INPUT_RECORD))
    {
        /*
         * Adjust the internal pointer, because its old value points to
         * the static buffer in the original ApiMessage structure.
         */
        // GetInputRequest->RecordBufPtr = GetInputRequest->RecordStaticBuffer;
        InputRecord = GetInputRequest->RecordStaticBuffer;
    }
    else
    {
        InputRecord = GetInputRequest->RecordBufPtr;
    }

    NumEventsRead = 0;
    Status = ConDrvGetConsoleInput(InputBuffer->Header.Console,
                                   InputBuffer,
                                   (GetInputRequest->Flags & CONSOLE_READ_KEEPEVENT) != 0,
                                   (GetInputRequest->Flags & CONSOLE_READ_CONTINUE ) == 0,
                                   GetInputRequest->Unicode,
                                   InputRecord,
                                   GetInputRequest->NumRecords,
                                   &NumEventsRead);

    if (Status == STATUS_PENDING)
    {
        /* We haven't completed a read, so start a wait */
        return WaitBeforeReading(InputInfo,
                                 ApiMessage,
                                 ReadInputBufferThread,
                                 CreateWaitBlock);
    }
    else
    {
        /*
         * We read all what we wanted. Set the number of events read and
         * return the error code we were given.
         */
        GetInputRequest->NumRecords = NumEventsRead;

        return Status;
        // return STATUS_SUCCESS;
    }
}


/* PUBLIC SERVER APIS *********************************************************/

CSR_API(SrvReadConsole)
{
    NTSTATUS Status;
    PCONSOLE_READCONSOLE ReadConsoleRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.ReadConsoleRequest;
    PCONSOLE_PROCESS_DATA ProcessData = ConsoleGetPerProcessData(CsrGetClientThread()->Process);
    PVOID HandleEntry;
    PCONSOLE_INPUT_BUFFER InputBuffer;
    GET_INPUT_INFO InputInfo;

    DPRINT("SrvReadConsole\n");

    /*
     * For optimization purposes, Windows (and hence ReactOS, too, for
     * compatibility reasons) uses a static buffer if no more than eighty
     * bytes are read. Otherwise a new buffer is used.
     * The client-side expects that we know this behaviour.
     */
    if (ReadConsoleRequest->CaptureBufferSize <= sizeof(ReadConsoleRequest->StaticBuffer))
    {
        /*
         * Adjust the internal pointer, because its old value points to
         * the static buffer in the original ApiMessage structure.
         */
        // ReadConsoleRequest->Buffer = ReadConsoleRequest->StaticBuffer;
    }
    else
    {
        if (!CsrValidateMessageBuffer(ApiMessage,
                                      (PVOID*)&ReadConsoleRequest->Buffer,
                                      ReadConsoleRequest->CaptureBufferSize,
                                      sizeof(BYTE)))
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    if (ReadConsoleRequest->InitialNumBytes > ReadConsoleRequest->NumBytes)
    {
        return STATUS_INVALID_PARAMETER;
    }

    Status = ConSrvGetInputBufferAndHandleEntry(ProcessData, ReadConsoleRequest->InputHandle, &InputBuffer, &HandleEntry, GENERIC_READ, TRUE);
    if (!NT_SUCCESS(Status)) return Status;

    InputInfo.CallingThread = CsrGetClientThread();
    InputInfo.HandleEntry   = HandleEntry;
    InputInfo.InputBuffer   = InputBuffer;

    Status = ReadChars(&InputInfo, ApiMessage, TRUE);

    ConSrvReleaseInputBuffer(InputBuffer, TRUE);

    if (Status == STATUS_PENDING) *ReplyCode = CsrReplyPending;

    return Status;
}

CSR_API(SrvGetConsoleInput)
{
    NTSTATUS Status;
    PCONSOLE_GETINPUT GetInputRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.GetInputRequest;
    PCONSOLE_PROCESS_DATA ProcessData = ConsoleGetPerProcessData(CsrGetClientThread()->Process);
    PVOID HandleEntry;
    PCONSOLE_INPUT_BUFFER InputBuffer;
    GET_INPUT_INFO InputInfo;

    DPRINT("SrvGetConsoleInput\n");

    if (GetInputRequest->Flags & ~(CONSOLE_READ_KEEPEVENT | CONSOLE_READ_CONTINUE))
        return STATUS_INVALID_PARAMETER;

    /*
     * For optimization purposes, Windows (and hence ReactOS, too, for
     * compatibility reasons) uses a static buffer if no more than five
     * input records are read. Otherwise a new buffer is used.
     * The client-side expects that we know this behaviour.
     */
    if (GetInputRequest->NumRecords <= sizeof(GetInputRequest->RecordStaticBuffer)/sizeof(INPUT_RECORD))
    {
        /*
         * Adjust the internal pointer, because its old value points to
         * the static buffer in the original ApiMessage structure.
         */
        // GetInputRequest->RecordBufPtr = GetInputRequest->RecordStaticBuffer;
    }
    else
    {
        if (!CsrValidateMessageBuffer(ApiMessage,
                                      (PVOID*)&GetInputRequest->RecordBufPtr,
                                      GetInputRequest->NumRecords,
                                      sizeof(INPUT_RECORD)))
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    Status = ConSrvGetInputBufferAndHandleEntry(ProcessData, GetInputRequest->InputHandle, &InputBuffer, &HandleEntry, GENERIC_READ, TRUE);
    if (!NT_SUCCESS(Status)) return Status;

    InputInfo.CallingThread = CsrGetClientThread();
    InputInfo.HandleEntry   = HandleEntry;
    InputInfo.InputBuffer   = InputBuffer;

    Status = ReadInputBuffer(&InputInfo, ApiMessage, TRUE);

    ConSrvReleaseInputBuffer(InputBuffer, TRUE);

    if (Status == STATUS_PENDING) *ReplyCode = CsrReplyPending;

    return Status;
}

NTSTATUS NTAPI
ConDrvWriteConsoleInput(IN PCONSOLE Console,
                        IN PCONSOLE_INPUT_BUFFER InputBuffer,
                        IN BOOLEAN Unicode,
                        IN BOOLEAN AppendToEnd,
                        IN PINPUT_RECORD InputRecord,
                        IN ULONG NumEventsToWrite,
                        OUT PULONG NumEventsWritten OPTIONAL);
CSR_API(SrvWriteConsoleInput)
{
    NTSTATUS Status;
    PCONSOLE_WRITEINPUT WriteInputRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.WriteInputRequest;
    PCONSOLE_INPUT_BUFFER InputBuffer;
    ULONG NumEventsWritten;

    PINPUT_RECORD InputRecord;

    DPRINT("SrvWriteConsoleInput\n");

    /*
     * For optimization purposes, Windows (and hence ReactOS, too, for
     * compatibility reasons) uses a static buffer if no more than five
     * input records are written. Otherwise a new buffer is used.
     * The client-side expects that we know this behaviour.
     */
    if (WriteInputRequest->NumRecords <= sizeof(WriteInputRequest->RecordStaticBuffer)/sizeof(INPUT_RECORD))
    {
        /*
         * Adjust the internal pointer, because its old value points to
         * the static buffer in the original ApiMessage structure.
         */
        // WriteInputRequest->RecordBufPtr = WriteInputRequest->RecordStaticBuffer;
        InputRecord = WriteInputRequest->RecordStaticBuffer;
    }
    else
    {
        if (!CsrValidateMessageBuffer(ApiMessage,
                                      (PVOID*)&WriteInputRequest->RecordBufPtr,
                                      WriteInputRequest->NumRecords,
                                      sizeof(INPUT_RECORD)))
        {
            return STATUS_INVALID_PARAMETER;
        }

        InputRecord = WriteInputRequest->RecordBufPtr;
    }

    Status = ConSrvGetInputBuffer(ConsoleGetPerProcessData(CsrGetClientThread()->Process),
                                  WriteInputRequest->InputHandle,
                                  &InputBuffer, GENERIC_WRITE, TRUE);
    if (!NT_SUCCESS(Status))
    {
        WriteInputRequest->NumRecords = 0;
        return Status;
    }

    NumEventsWritten = 0;
    Status = ConDrvWriteConsoleInput(InputBuffer->Header.Console,
                                     InputBuffer,
                                     WriteInputRequest->Unicode,
                                     WriteInputRequest->AppendToEnd,
                                     InputRecord,
                                     WriteInputRequest->NumRecords,
                                     &NumEventsWritten);
    WriteInputRequest->NumRecords = NumEventsWritten;

    ConSrvReleaseInputBuffer(InputBuffer, TRUE);
    return Status;
}

NTSTATUS NTAPI
ConDrvFlushConsoleInputBuffer(IN PCONSOLE Console,
                              IN PCONSOLE_INPUT_BUFFER InputBuffer);
CSR_API(SrvFlushConsoleInputBuffer)
{
    NTSTATUS Status;
    PCONSOLE_FLUSHINPUTBUFFER FlushInputBufferRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.FlushInputBufferRequest;
    PCONSOLE_INPUT_BUFFER InputBuffer;

    DPRINT("SrvFlushConsoleInputBuffer\n");

    Status = ConSrvGetInputBuffer(ConsoleGetPerProcessData(CsrGetClientThread()->Process),
                                  FlushInputBufferRequest->InputHandle,
                                  &InputBuffer, GENERIC_WRITE, TRUE);
    if (!NT_SUCCESS(Status)) return Status;

    Status = ConDrvFlushConsoleInputBuffer(InputBuffer->Header.Console,
                                           InputBuffer);

    ConSrvReleaseInputBuffer(InputBuffer, TRUE);
    return Status;
}

NTSTATUS NTAPI
ConDrvGetConsoleNumberOfInputEvents(IN PCONSOLE Console,
                                    IN PCONSOLE_INPUT_BUFFER InputBuffer,
                                    OUT PULONG NumberOfEvents);
CSR_API(SrvGetConsoleNumberOfInputEvents)
{
    NTSTATUS Status;
    PCONSOLE_GETNUMINPUTEVENTS GetNumInputEventsRequest = &((PCONSOLE_API_MESSAGE)ApiMessage)->Data.GetNumInputEventsRequest;
    PCONSOLE_INPUT_BUFFER InputBuffer;

    DPRINT("SrvGetConsoleNumberOfInputEvents\n");

    Status = ConSrvGetInputBuffer(ConsoleGetPerProcessData(CsrGetClientThread()->Process),
                                  GetNumInputEventsRequest->InputHandle,
                                  &InputBuffer, GENERIC_READ, TRUE);
    if (!NT_SUCCESS(Status)) return Status;

    Status = ConDrvGetConsoleNumberOfInputEvents(InputBuffer->Header.Console,
                                                 InputBuffer,
                                                 &GetNumInputEventsRequest->NumberOfEvents);

    ConSrvReleaseInputBuffer(InputBuffer, TRUE);
    return Status;
}

/* EOF */
