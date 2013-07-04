/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/mm/balance.c
 * PURPOSE:         kernel memory managment functions
 *
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 *                  Cameron Gutman (cameron.gutman@reactos.org)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

#include "ARM3/miarm.h"

#if defined (ALLOC_PRAGMA)
#pragma alloc_text(INIT, MmInitializeBalancer)
#pragma alloc_text(INIT, MmInitializeMemoryConsumer)
#pragma alloc_text(INIT, MiInitBalancerThread)
#endif


/* TYPES ********************************************************************/
typedef struct _MM_ALLOCATION_REQUEST
{
   PFN_NUMBER Page;
   LIST_ENTRY ListEntry;
   KEVENT Event;
}
MM_ALLOCATION_REQUEST, *PMM_ALLOCATION_REQUEST;
/* GLOBALS ******************************************************************/

MM_MEMORY_CONSUMER MiMemoryConsumers[MC_MAXIMUM];
static ULONG MiMinimumAvailablePages;
static ULONG MiNrTotalPages;
static LIST_ENTRY AllocationListHead;
static KSPIN_LOCK AllocationListLock;
static ULONG MiMinimumPagesPerRun;

static CLIENT_ID MiBalancerThreadId;
static HANDLE MiBalancerThreadHandle = NULL;
static KEVENT MiBalancerEvent;
static KTIMER MiBalancerTimer;

/* FUNCTIONS ****************************************************************/

VOID
INIT_FUNCTION
NTAPI
MmInitializeBalancer(ULONG NrAvailablePages, ULONG NrSystemPages)
{
   memset(MiMemoryConsumers, 0, sizeof(MiMemoryConsumers));
   InitializeListHead(&AllocationListHead);
   KeInitializeSpinLock(&AllocationListLock);

   MiNrTotalPages = NrAvailablePages;

   /* Set up targets. */
   MiMinimumAvailablePages = 128;
   MiMinimumPagesPerRun = 256;
    if ((NrAvailablePages + NrSystemPages) >= 8192)
    {
        MiMemoryConsumers[MC_CACHE].PagesTarget = NrAvailablePages / 4 * 3;
    }
    else if ((NrAvailablePages + NrSystemPages) >= 4096)
    {
        MiMemoryConsumers[MC_CACHE].PagesTarget = NrAvailablePages / 3 * 2;
    }
    else
    {
        MiMemoryConsumers[MC_CACHE].PagesTarget = NrAvailablePages / 8;
    }
   MiMemoryConsumers[MC_USER].PagesTarget = NrAvailablePages - MiMinimumAvailablePages;
}

VOID
INIT_FUNCTION
NTAPI
MmInitializeMemoryConsumer(ULONG Consumer,
                           NTSTATUS (*Trim)(ULONG Target, ULONG Priority,
                                            PULONG NrFreed))
{
   MiMemoryConsumers[Consumer].Trim = Trim;
}

VOID
NTAPI
MiZeroPhysicalPage(
    IN PFN_NUMBER PageFrameIndex
);

