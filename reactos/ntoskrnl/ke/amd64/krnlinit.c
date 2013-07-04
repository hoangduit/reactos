/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/ke/krnlinit.c
 * PURPOSE:         Portable part of kernel initialization
 * PROGRAMMERS:     Timo Kreuzer (timo.kreuzer@reactos.org)
 *                  Alex Ionescu (alex.ionescu@reactos.org)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

extern ULONG_PTR MainSSDT[];
extern UCHAR MainSSPT[];

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
INIT_FUNCTION
KiInitializeKernel(IN PKPROCESS InitProcess,
                   IN PKTHREAD InitThread,
                   IN PVOID IdleStack,
                   IN PKPRCB Prcb,
                   IN PLOADER_PARAMETER_BLOCK LoaderBlock);


VOID
NTAPI
KiInitializeHandBuiltThread(
    IN PKTHREAD Thread,
    IN PKPROCESS Process,
    IN PVOID Stack)
{
    PKPRCB Prcb = KeGetCurrentPrcb();

    /* Setup the Thread */
    KeInitializeThread(Process, Thread, NULL, NULL, NULL, NULL, NULL, Stack);

    Thread->NextProcessor = Prcb->Number;
    Thread->Priority = HIGH_PRIORITY;
    Thread->State = Running;
    Thread->Affinity = (ULONG_PTR)1 << Prcb->Number;
    Thread->WaitIrql = DISPATCH_LEVEL;
    Process->ActiveProcessors |= (ULONG_PTR)1 << Prcb->Number;

}

VOID
NTAPI
INIT_FUNCTION
KiSystemStartupBootStack(VOID)
{
    PLOADER_PARAMETER_BLOCK LoaderBlock = KeLoaderBlock; // hack
    PKPRCB Prcb = KeGetCurrentPrcb();
    PKTHREAD Thread = (PKTHREAD)KeLoaderBlock->Thread;
    PKPROCESS Process = Thread->ApcState.Process;
    PVOID KernelStack = (PVOID)KeLoaderBlock->KernelStack;

    /* Initialize the Power Management Support for this PRCB */
    PoInitializePrcb(Prcb);

    /* Save CPU state */
    KiSaveProcessorControlState(&Prcb->ProcessorState);

    /* Get cache line information for this CPU */
    KiGetCacheInformation();

    /* Initialize spinlocks and DPC data */
    KiInitSpinLocks(Prcb, Prcb->Number);

    /* Set up the thread-related fields in the PRCB */
    Prcb->CurrentThread = Thread;
    Prcb->NextThread = NULL;
    Prcb->IdleThread = Thread;

    /* Initialize PRCB pool lookaside pointers */
    ExInitPoolLookasidePointers();

    /* Lower to APC_LEVEL */
    KeLowerIrql(APC_LEVEL);

    /* Check if this is the boot cpu */
    if (Prcb->Number == 0)
    {
        /* Initialize the kernel */
        KiInitializeKernel(Process, Thread, KernelStack, Prcb, LoaderBlock);
    }
    else
    {
        /* Initialize the startup thread */
        KiInitializeHandBuiltThread(Thread, Process, KernelStack);

        /* Initialize cpu with HAL */
        if (!HalInitSystem(0, LoaderBlock))
        {
            /* Initialization failed */
            KeBugCheck(HAL_INITIALIZATION_FAILED);
        }
    }

    /* Raise to Dispatch */
    KfRaiseIrql(DISPATCH_LEVEL);

    /* Set the Idle Priority to 0. This will jump into Phase 1 */
    KeSetPriorityThread(Thread, 0);

    /* If there's no thread scheduled, put this CPU in the Idle summary */
    KiAcquirePrcbLock(Prcb);
    if (!Prcb->NextThread) KiIdleSummary |= (ULONG_PTR)1 << Prcb->Number;
    KiReleasePrcbLock(Prcb);

    /* Raise back to HIGH_LEVEL and clear the PRCB for the loader block */
    KfRaiseIrql(HIGH_LEVEL);
    LoaderBlock->Prcb = 0;

    /* Set the priority of this thread to 0 */
    Thread = KeGetCurrentThread();
    Thread->Priority = 0;

    /* Force interrupts enabled and lower IRQL back to DISPATCH_LEVEL */
    _enable();
    KeLowerIrql(DISPATCH_LEVEL);

    /* Set the right wait IRQL */
    Thread->WaitIrql = DISPATCH_LEVEL;

    /* Jump into the idle loop */
    KiIdleLoop();
}

