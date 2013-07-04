/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/ke/arm/kiinit.c
 * PURPOSE:         Implements the kernel entry point for ARM machines
 * PROGRAMMERS:     ReactOS Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

KINTERRUPT KxUnexpectedInterrupt;
BOOLEAN KeIsArmV6;
ULONG KeNumberProcessIds;
ULONG KeNumberTbEntries;
ULONG ProcessCount; // PERF
extern PVOID KiArmVectorTable;
#define __ARMV6__ KeIsArmV6

/* FUNCTIONS ******************************************************************/

VOID
NTAPI
KiInitMachineDependent(VOID)
{
    /* There is nothing to do on ARM */
    return;
}

VOID
NTAPI
KiInitializeKernel(IN PKPROCESS InitProcess,
                   IN PKTHREAD InitThread,
                   IN PVOID IdleStack,
                   IN PKPRCB Prcb,
                   IN CCHAR Number,
                   IN PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    PKIPCR Pcr = (PKIPCR)KeGetPcr();
    ULONG PageDirectory[2];
    ULONG i;
    
    /* Set the default NX policy (opt-in) */
    SharedUserData->NXSupportPolicy = NX_SUPPORT_POLICY_OPTIN;

    /* Initialize spinlocks and DPC data */
    KiInitSpinLocks(Prcb, Number);
    
    /* Set stack pointers */
    Pcr->InitialStack = IdleStack;

    /* Check if this is the Boot CPU */
    if (!Number)
    {
        /* Setup the unexpected interrupt */
        KxUnexpectedInterrupt.DispatchAddress = KiUnexpectedInterrupt;
        for (i = 0; i < 4; i++)
        {
            /* Copy the template code */
            KxUnexpectedInterrupt.DispatchCode[i] = ((PULONG)KiInterruptTemplate)[i];
        }
        
        /* Set DMA coherency */
        KiDmaIoCoherency = 0;
        
        /* Sweep D-Cache */
        HalSweepDcache();
        
        /* Set boot-level flags */
        KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM;
        KeFeatureBits = 0;
        KeProcessorLevel = (USHORT)(Pcr->ProcessorId >> 8);
        KeProcessorRevision = (USHORT)(Pcr->ProcessorId & 0xFF);

        /* Set the current MP Master KPRCB to the Boot PRCB */
        Prcb->MultiThreadSetMaster = Prcb;

        /* Lower to APC_LEVEL */
        KeLowerIrql(APC_LEVEL);

        /* Initialize portable parts of the OS */
        KiInitSystem();

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
    }
    else
    {
        /* FIXME-V6: See if we want to support MP */
        DPRINT1("ARM MPCore not supported\n");
    }

    /* Setup the Idle Thread */
    KeInitializeThread(InitProcess,
                       InitThread,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       IdleStack);
    InitThread->NextProcessor = Number;
    InitThread->Priority = HIGH_PRIORITY;
    InitThread->State = Running;
    InitThread->Affinity = 1 << Number;
    InitThread->WaitIrql = DISPATCH_LEVEL;
    InitProcess->ActiveProcessors = 1 << Number;

    /* HACK for MmUpdatePageDir */
    ((PETHREAD)InitThread)->ThreadsProcess = (PEPROCESS)InitProcess;

    /* Set up the thread-related fields in the PRCB */
    Prcb->CurrentThread = InitThread;
    Prcb->NextThread = NULL;
    Prcb->IdleThread = InitThread;

    /* Initialize the Kernel Executive */
    ExpInitializeExecutive(Number, LoaderBlock);

    /* Only do this on the boot CPU */
    if (!Number)
    {
        /* Calculate the time reciprocal */
        KiTimeIncrementReciprocal =
            KiComputeReciprocal(KeMaximumIncrement,
                                &KiTimeIncrementShiftCount);

        /* Update DPC Values in case they got updated by the executive */
        Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
        Prcb->MinimumDpcRate = KiMinimumDpcRate;
        Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;
    }

    /* Raise to Dispatch */
    KfRaiseIrql(DISPATCH_LEVEL);

    /* Set the Idle Priority to 0. This will jump into Phase 1 */
    KeSetPriorityThread(InitThread, 0);

    /* If there's no thread scheduled, put this CPU in the Idle summary */
    KiAcquirePrcbLock(Prcb);
    if (!Prcb->NextThread) KiIdleSummary |= 1 << Number;
    KiReleasePrcbLock(Prcb);

    /* Raise back to HIGH_LEVEL and clear the PRCB for the loader block */
    KfRaiseIrql(HIGH_LEVEL);
    LoaderBlock->Prcb = 0;
}

C_ASSERT((PKIPCR)KeGetPcr() == (PKIPCR)0xFFDFF000);
C_ASSERT((FIELD_OFFSET(KIPCR, FirstLevelDcacheSize) & 4) == 0);
C_ASSERT(sizeof(KIPCR) <= PAGE_SIZE);

