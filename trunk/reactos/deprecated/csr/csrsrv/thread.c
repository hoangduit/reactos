/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS CSR Sub System
 * FILE:            subsys/csr/csrsrv/thread.c
 * PURPOSE:         CSR Server DLL Thread Implementation
 * PROGRAMMERS:     Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/

#include "srv.h"

#define NDEBUG
#include <debug.h>

/* DATA **********************************************************************/

LIST_ENTRY CsrThreadHashTable[256];
SECURITY_QUALITY_OF_SERVICE CsrSecurityQos =
{
    sizeof(SECURITY_QUALITY_OF_SERVICE),
    SecurityImpersonation,
    SECURITY_STATIC_TRACKING,
    FALSE
};

/* PRIVATE FUNCTIONS *********************************************************/

/*++
 * @name CsrAllocateThread
 *
 * The CsrAllocateThread routine allocates a new CSR Thread object.
 *
 * @param CsrProcess
 *        Pointer to the CSR Process which will contain this CSR Thread.
 *
 * @return Pointer to the newly allocated CSR Thread.
 *
 * @remarks None.
 *
 *--*/
PCSR_THREAD
NTAPI
CsrAllocateThread(IN PCSR_PROCESS CsrProcess)
{
    PCSR_THREAD CsrThread;

    /* Allocate the structure */
    CsrThread = RtlAllocateHeap(CsrHeap, HEAP_ZERO_MEMORY, sizeof(CSR_THREAD));
    if (!CsrThread) return NULL;

    /* Reference the Thread and Process */
    CsrThread->ReferenceCount++;
    CsrProcess->ReferenceCount++;

    /* Set the Parent Process */
    CsrThread->Process = CsrProcess;

    /* Return Thread */
    return CsrThread;
}

/*++
 * @name CsrLocateThreadByClientId
 *
 * The CsrLocateThreadByClientId routine locates the CSR Thread and,
 * optionally, its parent CSR Process, corresponding to a Client ID.
 *
 * @param Process
 *        Optional pointer to a CSR Process pointer which will contain
 *        the CSR Thread's parent.
 *
 * @param ClientId
 *        Pointer to a Client ID structure containing the Unique Thread ID
 *        to look up.
 *
 * @return Pointer to the CSR Thread corresponding to this CID, or NULL if
 *         none was found.
 *
 * @remarks None.
 *
 *--*/
PCSR_THREAD
NTAPI
CsrLocateThreadByClientId(OUT PCSR_PROCESS *Process OPTIONAL,
                          IN PCLIENT_ID ClientId)
{
    ULONG i;
    PLIST_ENTRY NextEntry;
    PCSR_THREAD FoundThread;
    ASSERT(ProcessStructureListLocked());

    /* Hash the Thread */
    i = CsrHashThread(ClientId->UniqueThread);

    /* Set the list pointers */
    NextEntry = CsrThreadHashTable[i].Flink;

    /* Star the loop */
    while (NextEntry != &CsrThreadHashTable[i])
    {
        /* Get the thread */
        FoundThread = CONTAINING_RECORD(NextEntry, CSR_THREAD, HashLinks);

        /* Compare the CID */
        if (*(PULONGLONG)&FoundThread->ClientId == *(PULONGLONG)ClientId)
        {
            /* Match found, return the process */
            *Process = FoundThread->Process;

            /* Return thread too */
            return FoundThread;
        }

        /* Next */
        NextEntry = NextEntry->Flink;
    }

    /* Nothing found */
    return NULL;
}

/*++
 * @name CsrLocateThreadInProcess
 *
 * The CsrLocateThreadInProcess routine locates the CSR Thread
 * corresponding to a Client ID inside a specific CSR Process.
 *
 * @param Process
 *        Optional pointer to the CSR Process which contains the CSR Thread
 *        that will be looked up.
 *
 * @param ClientId
 *        Pointer to a Client ID structure containing the Unique Thread ID
 *        to look up.
 *
 * @return Pointer to the CSR Thread corresponding to this CID, or NULL if
 *         none was found.
 *
 * @remarks If the CsrProcess argument is NULL, the lookup will be done inside
 *          CsrRootProcess.
 *
 *--*/
