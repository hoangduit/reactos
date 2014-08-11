/*
 * PROJECT:     ReactOS Named Pipe FileSystem
 * LICENSE:     BSD - See COPYING.ARM in the top level directory
 * FILE:        drivers/filesystems/npfs/waitsup.c
 * PURPOSE:     Pipes Waiting Support
 * PROGRAMMERS: ReactOS Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include "npfs.h"

// File ID number for NPFS bugchecking support
#define NPFS_BUGCHECK_FILE_ID   (NPFS_BUGCHECK_WAITSUP)

/* FUNCTIONS ******************************************************************/

VOID
NTAPI
NpCancelWaitQueueIrp(IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp)
{
    KIRQL OldIrql;
    PNP_WAIT_QUEUE_ENTRY WaitEntry;
    PNP_WAIT_QUEUE WaitQueue;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    WaitQueue = (PNP_WAIT_QUEUE)Irp->Tail.Overlay.DriverContext[0];

    KeAcquireSpinLock(&WaitQueue->WaitLock, &OldIrql);

    WaitEntry = (PNP_WAIT_QUEUE_ENTRY)Irp->Tail.Overlay.DriverContext[1];
    if (WaitEntry)
    {
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
        if (!KeCancelTimer(&WaitEntry->Timer))
        {
            WaitEntry->Irp = NULL;
            WaitEntry = NULL;
        }
    }

    KeReleaseSpinLock(&WaitQueue->WaitLock, OldIrql);

    if (WaitEntry)
    {
        ObDereferenceObject(WaitEntry->FileObject);
        ExFreePool(WaitEntry);
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NAMED_PIPE_INCREMENT);
}

VOID
NTAPI
NpTimerDispatch(IN PKDPC Dpc,
                IN PVOID Context,
                IN PVOID Argument1,
                IN PVOID Argument2)
{
    PIRP Irp;
    KIRQL OldIrql;
    PNP_WAIT_QUEUE_ENTRY WaitEntry = Context;

    KeAcquireSpinLock(&WaitEntry->WaitQueue->WaitLock, &OldIrql);

    Irp = WaitEntry->Irp;
    if (Irp)
    {
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

        if (!IoSetCancelRoutine(Irp, NULL))
        {
            Irp->Tail.Overlay.DriverContext[1] = NULL;
            Irp = NULL;
        }
    }

    KeReleaseSpinLock(&WaitEntry->WaitQueue->WaitLock, OldIrql);

    if (Irp)
    {
        Irp->IoStatus.Status = STATUS_IO_TIMEOUT;
        IoCompleteRequest(Irp, IO_NAMED_PIPE_INCREMENT);
    }

    ObDereferenceObject(WaitEntry->FileObject);
    ExFreePool(WaitEntry);
}

VOID
NTAPI
NpInitializeWaitQueue(IN PNP_WAIT_QUEUE WaitQueue)
{
    InitializeListHead(&WaitQueue->WaitList);
    KeInitializeSpinLock(&WaitQueue->WaitLock);
}