VOID
NTAPI
KiInitializePcr(IN ULONG ProcessorNumber,
                IN PKIPCR Pcr,
                IN PKTHREAD IdleThread,
                IN PVOID PanicStack,
                IN PVOID InterruptStack)
{
    ULONG i;
    
    /* Set the Current Thread */
    Pcr->PrcbData.CurrentThread = IdleThread;

    /* Set pointers to ourselves */
    Pcr->Self = (PKPCR)Pcr;
    Pcr->Prcb = &Pcr->PrcbData;

    /* Set the PCR Version */
    Pcr->MajorVersion = PCR_MAJOR_VERSION;
    Pcr->MinorVersion = PCR_MINOR_VERSION;

    /* Set the PCRB Version */
    Pcr->PrcbData.MajorVersion = 1;
    Pcr->PrcbData.MinorVersion = 1;

    /* Set the Build Type */
    Pcr->PrcbData.BuildType = 0;
#ifndef CONFIG_SMP
    Pcr->PrcbData.BuildType |= PRCB_BUILD_UNIPROCESSOR;
#endif
#if DBG
    Pcr->PrcbData.BuildType |= PRCB_BUILD_DEBUG;
#endif

    /* Set the Processor Number and current Processor Mask */
    Pcr->PrcbData.Number = (UCHAR)ProcessorNumber;
    Pcr->PrcbData.SetMember = 1 << ProcessorNumber;

    /* Set the PRCB for this Processor */
    KiProcessorBlock[ProcessorNumber] = Pcr->Prcb;

    /* Start us out at PASSIVE_LEVEL */
    Pcr->Irql = PASSIVE_LEVEL;

    /* Set the stacks */
    Pcr->PanicStack = PanicStack;
    Pcr->InterruptStack = InterruptStack;

    /* Setup the processor set */
    Pcr->PrcbData.MultiThreadProcessorSet = Pcr->PrcbData.SetMember;
    
    /* Copy cache information from the loader block */
    Pcr->FirstLevelDcacheSize = KeLoaderBlock->u.Arm.FirstLevelDcacheSize;
    Pcr->SecondLevelDcacheSize = KeLoaderBlock->u.Arm.SecondLevelDcacheSize;
    Pcr->FirstLevelIcacheSize = KeLoaderBlock->u.Arm.FirstLevelIcacheSize;
    Pcr->SecondLevelIcacheSize = KeLoaderBlock->u.Arm.SecondLevelIcacheSize;
    Pcr->FirstLevelDcacheFillSize = KeLoaderBlock->u.Arm.FirstLevelDcacheFillSize;
    Pcr->SecondLevelDcacheFillSize = KeLoaderBlock->u.Arm.SecondLevelDcacheFillSize;
    Pcr->FirstLevelIcacheFillSize = KeLoaderBlock->u.Arm.FirstLevelIcacheFillSize;
    Pcr->SecondLevelIcacheFillSize = KeLoaderBlock->u.Arm.SecondLevelIcacheFillSize;

    /* Set global d-cache fill and alignment values */
    if (!Pcr->SecondLevelDcacheSize)
    {
        /* Use the first level */
        Pcr->DcacheFillSize = Pcr->FirstLevelDcacheSize;
    }
    else
    {
        /* Use the second level */
        Pcr->DcacheFillSize = Pcr->SecondLevelDcacheSize;
    }
    
    /* Set the alignment */
    Pcr->DcacheAlignment = Pcr->DcacheFillSize - 1;
    
    /* Set global i-cache fill and alignment values */
    if (!Pcr->SecondLevelIcacheSize)
    {
        /* Use the first level */
        Pcr->IcacheFillSize = Pcr->FirstLevelIcacheSize;
    }
    else
    {
        /* Use the second level */
        Pcr->IcacheFillSize = Pcr->SecondLevelIcacheSize;
    }
    
    /* Set the alignment */
    Pcr->IcacheAlignment = Pcr->IcacheFillSize - 1;
    
    /* Set processor information */
    Pcr->ProcessorId = KeArmIdCodeRegisterGet().AsUlong;
    
    /* Set all interrupt routines to unexpected interrupts as well */
    for (i = 0; i < MAXIMUM_VECTOR; i++)
    {
        /* Point to the same template */
        Pcr->InterruptRoutine[i] = (PVOID)&KxUnexpectedInterrupt.DispatchCode;
    }

    /* Set default stall factor */
    Pcr->StallScaleFactor = 50;
    
    /* Setup software interrupts */
    Pcr->InterruptRoutine[PASSIVE_LEVEL] = KiPassiveRelease;
    Pcr->InterruptRoutine[APC_LEVEL] = KiApcInterrupt;
    Pcr->InterruptRoutine[DISPATCH_LEVEL] = KiDispatchInterrupt;
    Pcr->ReservedVectors = (1 << PASSIVE_LEVEL) |
                           (1 << APC_LEVEL) |
                           (1 << DISPATCH_LEVEL) |
                           (1 << IPI_LEVEL);
}