PCSR_THREAD
NTAPI
CsrLocateThreadInProcess(IN PCSR_PROCESS CsrProcess OPTIONAL,
                         IN PCLIENT_ID Cid)
{
    PLIST_ENTRY NextEntry;
    PCSR_THREAD FoundThread = NULL;

    /* Use the Root Process if none was specified */
    if (!CsrProcess) CsrProcess = CsrRootProcess;

    /* Save the List pointers */
    NextEntry = CsrProcess->ThreadList.Flink;

    /* Start the Loop */
    while (NextEntry != &CsrProcess->ThreadList)
    {
        /* Get Thread Entry */
        FoundThread = CONTAINING_RECORD(NextEntry, CSR_THREAD, Link);

        /* Check for TID Match */
        if (FoundThread->ClientId.UniqueThread == Cid->UniqueThread) break;

        /* Next entry */
        NextEntry = NextEntry->Flink;
    }

    /* Return what we found */
    return FoundThread;
}

/*++
 * @name CsrInsertThread
 *
 * The CsrInsertThread routine inserts a CSR Thread into its parent's
 * Thread List and into the Thread Hash Table.
 *
 * @param Process
 *        Pointer to the CSR Process containing this CSR Thread.
 *
 * @param Thread
 *        Pointer to the CSR Thread to be inserted.
 *
 * @return None.
 *
 * @remarks None.
 *
 *--*/
VOID
NTAPI
CsrInsertThread(IN PCSR_PROCESS Process,
                IN PCSR_THREAD Thread)
{
    ULONG i;
    ASSERT(ProcessStructureListLocked());

    /* Insert it into the Regular List */
    InsertTailList(&Process->ThreadList, &Thread->Link);

    /* Increase Thread Count */
    Process->ThreadCount++;

    /* Hash the Thread */
    i = CsrHashThread(Thread->ClientId.UniqueThread);

    /* Insert it there too */
    InsertHeadList(&CsrThreadHashTable[i], &Thread->HashLinks);
}

/*++
 * @name CsrDeallocateThread
 *
 * The CsrDeallocateThread frees the memory associated with a CSR Thread.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread to be freed.
 *
 * @return None.
 *
 * @remarks Do not call this routine. It is reserved for the internal
 *          thread management routines when a CSR Thread has been cleanly
 *          dereferenced and killed.
 *
 *--*/
VOID
NTAPI
CsrDeallocateThread(IN PCSR_THREAD CsrThread)
{
    /* Free the process object from the heap */
    ASSERT(CsrThread->WaitBlock == NULL);
    RtlFreeHeap(CsrHeap, 0, CsrThread);
}

/*++
 * @name CsrLockedReferenceThread
 *
 * The CsrLockedReferenceThread refences a CSR Thread while the
 * Process Lock is already being held.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread to be referenced.
 *
 * @return None.
 *
 * @remarks This routine will return with the Process Lock held.
 *
 *--*/
VOID
NTAPI
CsrLockedReferenceThread(IN PCSR_THREAD CsrThread)
{
    /* Increment the reference count */
    ++CsrThread->ReferenceCount;
}

/*++
 * @name CsrLockedDereferenceThread
 *
 * The CsrLockedDereferenceThread derefences a CSR Thread while the
 * Process Lock is already being held.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread to be dereferenced.
 *
 * @return None.
 *
 * @remarks This routine will return with the Process Lock held.
 *
 *--*/
VOID
NTAPI
CsrLockedDereferenceThread(IN PCSR_THREAD CsrThread)
{
    LONG LockCount;

    /* Decrease reference count */
    LockCount = --CsrThread->ReferenceCount;
    ASSERT(LockCount >= 0);
    if (!LockCount)
    {
        /* Call the generic cleanup code */
        CsrThreadRefcountZero(CsrThread);
        CsrAcquireProcessLock();
    }
}

/*++
 * @name CsrRemoveThread
 *
 * The CsrRemoveThread function undoes a CsrInsertThread operation and
 * removes the CSR Thread from the the Hash Table and Thread List.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread to remove.
 *
 * @return None.
 *
 * @remarks If this CSR Thread is the last one inside a CSR Process, the
 *          parent will be dereferenced and the CsrProcessLastThreadTerminated
 *          flag will be set.
 *
 *          After executing this routine, the CSR Thread will have the
 *          CsrThreadInTermination flag set.
 *
 *--*/