NTSTATUS
NTAPI
NpCancelWaiter(IN PNP_WAIT_QUEUE WaitQueue,
               IN PUNICODE_STRING PipeName,
               IN NTSTATUS Status,
               IN PLIST_ENTRY List)
{
    UNICODE_STRING DestinationString;
    KIRQL OldIrql;
    PWCHAR Buffer;
    PLIST_ENTRY NextEntry;
    PNP_WAIT_QUEUE_ENTRY WaitEntry, Linkage;
    PIRP WaitIrp;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitBuffer;
    ULONG i, NameLength;

    Linkage = NULL;

    Buffer = ExAllocatePoolWithTag(NonPagedPool,
                                   PipeName->Length,
                                   NPFS_WAIT_BLOCK_TAG);
    if (!Buffer) return STATUS_INSUFFICIENT_RESOURCES;

    RtlInitEmptyUnicodeString(&DestinationString, Buffer, PipeName->Length);
    RtlUpcaseUnicodeString(&DestinationString, PipeName, FALSE);

    KeAcquireSpinLock(&WaitQueue->WaitLock, &OldIrql);

    for (NextEntry = WaitQueue->WaitList.Flink;
         NextEntry != &WaitQueue->WaitList;
         NextEntry = NextEntry->Flink)
    {
        WaitIrp = CONTAINING_RECORD(NextEntry, IRP, Tail.Overlay.ListEntry);
        WaitEntry = WaitIrp->Tail.Overlay.DriverContext[1];

        if (WaitEntry->AliasName.Length)
        {
            ASSERT(FALSE);
            if (DestinationString.Length == WaitEntry->AliasName.Length)
            {
                if (RtlCompareMemory(WaitEntry->AliasName.Buffer,
                                     DestinationString.Buffer,
                                     DestinationString.Length) ==
                    DestinationString.Length)
                {
CancelWait:
                    RemoveEntryList(&WaitIrp->Tail.Overlay.ListEntry);
                    if (KeCancelTimer(&WaitEntry->Timer))
                    {
                        WaitEntry->WaitQueue = (PNP_WAIT_QUEUE)Linkage;
                        Linkage = WaitEntry;
                    }
                    else
                    {
                        WaitEntry->Irp = NULL;
                        WaitIrp->Tail.Overlay.DriverContext[1] = NULL;
                    }

                    if (IoSetCancelRoutine(WaitIrp, NULL))
                    {
                        WaitIrp->IoStatus.Information = 0;
                        WaitIrp->IoStatus.Status = Status;
                        InsertTailList(List, &WaitIrp->Tail.Overlay.ListEntry);
                    }
                    else
                    {
                        WaitIrp->Tail.Overlay.DriverContext[1] = NULL;
                    }
                }
            }
        }
        else
        {
            WaitBuffer = WaitIrp->AssociatedIrp.SystemBuffer;

            if (WaitBuffer->NameLength + sizeof(WCHAR) == DestinationString.Length)
            {
                NameLength = WaitBuffer->NameLength / sizeof(WCHAR);
                for (i = 0; i < NameLength; i++)
                {
                    if (WaitBuffer->Name[i] != DestinationString.Buffer[i + 1]) break;
                }

                if (i >= NameLength) goto CancelWait;
            }
        }
    }

    KeReleaseSpinLock(&WaitQueue->WaitLock, OldIrql);

    ExFreePool(DestinationString.Buffer);

    while (Linkage)
    {
        WaitEntry = Linkage;
        Linkage = (PNP_WAIT_QUEUE_ENTRY)Linkage->WaitQueue;
        ObDereferenceObject(WaitEntry->FileObject);
        ExFreePool(WaitEntry);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
NpAddWaiter(IN PNP_WAIT_QUEUE WaitQueue,
            IN LARGE_INTEGER WaitTime,
            IN PIRP Irp,
            IN PUNICODE_STRING AliasName)
{
    PIO_STACK_LOCATION IoStack;
    KIRQL OldIrql;
    NTSTATUS Status;
    PNP_WAIT_QUEUE_ENTRY WaitEntry;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitBuffer;
    LARGE_INTEGER DueTime;
    ULONG i;

    IoStack = IoGetCurrentIrpStackLocation(Irp);

    WaitEntry = ExAllocatePoolWithQuotaTag(NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                           sizeof(*WaitEntry),
                                           NPFS_WRITE_BLOCK_TAG);
    if (!WaitEntry)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeDpc(&WaitEntry->Dpc, NpTimerDispatch, WaitEntry);
    KeInitializeTimer(&WaitEntry->Timer);

    if (AliasName)
    {
        WaitEntry->AliasName = *AliasName;
    }
    else
    {
        WaitEntry->AliasName.Length = 0;
        WaitEntry->AliasName.Buffer = NULL;
    }

    WaitEntry->WaitQueue = WaitQueue;
    WaitEntry->Irp = Irp;

    WaitBuffer = Irp->AssociatedIrp.SystemBuffer;
    if (WaitBuffer->TimeoutSpecified)
    {
        DueTime = WaitBuffer->Timeout;
    }
    else
    {
        DueTime = WaitTime;
    }

    for (i = 0; i < WaitBuffer->NameLength / sizeof(WCHAR); i++)
    {
        WaitBuffer->Name[i] = RtlUpcaseUnicodeChar(WaitBuffer->Name[i]);
    }

    Irp->Tail.Overlay.DriverContext[0] = WaitQueue;
    Irp->Tail.Overlay.DriverContext[1] = WaitEntry;

    KeAcquireSpinLock(&WaitQueue->WaitLock, &OldIrql);

    IoSetCancelRoutine(Irp, NpCancelWaitQueueIrp);

    if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL))
    {
        Status = STATUS_CANCELLED;
    }
    else
    {
        InsertTailList(&WaitQueue->WaitList, &Irp->Tail.Overlay.ListEntry);

        IoMarkIrpPending(Irp);
        Status = STATUS_PENDING;

        WaitEntry->FileObject = IoStack->FileObject;
        ObReferenceObject(WaitEntry->FileObject);

        KeSetTimer(&WaitEntry->Timer, DueTime, &WaitEntry->Dpc);
        WaitEntry = NULL;
    }

    KeReleaseSpinLock(&WaitQueue->WaitLock, OldIrql);
    if (WaitEntry) ExFreePool(WaitEntry);

    return Status;
}

/* EOF */