NTSTATUS
NTAPI
MmReleasePageMemoryConsumer(ULONG Consumer, PFN_NUMBER Page)
{
   PMM_ALLOCATION_REQUEST Request;
   PLIST_ENTRY Entry;
   KIRQL OldIrql;

   if (Page == 0)
   {
      DPRINT1("Tried to release page zero.\n");
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   if (MmGetReferenceCountPage(Page) == 1)
   {
      if(Consumer == MC_USER) MmRemoveLRUUserPage(Page);
      (void)InterlockedDecrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
      if ((Entry = ExInterlockedRemoveHeadList(&AllocationListHead, &AllocationListLock)) == NULL)
      {
         OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
         MmDereferencePage(Page);
         KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
      }
      else
      {
         Request = CONTAINING_RECORD(Entry, MM_ALLOCATION_REQUEST, ListEntry);
         MiZeroPhysicalPage(Page);
         Request->Page = Page;
         KeSetEvent(&Request->Event, IO_NO_INCREMENT, FALSE);
      }
   }
   else
   {
      OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
      MmDereferencePage(Page);
      KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
   }

   return(STATUS_SUCCESS);
}

ULONG
NTAPI
MiTrimMemoryConsumer(ULONG Consumer, ULONG InitialTarget)
{
    ULONG Target = InitialTarget;
    ULONG NrFreedPages = 0;
    NTSTATUS Status;

    /* Make sure we can trim this consumer */
    if (!MiMemoryConsumers[Consumer].Trim)
    {
        /* Return the unmodified initial target */
        return InitialTarget;
    }

    if (MiMemoryConsumers[Consumer].PagesUsed > MiMemoryConsumers[Consumer].PagesTarget)
    {
        /* Consumer page limit exceeded */
        Target = max(Target, MiMemoryConsumers[Consumer].PagesUsed - MiMemoryConsumers[Consumer].PagesTarget);
    }
    if (MmAvailablePages < MiMinimumAvailablePages)
    {
        /* Global page limit exceeded */
        Target = (ULONG)max(Target, MiMinimumAvailablePages - MmAvailablePages);
    }

    if (Target)
    {
        if (!InitialTarget)
        {
            /* If there was no initial target,
             * swap at least MiMinimumPagesPerRun */
            Target = max(Target, MiMinimumPagesPerRun);
        }

        /* Now swap the pages out */
        Status = MiMemoryConsumers[Consumer].Trim(Target, 0, &NrFreedPages);

        DPRINT("Trimming consumer %d: Freed %d pages with a target of %d pages\n", Consumer, NrFreedPages, Target);

        if (!NT_SUCCESS(Status))
        {
            KeBugCheck(MEMORY_MANAGEMENT);
        }

        /* Update the target */
        if (NrFreedPages < Target)
            Target -= NrFreedPages;
        else
            Target = 0;

        /* Return the remaining pages needed to meet the target */
        return Target;
    }
    else
    {
        /* Initial target is zero and we don't have anything else to add */
        return 0;
    }
}

NTSTATUS
MmTrimUserMemory(ULONG Target, ULONG Priority, PULONG NrFreedPages)
{
    PFN_NUMBER CurrentPage;
    PFN_NUMBER NextPage;
    NTSTATUS Status;

    (*NrFreedPages) = 0;

    CurrentPage = MmGetLRUFirstUserPage();
    while (CurrentPage != 0 && Target > 0)
    {
        Status = MmPageOutPhysicalAddress(CurrentPage);
        if (NT_SUCCESS(Status))
        {
            DPRINT("Succeeded\n");
            Target--;
            (*NrFreedPages)++;
        }

        NextPage = MmGetLRUNextUserPage(CurrentPage);
        if (NextPage <= CurrentPage)
        {
            /* We wrapped around, so we're done */
            break;
        }
        CurrentPage = NextPage;
    }

    return STATUS_SUCCESS;
}

static BOOLEAN
MiIsBalancerThread(VOID)
{
   return (MiBalancerThreadHandle != NULL) &&
          (PsGetCurrentThreadId() == MiBalancerThreadId.UniqueThread);
}

VOID
NTAPI
MiDeletePte(IN PMMPTE PointerPte,
            IN PVOID VirtualAddress,
            IN PEPROCESS CurrentProcess,
            IN PMMPTE PrototypePte);

VOID
NTAPI
MmRebalanceMemoryConsumers(VOID)
{
    if (MiBalancerThreadHandle != NULL &&
        !MiIsBalancerThread())
    {
        KeSetEvent(&MiBalancerEvent, IO_NO_INCREMENT, FALSE);
    }
}

NTSTATUS
NTAPI
MmRequestPageMemoryConsumer(ULONG Consumer, BOOLEAN CanWait,
                            PPFN_NUMBER AllocatedPage)
{
   ULONG PagesUsed;
   PFN_NUMBER Page;
   KIRQL OldIrql;

   /*
    * Make sure we don't exceed our individual target.
    */
   PagesUsed = InterlockedIncrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
   if (PagesUsed > MiMemoryConsumers[Consumer].PagesTarget &&
       !MiIsBalancerThread())
   {
      MmRebalanceMemoryConsumers();
   }

   /*
    * Allocate always memory for the non paged pool and for the pager thread.
    */
   if ((Consumer == MC_SYSTEM) || MiIsBalancerThread())
   {
      OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
      Page = MmAllocPage(Consumer);
      KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
      if (Page == 0)
      {
         KeBugCheck(NO_PAGES_AVAILABLE);
      }
      if (Consumer == MC_USER) MmInsertLRULastUserPage(Page);
      *AllocatedPage = Page;
      if (MmAvailablePages < MiMinimumAvailablePages)
          MmRebalanceMemoryConsumers();
      return(STATUS_SUCCESS);
   }

   /*
    * Make sure we don't exceed global targets.
    */
   if (MmAvailablePages < MiMinimumAvailablePages)
   {
      MM_ALLOCATION_REQUEST Request;

      if (!CanWait)
      {
         (void)InterlockedDecrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
         MmRebalanceMemoryConsumers();
         return(STATUS_NO_MEMORY);
      }

      /* Insert an allocation request. */
      Request.Page = 0;
      KeInitializeEvent(&Request.Event, NotificationEvent, FALSE);

      ExInterlockedInsertTailList(&AllocationListHead, &Request.ListEntry, &AllocationListLock);
      MmRebalanceMemoryConsumers();

      KeWaitForSingleObject(&Request.Event,
                            0,
                            KernelMode,
                            FALSE,
                            NULL);

      Page = Request.Page;
      if (Page == 0)
      {
         KeBugCheck(NO_PAGES_AVAILABLE);
      }

      if(Consumer == MC_USER) MmInsertLRULastUserPage(Page);
      *AllocatedPage = Page;

      if (MmAvailablePages < MiMinimumAvailablePages)
      {
          MmRebalanceMemoryConsumers();
      }

      return(STATUS_SUCCESS);
   }

   /*
    * Actually allocate the page.
    */
   OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
   Page = MmAllocPage(Consumer);
   KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
   if (Page == 0)
   {
      KeBugCheck(NO_PAGES_AVAILABLE);
   }
   if(Consumer == MC_USER) MmInsertLRULastUserPage(Page);
   *AllocatedPage = Page;

   if (MmAvailablePages < MiMinimumAvailablePages)
   {
       MmRebalanceMemoryConsumers();
   }

   return(STATUS_SUCCESS);
}

VOID NTAPI
MiBalancerThread(PVOID Unused)
{
   PVOID WaitObjects[2];
   NTSTATUS Status;
   ULONG i;

   WaitObjects[0] = &MiBalancerEvent;
   WaitObjects[1] = &MiBalancerTimer;

   while (1)
   {
      Status = KeWaitForMultipleObjects(2,
                                        WaitObjects,
                                        WaitAny,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL,
                                        NULL);

      if (Status == STATUS_WAIT_0 || Status == STATUS_WAIT_1)
      {
        ULONG InitialTarget = 0;

#if (_MI_PAGING_LEVELS == 2)
        if(!MiIsBalancerThread())
        {
            /* Clean up the unused PDEs */
            ULONG_PTR Address;
            PEPROCESS Process = PsGetCurrentProcess();

            /* Acquire PFN lock */
            KIRQL OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
            PMMPDE pointerPde;
            for(Address = (ULONG_PTR)MI_LOWEST_VAD_ADDRESS;
                Address < (ULONG_PTR)MM_HIGHEST_VAD_ADDRESS;
                Address += (PAGE_SIZE * PTE_COUNT))
            {
                if(MmWorkingSetList->UsedPageTableEntries[MiGetPdeOffset(Address)] == 0)
                {
                    pointerPde = MiAddressToPde(Address);
                    if(pointerPde->u.Hard.Valid)
                        MiDeletePte(pointerPde, MiPdeToPte(pointerPde), Process, NULL);
                    ASSERT(pointerPde->u.Hard.Valid == 0);
                }
            }
            /* Release lock */
            KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
        }
#endif
          do
          {
              ULONG OldTarget = InitialTarget;

              /* Trim each consumer */
              for (i = 0; i < MC_MAXIMUM; i++)
              {
                  InitialTarget = MiTrimMemoryConsumer(i, InitialTarget);
              }

              /* No pages left to swap! */
              if (InitialTarget != 0 &&
                  InitialTarget == OldTarget)
              {
                  /* Game over */
                  KeBugCheck(NO_PAGES_AVAILABLE);
              }
          } while (InitialTarget != 0);
      }
      else
      {
         DPRINT1("KeWaitForMultipleObjects failed, status = %x\n", Status);
         KeBugCheck(MEMORY_MANAGEMENT);
      }
   }
}

VOID
INIT_FUNCTION
NTAPI
MiInitBalancerThread(VOID)
{
   KPRIORITY Priority;
   NTSTATUS Status;
#if !defined(__GNUC__)

   LARGE_INTEGER dummyJunkNeeded;
   dummyJunkNeeded.QuadPart = -20000000; /* 2 sec */
   ;
#endif


   KeInitializeEvent(&MiBalancerEvent, SynchronizationEvent, FALSE);
   KeInitializeTimerEx(&MiBalancerTimer, SynchronizationTimer);
   KeSetTimerEx(&MiBalancerTimer,
#if defined(__GNUC__)
                (LARGE_INTEGER)(LONGLONG)-20000000LL,     /* 2 sec */
#else
                dummyJunkNeeded,
#endif
                2000,         /* 2 sec */
                NULL);

   Status = PsCreateSystemThread(&MiBalancerThreadHandle,
                                 THREAD_ALL_ACCESS,
                                 NULL,
                                 NULL,
                                 &MiBalancerThreadId,
                                 (PKSTART_ROUTINE) MiBalancerThread,
                                 NULL);
   if (!NT_SUCCESS(Status))
   {
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   Priority = LOW_REALTIME_PRIORITY + 1;
   NtSetInformationThread(MiBalancerThreadHandle,
                          ThreadPriority,
                          &Priority,
                          sizeof(Priority));

}


/* EOF */