VOID
NTAPI
CsrRemoveThread(IN PCSR_THREAD CsrThread)
{
    ASSERT(ProcessStructureListLocked());

    /* Remove it from the List */
    RemoveEntryList(&CsrThread->Link);

    /* Decreate the thread count of the process */
    --CsrThread->Process->ThreadCount;

    /* Remove it from the Hash List as well */
    if (CsrThread->HashLinks.Flink) RemoveEntryList(&CsrThread->HashLinks);

    /* Check if this is the last Thread */
    if (!CsrThread->Process->ThreadCount)
    {
        /* Check if it's not already been marked for deletion */
        if (!(CsrThread->Process->Flags & CsrProcessLastThreadTerminated))
        {
            /* Let everyone know this process is about to lose the thread */
            CsrThread->Process->Flags |= CsrProcessLastThreadTerminated;

            /* Reference the Process */
            CsrLockedDereferenceProcess(CsrThread->Process);
        }
    }

    /* Mark the thread for deletion */
    CsrThread->Flags |= CsrThreadInTermination;
}

/*++
 * @name CsrThreadRefcountZero
 *
 * The CsrThreadRefcountZero routine is executed when a CSR Thread has lost
 * all its active references. It removes and de-allocates the CSR Thread.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread that is to be deleted.
 *
 * @return None.
 *
 * @remarks Do not call this routine. It is reserved for the internal
 *          thread management routines when a CSR Thread has lost all
 *          its references.
 *
 *          This routine is called with the Process Lock held.
 *
 *--*/