VOID
NTAPI
INIT_FUNCTION
KiInitializeKernel(IN PKPROCESS InitProcess,
                   IN PKTHREAD InitThread,
                   IN PVOID IdleStack,
                   IN PKPRCB Prcb,
                   IN PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    ULONG_PTR PageDirectory[2];
    PVOID DpcStack;
    ULONG i;

    /* Set Node Data */
    KeNodeBlock[0] = &KiNode0;
    Prcb->ParentNode = KeNodeBlock[0];
    KeNodeBlock[0]->ProcessorMask = Prcb->SetMember;

    /* Set boot-level flags */
    KeFeatureBits = Prcb->FeatureBits;

    /* Set the current MP Master KPRCB to the Boot PRCB */
    Prcb->MultiThreadSetMaster = Prcb;

    /* Initialize Bugcheck Callback data */
    InitializeListHead(&KeBugcheckCallbackListHead);
    InitializeListHead(&KeBugcheckReasonCallbackListHead);
    KeInitializeSpinLock(&BugCheckCallbackLock);

    /* Initialize the Timer Expiration DPC */
    KeInitializeDpc(&KiTimerExpireDpc, KiTimerExpiration, NULL);
    KeSetTargetProcessorDpc(&KiTimerExpireDpc, 0);

    /* Initialize Profiling data */
    KeInitializeSpinLock(&KiProfileLock);
    InitializeListHead(&KiProfileListHead);
    InitializeListHead(&KiProfileSourceListHead);

    /* Loop the timer table */
    for (i = 0; i < TIMER_TABLE_SIZE; i++)
    {
        /* Initialize the list and entries */
        InitializeListHead(&KiTimerTableListHead[i].Entry);
        KiTimerTableListHead[i].Time.HighPart = 0xFFFFFFFF;
        KiTimerTableListHead[i].Time.LowPart = 0;
    }

    /* Initialize the Swap event and all swap lists */
    KeInitializeEvent(&KiSwapEvent, SynchronizationEvent, FALSE);
    InitializeListHead(&KiProcessInSwapListHead);
    InitializeListHead(&KiProcessOutSwapListHead);
    InitializeListHead(&KiStackInSwapListHead);

    /* Initialize the mutex for generic DPC calls */
    ExInitializeFastMutex(&KiGenericCallDpcMutex);

    /* Initialize the syscall table */
    KeServiceDescriptorTable[0].Base = MainSSDT;
    KeServiceDescriptorTable[0].Count = NULL;
    KeServiceDescriptorTable[0].Limit = KiServiceLimit;
    KeServiceDescriptorTable[1].Limit = 0;
    KeServiceDescriptorTable[0].Number = MainSSPT;

    /* Copy the the current table into the shadow table for win32k */
    RtlCopyMemory(KeServiceDescriptorTableShadow,
                  KeServiceDescriptorTable,
                  sizeof(KeServiceDescriptorTable));

    /* Initialize the Idle Process and the Process Listhead */
    InitializeListHead(&KiProcessListHead);
    PageDirectory[0] = 0;
    PageDirectory[1] = 0;
    KeInitializeProcess(InitProcess,
                        0,
                        0xFFFFFFFF,
                        PageDirectory,
                        FALSE);
    InitProcess->QuantumReset = MAXCHAR;

    /* Initialize the startup thread */
    KiInitializeHandBuiltThread(InitThread, InitProcess, IdleStack);

    /* Initialize the Kernel Executive */
    ExpInitializeExecutive(0, LoaderBlock);

    /* Calculate the time reciprocal */
    KiTimeIncrementReciprocal =
        KiComputeReciprocal(KeMaximumIncrement,
                            &KiTimeIncrementShiftCount);

    /* Update DPC Values in case they got updated by the executive */
    Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
    Prcb->MinimumDpcRate = KiMinimumDpcRate;
    Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;

    /* Allocate the DPC Stack */
    DpcStack = MmCreateKernelStack(FALSE, 0);
    if (!DpcStack) KeBugCheckEx(NO_PAGES_AVAILABLE, 1, 0, 0, 0);
    Prcb->DpcStack = DpcStack;
}