VOID
KiInitializeMachineType(VOID)
{
    /* Detect ARM version */
    KeIsArmV6 = KeArmIdCodeRegisterGet().Architecture >= 7;
    
    /* Set the number of TLB entries and ASIDs */
    KeNumberTbEntries = 64;
    if (__ARMV6__)
    {
        /* 256 ASIDs on v6/v7 */
        KeNumberProcessIds = 256;
    }
    else
    {
        /* The TLB is VIVT on v4/v5 */
        KeNumberProcessIds = 0;
    }
}

VOID
KiInitializeSystem(IN PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    ULONG Cpu;
    PKTHREAD InitialThread;
    PKPROCESS InitialProcess;
    ARM_CONTROL_REGISTER ControlRegister;
    PKIPCR Pcr = (PKIPCR)KeGetPcr();
    PKTHREAD Thread;

    /* Flush the TLB */
    KeFlushTb();
    
    /* Save the loader block and get the current CPU */
    KeLoaderBlock = LoaderBlock;
    Cpu = KeNumberProcessors;

    /* Save the initial thread and process */
    InitialThread = (PKTHREAD)LoaderBlock->Thread;
    InitialProcess = (PKPROCESS)LoaderBlock->Process;

    /* Clean the APC List Head */
    InitializeListHead(&InitialThread->ApcState.ApcListHead[KernelMode]);

    /* Initialize the machine type */
    KiInitializeMachineType();

    /* Skip initial setup if this isn't the Boot CPU */
    if (Cpu) goto AppCpuInit;
    
    /* Initialize the PCR */
    RtlZeroMemory(Pcr, PAGE_SIZE);
    KiInitializePcr(Cpu,
                    Pcr,
                    InitialThread,
                    (PVOID)LoaderBlock->u.Arm.PanicStack,
                    (PVOID)LoaderBlock->u.Arm.InterruptStack);

    /* Now sweep caches */
    HalSweepIcache();
    HalSweepDcache();
    
    /* Set us as the current process */
    InitialThread->ApcState.Process = InitialProcess;
    
AppCpuInit:
    /* Setup CPU-related fields */
    Pcr->Number = Cpu;
    Pcr->SetMember = 1 << Cpu;
    Pcr->SetMemberCopy = 1 << Cpu;
    Pcr->PrcbData.SetMember = 1 << Cpu;
    
    /* Initialize the Processor with HAL */
    HalInitializeProcessor(Cpu, KeLoaderBlock);

    /* Set active processors */
    KeActiveProcessors |= Pcr->SetMember;
    KeNumberProcessors++;

    /* Check if this is the boot CPU */
    if (!Cpu)
    {
        /* Initialize debugging system */
        KdInitSystem(0, KeLoaderBlock);

        /* Check for break-in */
        if (KdPollBreakIn()) DbgBreakPointWithStatus(DBG_STATUS_CONTROL_C);
    }

    /* Raise to HIGH_LEVEL */
    KfRaiseIrql(HIGH_LEVEL);
    
    /* Set the exception address to high */
    ControlRegister = KeArmControlRegisterGet();
    ControlRegister.HighVectors = TRUE;
    KeArmControlRegisterSet(ControlRegister);
    
    /* Setup the exception vector table */
    RtlCopyMemory((PVOID)0xFFFF0000, &KiArmVectorTable, 14 * sizeof(PVOID));

    /* Initialize the rest of the kernel now */
    KiInitializeKernel((PKPROCESS)LoaderBlock->Process,
                       (PKTHREAD)LoaderBlock->Thread,
                       (PVOID)LoaderBlock->KernelStack,
                       Pcr->Prcb,
                       Pcr->Prcb->Number,
                       KeLoaderBlock);

    /* Set the priority of this thread to 0 */
    Thread = KeGetCurrentThread();
    Thread->Priority = 0;

    /* Force interrupts enabled and lower IRQL back to DISPATCH_LEVEL */
    _enable();
    KfLowerIrql(DISPATCH_LEVEL);

    /* Set the right wait IRQL */
    Thread->WaitIrql = DISPATCH_LEVEL;

    /* Jump into the idle loop */
    KiIdleLoop();
}

ULONG
DbgPrintEarly(const char *fmt, ...)
{
    va_list args;
    unsigned int i;
    char Buffer[1024];
    PCHAR String = Buffer;

    va_start(args, fmt);
    i = vsprintf(Buffer, fmt, args);
    va_end(args);
    
    /* Output the message */
    while (*String != 0)
    {
        if (*String == '\n')
        {
            KdPortPutByteEx(NULL, '\r');
        }
        KdPortPutByteEx(NULL, *String);
        String++;
    }
    
    return STATUS_SUCCESS;
}