VOID
NTAPI
CsrThreadRefcountZero(IN PCSR_THREAD CsrThread)
{
    PCSR_PROCESS CsrProcess = CsrThread->Process;
    NTSTATUS Status;

    /* Remove this thread */
    CsrRemoveThread(CsrThread);

    /* Release the Process Lock */
    CsrReleaseProcessLock();

    /* Close the NT Thread Handle */
    UnProtectHandle(CsrThread->ThreadHandle);
    Status = NtClose(CsrThread->ThreadHandle);
    ASSERT(NT_SUCCESS(Status));

    /* De-allocate the CSR Thread Object */
    CsrDeallocateThread(CsrThread);

    /* Remove a reference from the process */
    CsrDereferenceProcess(CsrProcess);
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*++
 * @name CsrAddStaticServerThread
 * @implemented NT4
 *
 * The CsrAddStaticServerThread routine adds a new CSR Thread to the
 * CSR Server Process (CsrRootProcess).
 *
 * @param hThread
 *        Handle to an existing NT Thread to which to associate this
 *        CSR Thread.
 *
 * @param ClientId
 *        Pointer to the Client ID structure of the NT Thread to associate
 *        with this CSR Thread.
 *
 * @param ThreadFlags
 *        Initial CSR Thread Flags to associate to this CSR Thread. Usually
 *        CsrThreadIsServerThread.
 *
 * @return Pointer to the newly allocated CSR Thread.
 *
 * @remarks None.
 *
 *--*/
PCSR_THREAD
NTAPI
CsrAddStaticServerThread(IN HANDLE hThread,
                         IN PCLIENT_ID ClientId,
                         IN ULONG ThreadFlags)
{
    PCSR_THREAD CsrThread;

    /* Get the Lock */
    CsrAcquireProcessLock();

    /* Allocate the Server Thread */
    CsrThread = CsrAllocateThread(CsrRootProcess);
    if (CsrThread)
    {
        /* Setup the Object */
        CsrThread->ThreadHandle = hThread;
        ProtectHandle(hThread);
        CsrThread->ClientId = *ClientId;
        CsrThread->Flags = ThreadFlags;

        /* Insert it into the Thread List */
        InsertTailList(&CsrRootProcess->ThreadList, &CsrThread->Link);

        /* Increment the thread count */
        CsrRootProcess->ThreadCount++;
    }
    else
    {
        DPRINT1("CsrAddStaticServerThread: alloc failed for thread 0x%x\n", hThread);
    }

    /* Release the Process Lock and return */
    CsrReleaseProcessLock();
    return CsrThread;
}

/*++
 * @name CsrCreateRemoteThread
 * @implemented NT4
 *
 * The CsrCreateRemoteThread routine creates a CSR Thread object for
 * an NT Thread which is not part of the current NT Process.
 *
 * @param hThread
 *        Handle to an existing NT Thread to which to associate this
 *        CSR Thread.
 *
 * @param ClientId
 *        Pointer to the Client ID structure of the NT Thread to associate
 *        with this CSR Thread.
 *
 * @return STATUS_SUCCESS in case of success, STATUS_UNSUCCESSFUL
 *         othwerwise.
 *
 * @remarks None.
 *
 *--*/
NTSTATUS
NTAPI
CsrCreateRemoteThread(IN HANDLE hThread,
                      IN PCLIENT_ID ClientId)
{
    NTSTATUS Status;
    HANDLE ThreadHandle;
    PCSR_THREAD CsrThread;
    PCSR_PROCESS CsrProcess;
    KERNEL_USER_TIMES KernelTimes;

    DPRINT("CSRSRV: %s called\n", __FUNCTION__);

    /* Get the Thread Create Time */
    Status = NtQueryInformationThread(hThread,
                                      ThreadTimes,
                                      &KernelTimes,
                                      sizeof(KernelTimes),
                                      NULL);
    if (!NT_SUCCESS(Status)) return Status;

    /* Lock the Owner Process */
    Status = CsrLockProcessByClientId(&ClientId->UniqueProcess, &CsrProcess);

    /* Make sure the thread didn't terminate */
    if (KernelTimes.ExitTime.QuadPart)
    {
        /* Unlock the process and return */
        CsrUnlockProcess(CsrProcess);
        return STATUS_THREAD_IS_TERMINATING;
    }

    /* Allocate a CSR Thread Structure */
    CsrThread = CsrAllocateThread(CsrProcess);
    if (!CsrThread)
    {
        DPRINT1("CSRSRV:%s: out of memory!\n", __FUNCTION__);
        CsrUnlockProcess(CsrProcess);
        return STATUS_NO_MEMORY;
    }

    /* Duplicate the Thread Handle */
    Status = NtDuplicateObject(NtCurrentProcess(),
                               hThread,
                               NtCurrentProcess(),
                               &ThreadHandle,
                               0,
                               0,
                               DUPLICATE_SAME_ACCESS);
    /* Allow failure */
    if (!NT_SUCCESS(Status)) ThreadHandle = hThread;

    /* Save the data we have */
    CsrThread->CreateTime = KernelTimes.CreateTime;
    CsrThread->ClientId = *ClientId;
    CsrThread->ThreadHandle = ThreadHandle;
    ProtectHandle(ThreadHandle);
    CsrThread->Flags = 0;

    /* Insert the Thread into the Process */
    CsrInsertThread(CsrProcess, CsrThread);

    /* Release the lock and return */
    CsrUnlockProcess(CsrProcess);
    return STATUS_SUCCESS;
}

/*++
 * @name CsrCreateThread
 * @implemented NT4
 *
 * The CsrCreateThread routine creates a CSR Thread object for an NT Thread.
 *
 * @param CsrProcess
 *        Pointer to the CSR Process which will contain the CSR Thread.
 *
 * @param hThread
 *        Handle to an existing NT Thread to which to associate this
 *        CSR Thread.
 *
 * @param ClientId
 *        Pointer to the Client ID structure of the NT Thread to associate
 *        with this CSR Thread.
 *
 * @return STATUS_SUCCESS in case of success, STATUS_UNSUCCESSFUL
 *         othwerwise.
 *
 * @remarks None.
 *
 *--*/
NTSTATUS
NTAPI
CsrCreateThread(IN PCSR_PROCESS CsrProcess,
                IN HANDLE hThread,
                IN PCLIENT_ID ClientId,
                IN BOOLEAN HaveClient)
{
    NTSTATUS Status;
    PCSR_THREAD CsrThread, CurrentThread;
    PCSR_PROCESS CurrentProcess;
    CLIENT_ID CurrentCid;
    KERNEL_USER_TIMES KernelTimes;
    DPRINT("CSRSRV: %s called\n", __FUNCTION__);

    if (HaveClient)
    {
        /* Get the current thread and CID */
        CurrentThread = NtCurrentTeb()->CsrClientThread;
        CurrentCid = CurrentThread->ClientId;

        /* Acquire the Process Lock */
        CsrAcquireProcessLock();

        /* Get the current Process and make sure the Thread is valid with this CID */
        CurrentThread = CsrLocateThreadByClientId(&CurrentProcess, &CurrentCid);
        if (!CurrentThread)
        {
            DPRINT1("CSRSRV:%s: invalid thread!\n", __FUNCTION__);
            CsrReleaseProcessLock();
            return STATUS_THREAD_IS_TERMINATING;
        }
    }
    else
    {
        /* Acquire the Process Lock */
        CsrAcquireProcessLock();
    }

    /* Get the Thread Create Time */
    Status = NtQueryInformationThread(hThread,
                                      ThreadTimes,
                                      (PVOID)&KernelTimes,
                                      sizeof(KernelTimes),
                                      NULL);
    if (!NT_SUCCESS(Status))
    {
        CsrReleaseProcessLock();
        return Status;
    }

    /* Allocate a CSR Thread Structure */
    CsrThread = CsrAllocateThread(CsrProcess);
    if (!CsrThread)
    {
        DPRINT1("CSRSRV:%s: out of memory!\n", __FUNCTION__);
        CsrReleaseProcessLock();
        return STATUS_NO_MEMORY;
    }

    /* Save the data we have */
    CsrThread->CreateTime = KernelTimes.CreateTime;
    CsrThread->ClientId = *ClientId;
    CsrThread->ThreadHandle = hThread;
    ProtectHandle(hThread);
    CsrThread->Flags = 0;

    /* Insert the Thread into the Process */
    CsrInsertThread(CsrProcess, CsrThread);

    /* Release the lock and return */
    CsrReleaseProcessLock();
    return STATUS_SUCCESS;
}

/*++
 * @name CsrDereferenceThread
 * @implemented NT4
 *
 * The CsrDereferenceThread routine removes a reference from a CSR Thread.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread to dereference.
 *
 * @return None.
 *
 * @remarks If the reference count has reached zero (ie: the CSR Thread has
 *          no more active references), it will be deleted.
 *
 *--*/
VOID
NTAPI
CsrDereferenceThread(IN PCSR_THREAD CsrThread)
{
    /* Acquire process lock */
    CsrAcquireProcessLock();

    /* Decrease reference count */
    ASSERT(CsrThread->ReferenceCount > 0);
    if (!(--CsrThread->ReferenceCount))
    {
        /* Call the generic cleanup code */
        CsrThreadRefcountZero(CsrThread);
    }
    else
    {
        /* Just release the lock */
        CsrReleaseProcessLock();
    }
}

/*++
 * @name CsrExecServerThread
 * @implemented NT4
 *
 * The CsrExecServerThread routine creates an NT Thread and then
 * initializes a CSR Thread for it.
 *
 * @param ThreadHandler
 *        Pointer to the thread's startup routine.
 *
 * @param Flags
 *        Initial CSR Thread Flags to set to the CSR Thread.
 *
 * @return STATUS_SUCCESS in case of success, STATUS_UNSUCCESSFUL
 *         othwerwise.
 *
 * @remarks This routine is similar to CsrAddStaticServerThread, but it
 *          also creates an NT Thread instead of expecting one to already
 *          exist.
 *
 *--*/
NTSTATUS
NTAPI
CsrExecServerThread(IN PVOID ThreadHandler,
                    IN ULONG Flags)
{
    PCSR_THREAD CsrThread;
    HANDLE hThread;
    CLIENT_ID ClientId;
    NTSTATUS Status;

    /* Acquire process lock */
    CsrAcquireProcessLock();

    /* Allocate a CSR Thread in the Root Process */
    ASSERT(CsrRootProcess != NULL);
    CsrThread = CsrAllocateThread(CsrRootProcess);
    if (!CsrThread)
    {
        /* Fail */
        CsrReleaseProcessLock();
        return STATUS_NO_MEMORY;
    }

    /* Create the Thread */
    Status = RtlCreateUserThread(NtCurrentProcess(),
                                 NULL,
                                 FALSE,
                                 0,
                                 0,
                                 0,
                                 ThreadHandler,
                                 NULL,
                                 &hThread,
                                 &ClientId);
    if (!NT_SUCCESS(Status))
    {
        /* Fail */
        CsrDeallocateThread(CsrThread);
        CsrReleaseProcessLock();
        return Status;
    }

    /* Setup the Thread Object */
    CsrThread->ThreadHandle = hThread;
    ProtectHandle(hThread);
    CsrThread->ClientId = ClientId;
    CsrThread->Flags = Flags;

    /* Insert it into the Thread List */
    InsertHeadList(&CsrRootProcess->ThreadList, &CsrThread->Link);

    /* Increase the thread count */
    CsrRootProcess->ThreadCount++;

    /* Return */
    CsrReleaseProcessLock();
    return Status;
}

/*++
 * @name CsrDestroyThread
 * @implemented NT4
 *
 * The CsrDestroyThread routine destroys the CSR Thread corresponding to
 * a given Thread ID.
 *
 * @param Cid
 *        Pointer to the Client ID Structure corresponding to the CSR
 *        Thread which is about to be destroyed.
 *
 * @return STATUS_SUCCESS in case of success, STATUS_THREAD_IS_TERMINATING
 *         if the CSR Thread is already terminating.
 *
 * @remarks None.
 *
 *--*/
NTSTATUS
NTAPI
CsrDestroyThread(IN PCLIENT_ID Cid)
{
    CLIENT_ID ClientId = *Cid;
    PCSR_THREAD CsrThread;
    PCSR_PROCESS CsrProcess;

    /* Acquire lock */
    CsrAcquireProcessLock();

    /* Find the thread */
    CsrThread = CsrLocateThreadByClientId(&CsrProcess,
                                          &ClientId);

    /* Make sure we got one back, and that it's not already gone */
    if (!CsrThread || CsrThread->Flags & CsrThreadTerminated)
    {
        /* Release the lock and return failure */
        CsrReleaseProcessLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    /* Set the terminated flag */
    CsrThread->Flags |= CsrThreadTerminated;

    /* Acquire the Wait Lock */
    CsrAcquireWaitLock();

    /* Do we have an active wait block? */
    if (CsrThread->WaitBlock)
    {
        /* Notify waiters of termination */
        CsrNotifyWaitBlock(CsrThread->WaitBlock,
                           NULL,
                           NULL,
                           NULL,
                           CsrProcessTerminating,
                           TRUE);
    }

    /* Release the Wait Lock */
    CsrReleaseWaitLock();

    /* Dereference the thread */
    CsrLockedDereferenceThread(CsrThread);

    /* Release the Process Lock and return success */
    CsrReleaseProcessLock();
    return STATUS_SUCCESS;
}

/*++
 * @name CsrImpersonateClient
 * @implemented NT4
 *
 * The CsrImpersonateClient will impersonate the given CSR Thread.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread to impersonate.
 *
 * @return TRUE if impersionation suceeded, false otherwise.
 *
 * @remarks Impersonation can be recursive.
 *
 *--*/
BOOLEAN
NTAPI
CsrImpersonateClient(IN PCSR_THREAD CsrThread)
{
    NTSTATUS Status;
    PCSR_THREAD CurrentThread = NtCurrentTeb()->CsrClientThread;

    /* Use the current thread if none given */
    if (!CsrThread) CsrThread = CurrentThread;

    /* Still no thread, something is wrong */
    if (!CsrThread) return FALSE;

    /* Make the call */
    Status = NtImpersonateThread(NtCurrentThread(),
                                 CsrThread->ThreadHandle,
                                 &CsrSecurityQos);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("CSRSS: Can't impersonate client thread - Status = %lx\n", Status);
        if (Status != STATUS_BAD_IMPERSONATION_LEVEL) DbgBreakPoint();
        return FALSE;
    }

    /* Increase the impersonation count for the current thread and return */
    if (CurrentThread) ++CurrentThread->ImpersonationCount;
    return TRUE;
}

/*++
 * @name CsrRevertToSelf
 * @implemented NT4
 *
 * The CsrRevertToSelf routine will attempt to remove an active impersonation.
 *
 * @param None.
 *
 * @return TRUE if the reversion was succesful, false otherwise.
 *
 * @remarks Impersonation can be recursive; as such, the impersonation token
 *          will only be deleted once the CSR Thread's impersonaton count
 *          has reached zero.
 *
 *--*/
BOOLEAN
NTAPI
CsrRevertToSelf(VOID)
{
    NTSTATUS Status;
    PCSR_THREAD CurrentThread = NtCurrentTeb()->CsrClientThread;
    HANDLE ImpersonationToken = NULL;

    /* Check if we have a Current Thread */
    if (CurrentThread)
    {
        /* Make sure impersonation is on */
        if (!CurrentThread->ImpersonationCount)
        {
            DPRINT1("CSRSS: CsrRevertToSelf called while not impersonating\n");
            DbgBreakPoint();
            return FALSE;
        }
        else if (--CurrentThread->ImpersonationCount > 0)
        {
            /* Success; impersonation count decreased but still not zero */
            return TRUE;
        }
    }

    /* Impersonation has been totally removed, revert to ourselves */
    Status = NtSetInformationThread(NtCurrentThread(),
                                    ThreadImpersonationToken,
                                    &ImpersonationToken,
                                    sizeof(HANDLE));

    /* Return TRUE or FALSE */
    ASSERT(NT_SUCCESS(Status));
    return NT_SUCCESS(Status);
}

/*++
 * @name CsrLockThreadByClientId
 * @implemented NT4
 *
 * The CsrLockThreadByClientId routine locks the CSR Thread corresponding
 * to the given Thread ID and optionally returns it.
 *
 * @param Tid
 *        Thread ID corresponding to the CSR Thread which will be locked.
 *
 * @param CsrThread
 *        Optional pointer to a CSR Thread pointer which will hold the
 *        CSR Thread corresponding to the given Thread ID.
 *
 * @return STATUS_SUCCESS in case of success, STATUS_UNSUCCESSFUL
 *         othwerwise.
 *
 * @remarks Locking a CSR Thread is defined as acquiring an extra
 *          reference to it and returning with the Process Lock held.
 *
 *--*/
NTSTATUS
NTAPI
CsrLockThreadByClientId(IN HANDLE Tid,
                        OUT PCSR_THREAD *CsrThread)
{
    PLIST_ENTRY NextEntry;
    PCSR_THREAD CurrentThread = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG i;

    /* Acquire the lock */
    CsrAcquireProcessLock();

    /* Assume failure */
    ASSERT(CsrThread != NULL);
    *CsrThread = NULL;

    /* Convert to Hash */
    i = CsrHashThread(Tid);

    /* Setup the List Pointers */
    NextEntry = CsrThreadHashTable[i].Flink;

    /* Start Loop */
    while (NextEntry != &CsrThreadHashTable[i])
    {
        /* Get the Process */
        CurrentThread = CONTAINING_RECORD(NextEntry, CSR_THREAD, HashLinks);

        /* Check for PID Match */
        if ((CurrentThread->ClientId.UniqueThread == Tid) &&
            !(CurrentThread->Flags & CsrThreadTerminated))
        {
            /* Get out of here */
            break;
        }

        /* Next entry */
        NextEntry = NextEntry->Flink;
    }

    /* Nothing found if we got back to the list */
    if (NextEntry == &CsrThreadHashTable[i]) CurrentThread = NULL;

    /* Did the loop find something? */
    if (CurrentThread)
    {
        /* Reference the found thread */
        Status = STATUS_SUCCESS;
        CurrentThread->ReferenceCount++;
        *CsrThread = CurrentThread;
    }
    else
    {
        /* Nothing found, release the lock */
        Status = STATUS_UNSUCCESSFUL;
        CsrReleaseProcessLock();
    }

    /* Return the status */
    return Status;
}

/*++
 * @name CsrReferenceThread
 * @implemented NT4
 *
 * The CsrReferenceThread routine increases the active reference count of
 * a CSR Thread.
 *
 * @param CsrThread
 *        Pointer to the CSR Thread whose reference count will be increased.
 *
 * @return None.
 *
 * @remarks Do not use this routine if the Process Lock is already held.
 *
 *--*/
VOID
NTAPI
CsrReferenceThread(PCSR_THREAD CsrThread)
{
    /* Acquire process lock */
    CsrAcquireProcessLock();

    /* Sanity checks */
    ASSERT(CsrThread->Flags & CsrThreadTerminated); // CSR_THREAD_DESTROYED in ASSERT
    ASSERT(CsrThread->ReferenceCount != 0);

    /* Increment reference count */
    CsrThread->ReferenceCount++;

    /* Release the lock */
    CsrReleaseProcessLock();
}

/*++
 * @name CsrUnlockThread
 * @implemented NT4
 *
 * The CsrUnlockThread undoes a previous CsrLockThreadByClientId operation.
 *
 * @param CsrThread
 *        Pointer to a previously locked CSR Thread.
 *
 * @return STATUS_SUCCESS.
 *
 * @remarks This routine must be called with the Process Lock held.
 *
 *--*/
NTSTATUS
NTAPI
CsrUnlockThread(PCSR_THREAD CsrThread)
{
    /* Dereference the Thread */
    ASSERT(ProcessStructureListLocked());
    CsrLockedDereferenceThread(CsrThread);

    /* Release the lock and return */
    CsrReleaseProcessLock();
    return STATUS_SUCCESS;
}

/* EOF */
