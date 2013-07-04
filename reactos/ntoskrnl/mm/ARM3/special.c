/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/mm/ARM3/special.c
 * PURPOSE:         ARM Memory Manager Special Pool implementation
 * PROGRAMMERS:     ReactOS Portable Systems Group
 */

/*
    References:
    http://msdn.microsoft.com/en-us/library/ff551832(v=VS.85).aspx
*/

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

#define MODULE_INVOLVED_IN_ARM3
#include "../ARM3/miarm.h"

extern PMMPTE MmSystemPteBase;

PMMPTE
NTAPI
MiReserveAlignedSystemPtes(IN ULONG NumberOfPtes,
                           IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType,
                           IN ULONG Alignment);

/* GLOBALS ********************************************************************/

#define SPECIAL_POOL_PAGED_PTE    0x2000
#define SPECIAL_POOL_NONPAGED_PTE 0x4000
#define SPECIAL_POOL_PAGED        0x8000

PVOID MmSpecialPoolStart;
PVOID MmSpecialPoolEnd;
PVOID MiSpecialPoolExtra;
ULONG MiSpecialPoolExtraCount;

PMMPTE MiSpecialPoolFirstPte;
PMMPTE MiSpecialPoolLastPte;

PFN_NUMBER MiSpecialPagesNonPagedMaximum;

BOOLEAN MmSpecialPoolCatchOverruns = TRUE;

typedef struct _MI_FREED_SPECIAL_POOL
{
    POOL_HEADER OverlaidPoolHeader;
    /* TODO: Add overlaid verifier pool header */
    ULONG Signature;
    ULONG TickCount;
    ULONG NumberOfBytesRequested;
    BOOLEAN Pagable;
    PVOID VirtualAddress;
    PVOID StackPointer;
    ULONG StackBytes;
    PETHREAD Thread;
    UCHAR StackData[0x400];
} MI_FREED_SPECIAL_POOL, *PMI_FREED_SPECIAL_POOL;

/* PRIVATE FUNCTIONS **********************************************************/

VOID NTAPI MiTestSpecialPool();

BOOLEAN
NTAPI
MmUseSpecialPool(SIZE_T NumberOfBytes, ULONG Tag)
{
    /* Special pool is not suitable for allocations bigger than 1 page */
    if (NumberOfBytes > (PAGE_SIZE - sizeof(POOL_HEADER)))
        return FALSE;

    // FIXME
    //return TRUE;
    return FALSE;
}

BOOLEAN
NTAPI
MmIsSpecialPoolAddress(PVOID P)
{
    return((P >= MmSpecialPoolStart) &&
           (P <= MmSpecialPoolEnd));
}

VOID
NTAPI
MiInitializeSpecialPool()
{
    ULONG SpecialPoolPtes, i;
    PMMPTE PointerPte;

    /* Check if there is a special pool tag */
    if ((MmSpecialPoolTag == 0) ||
        (MmSpecialPoolTag == -1)) return;

    /* Calculate number of system PTEs for the special pool */
    if ( MmNumberOfSystemPtes >= 0x3000 )
        SpecialPoolPtes = MmNumberOfSystemPtes / 3;
    else
        SpecialPoolPtes = MmNumberOfSystemPtes / 6;

    /* Don't let the number go too high */
    if (SpecialPoolPtes > 0x6000) SpecialPoolPtes = 0x6000;

    /* Round up to the page size */
    SpecialPoolPtes = PAGE_ROUND_UP(SpecialPoolPtes);

    ASSERT((SpecialPoolPtes & (PTE_PER_PAGE - 1)) == 0);

    /* Reserve those PTEs */
    do
    {
        PointerPte = MiReserveAlignedSystemPtes(SpecialPoolPtes, 0, /*0x400000*/0); // FIXME:
        if (PointerPte) break;

        /* Reserving didn't work, so try to reduce the requested size */
        ASSERT(SpecialPoolPtes >= PTE_PER_PAGE);
        SpecialPoolPtes -= 1024;
    } while (SpecialPoolPtes);

    /* Fail if we couldn't reserve them at all */
    if (!SpecialPoolPtes) return;

    /* Make sure we got enough */
    ASSERT(SpecialPoolPtes >= PTE_PER_PAGE);

    /* Save first PTE and its address */
    MiSpecialPoolFirstPte = PointerPte;
    MmSpecialPoolStart = MiPteToAddress(PointerPte);

    for (i = 0; i<512; i++)
    {
        /* Point it to the next entry */
        PointerPte->u.List.NextEntry = &PointerPte[2] - MmSystemPteBase;

        /* Move to the next pair */
        PointerPte += 2;
    }

    /* Save extra values */
    MiSpecialPoolExtra = PointerPte;
    MiSpecialPoolExtraCount = SpecialPoolPtes - 1024;

    /* Mark the previous PTE as the last one */
    MiSpecialPoolLastPte = PointerPte - 2;
    MiSpecialPoolLastPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

    /* Save end address of the special pool */
    MmSpecialPoolEnd = MiPteToAddress(MiSpecialPoolLastPte + 1);

    /* Calculate maximum non-paged part of the special pool */
    MiSpecialPagesNonPagedMaximum = MmResidentAvailablePages >> 4;

    /* And limit it if it turned out to be too big */
    if (MmNumberOfPhysicalPages > 0x3FFF)
        MiSpecialPagesNonPagedMaximum = MmResidentAvailablePages >> 3;

    DPRINT1("Special pool start %p - end %p\n", MmSpecialPoolStart, MmSpecialPoolEnd);

    //MiTestSpecialPool();
}

PVOID
NTAPI
MmAllocateSpecialPool(SIZE_T NumberOfBytes, ULONG Tag, POOL_TYPE PoolType, ULONG SpecialType)
{
    KIRQL Irql;
    MMPTE TempPte = ValidKernelPte;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameNumber;
    LARGE_INTEGER TickCount;
    PVOID Entry;
    PPOOL_HEADER Header;

    DPRINT1("MmAllocateSpecialPool(%x %x %x %x)\n", NumberOfBytes, Tag, PoolType, SpecialType);

    /* Check if the pool is initialized and quit if it's not */
    if (!MiSpecialPoolFirstPte) return NULL;

    /* Get the pool type */
    PoolType &= BASE_POOL_TYPE_MASK;

    /* Check whether current IRQL matches the pool type */
    Irql = KeGetCurrentIrql();

    if (((PoolType == PagedPool) && (Irql > APC_LEVEL)) ||
        ((PoolType != PagedPool) && (Irql > DISPATCH_LEVEL)))
    {
        /* Bad caller */
        KeBugCheckEx(SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION, Irql, PoolType, NumberOfBytes, 0x30);
    }

    /* TODO: Take into account various limitations */
    /*if ((PoolType != NonPagedPool) &&
        MiSpecialPagesNonPaged > MiSpecialPagesNonPagedMaximum)*/

    /* Lock PFN database */
    Irql = KeAcquireQueuedSpinLock(LockQueuePfnLock);

    /* Reject allocation in case amount of available pages is too small */
    if (MmAvailablePages < 0x100)
    {
        /* Release the PFN database lock */
        KeReleaseQueuedSpinLock(LockQueuePfnLock, Irql);
        DPRINT1("Special pool: MmAvailablePages 0x%x is too small\n", MmAvailablePages);
        return NULL;
    }

    /* Reject allocation if special pool PTE list is exhausted */
    if (MiSpecialPoolFirstPte->u.List.NextEntry == MM_EMPTY_PTE_LIST)
    {
        /* Release the PFN database lock */
        KeReleaseQueuedSpinLock(LockQueuePfnLock, Irql);
        DPRINT1("Special pool: No PTEs left!\n");
        /* TODO: Expand the special pool */
        return NULL;
    }

    /* Save allocation time */
    KeQueryTickCount(&TickCount);

    /* Get a pointer to the first PTE */
    PointerPte = MiSpecialPoolFirstPte;

    /* Set the first PTE pointer to the next one in the list */
    MiSpecialPoolFirstPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    /* Allocate a physical page */
    PageFrameNumber = MiRemoveAnyPage(MI_GET_NEXT_COLOR());

    /* Initialize PFN and make it valid */
    TempPte.u.Hard.PageFrameNumber = PageFrameNumber;
    MiInitializePfnAndMakePteValid(PageFrameNumber, PointerPte, TempPte);

    /* Release the PFN database lock */
    KeReleaseQueuedSpinLock(LockQueuePfnLock, Irql);

    /* Put some content into the page. Low value of tick count would do */
    Entry = MiPteToAddress(PointerPte);
    RtlFillMemory(Entry, PAGE_SIZE, TickCount.LowPart);

    /* Calculate header and entry addresses */
    if ((SpecialType != 0) &&
        ((SpecialType == 1) || (!MmSpecialPoolCatchOverruns)))
    {
        /* We catch underruns. Data is at the beginning of the page */
        Header = (PPOOL_HEADER)((PUCHAR)Entry + PAGE_SIZE - sizeof(POOL_HEADER));
    }
    else
    {
        /* We catch overruns. Data is at the end of the page */
        Header = (PPOOL_HEADER)Entry;
        Entry = (PVOID)((ULONG_PTR)((PUCHAR)Entry - NumberOfBytes + PAGE_SIZE) & ~((LONG_PTR)sizeof(POOL_HEADER) - 1));
    }

    /* Initialize the header */
    RtlZeroMemory(Header, sizeof(POOL_HEADER));

    /* Save allocation size there */
    Header->Ulong1 = (ULONG)NumberOfBytes;

    /* Make sure it's all good */
    ASSERT((NumberOfBytes <= PAGE_SIZE - sizeof(POOL_HEADER)) &&
           (PAGE_SIZE <= 32 * 1024));

    /* Mark it as paged or nonpaged */
    if (PoolType == PagedPool)
    {
        /* Add pagedpool flag into the pool header too */
        Header->Ulong1 |= SPECIAL_POOL_PAGED;

        /* Also mark the next PTE as special-pool-paged */
        PointerPte[1].u.Soft.PageFileHigh |= SPECIAL_POOL_PAGED_PTE;
    }
    else
    {
        /* Mark the next PTE as special-pool-nonpaged */
        PointerPte[1].u.Soft.PageFileHigh |= SPECIAL_POOL_NONPAGED_PTE;
    }

    /* Finally save tag and put allocation time into the header's blocksize.
       That time will be used to check memory consistency within the allocated
       page. */
    Header->PoolTag = Tag;
    Header->BlockSize = (USHORT)TickCount.LowPart;
    DPRINT1("%p\n", Entry);
    return Entry;
}

VOID
NTAPI
MiSpecialPoolCheckPattern(PUCHAR P, PPOOL_HEADER Header)
{
    ULONG BytesToCheck, BytesRequested, Index;
    PUCHAR Ptr;

    /* Get amount of bytes user requested to be allocated by clearing out the paged mask */
    BytesRequested = (Header->Ulong1 & ~SPECIAL_POOL_PAGED) & 0xFFFF;

    /* Get a pointer to the end of user's area */
    Ptr = P + BytesRequested;

    /* Calculate how many bytes to check */
    BytesToCheck = (ULONG)((PUCHAR)PAGE_ALIGN(P) + PAGE_SIZE - Ptr);

    /* Remove pool header size if we're catching underruns */
    if (((ULONG_PTR)P & (PAGE_SIZE - 1)) == 0)
    {
        /* User buffer is located in the beginning of the page */
        BytesToCheck -= sizeof(POOL_HEADER);
    }

    /* Check the pattern after user buffer */
    for (Index = 0; Index < BytesToCheck; Index++)
    {
        /* Bugcheck if bytes don't match */
        if (Ptr[Index] != Header->BlockSize)
        {
            KeBugCheckEx(BAD_POOL_HEADER, (ULONG_PTR)P, (ULONG_PTR)&Ptr[Index], Header->BlockSize, 0x24);
        }
    }
}

VOID
NTAPI
MmFreeSpecialPool(PVOID P)
{
    PMMPTE PointerPte;
    PPOOL_HEADER Header;
    BOOLEAN Overruns = FALSE;
    KIRQL Irql = KeGetCurrentIrql();
    POOL_TYPE PoolType;
    ULONG BytesRequested, BytesReal = 0;
    ULONG PtrOffset;
    PUCHAR b;
    PMI_FREED_SPECIAL_POOL FreedHeader;
    LARGE_INTEGER TickCount;
    PMMPFN Pfn;

    DPRINT1("MmFreeSpecialPool(%p)\n", P);

    /* Get the PTE */
    PointerPte = MiAddressToPte(P);

    /* Check if it's valid */
    if (PointerPte->u.Hard.Valid == 0)
    {
        /* Bugcheck if it has NOACCESS or 0 set as protection */
        if (PointerPte->u.Soft.Protection == MM_NOACCESS ||
            !PointerPte->u.Soft.Protection)
        {
            KeBugCheckEx(BAD_POOL_HEADER, (ULONG_PTR)P, (ULONG_PTR)PointerPte, 0, 0x20);
        }
    }

    /* Determine if it's a underruns or overruns pool pointer */
    PtrOffset = (ULONG)((ULONG_PTR)P & (PAGE_SIZE - 1));
    if (PtrOffset)
    {
        /* Pool catches overruns */
        Header = PAGE_ALIGN(P);
        Overruns = TRUE;
    }
    else
    {
        /* Pool catches underruns */
        Header = (PPOOL_HEADER)((PUCHAR)PAGE_ALIGN(P) + PAGE_SIZE - sizeof(POOL_HEADER));
    }

    /* Check if it's non paged pool */
    if ((Header->Ulong1 & SPECIAL_POOL_PAGED) == 0)
    {
        /* Non-paged allocation, ensure that IRQ is not higher that DISPATCH */
        ASSERT((PointerPte + 1)->u.Soft.PageFileHigh == SPECIAL_POOL_NONPAGED_PTE);
        if (Irql > DISPATCH_LEVEL)
        {
            KeBugCheckEx(BAD_POOL_HEADER, Irql, (ULONG_PTR)P, 0, 0x31);
        }

        PoolType = NonPagedPool;
    }
    else
    {
        /* Paged allocation, ensure */
        ASSERT((PointerPte + 1)->u.Soft.PageFileHigh == SPECIAL_POOL_PAGED_PTE);
        if (Irql > DISPATCH_LEVEL)
        {
            KeBugCheckEx(BAD_POOL_HEADER, Irql, (ULONG_PTR)P, 1, 0x31);
        }

        PoolType = PagedPool;
    }

    /* Get amount of bytes user requested to be allocated by clearing out the paged mask */
    BytesRequested = (Header->Ulong1 & ~SPECIAL_POOL_PAGED) & 0xFFFF;

    /* Check memory before the allocated user buffer in case of overruns detection */
    if (Overruns)
    {
        /* Calculate the real placement of the buffer */
        BytesReal = PAGE_SIZE - PtrOffset;

        /* If they mismatch, it's unrecoverable */
        if (BytesRequested > BytesReal)
        {
            KeBugCheckEx(BAD_POOL_HEADER, (ULONG_PTR)P, BytesRequested, BytesReal, 0x21);
        }

        if (BytesRequested + sizeof(POOL_HEADER) < BytesReal)
        {
            KeBugCheckEx(BAD_POOL_HEADER, (ULONG_PTR)P, BytesRequested, BytesReal, 0x22);
        }

        /* Actually check the memory pattern */
        for (b = (PUCHAR)(Header + 1); b < (PUCHAR)P; b++)
        {
            if (Header->BlockSize != b[0])
            {
                /* Bytes mismatch */
                KeBugCheckEx(BAD_POOL_HEADER, (ULONG_PTR)P, (ULONG_PTR)b, Header->BlockSize, 0x23);
            }
        }
    }

    /* Check the memory pattern after the user buffer */
    MiSpecialPoolCheckPattern(P, Header);

    /* Fill the freed header */
    KeQueryTickCount(&TickCount);
    FreedHeader = (PMI_FREED_SPECIAL_POOL)PAGE_ALIGN(P);
    FreedHeader->Signature = 0x98764321;
    FreedHeader->TickCount = TickCount.LowPart;
    FreedHeader->NumberOfBytesRequested = BytesRequested;
    FreedHeader->Pagable = PoolType;
    FreedHeader->VirtualAddress = P;
    FreedHeader->Thread = PsGetCurrentThread();
    /* TODO: Fill StackPointer and StackBytes */
    FreedHeader->StackPointer = NULL;
    FreedHeader->StackBytes = 0;

    if (PoolType == NonPagedPool)
    {
        /* Non pagable. Get PFN element corresponding to the PTE */
        Pfn = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);

        /* Lock PFN database */
        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
        Irql = KeAcquireQueuedSpinLock(LockQueuePfnLock);

        /* Delete this PFN */
        MI_SET_PFN_DELETED(Pfn);

        /* Decrement share count of this PFN */
        MiDecrementShareCount(Pfn, PointerPte->u.Hard.PageFrameNumber);

        /* Flush the TLB */
        //FIXME: Use KeFlushSingleTb() instead
        KeFlushEntireTb(TRUE, TRUE);
    }
    else
    {
        /* Pagable. Delete that virtual address */
        MiDeleteSystemPageableVm(PointerPte, 1, 0, NULL);

        /* Lock PFN database */
        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
        Irql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
    }

    /* Mark next PTE as invalid */
    PointerPte[1].u.Long = 0; //|= 8000;

    /* Make sure that the last entry is really the last one */
    ASSERT(MiSpecialPoolLastPte->u.List.NextEntry == MM_EMPTY_PTE_LIST);

    /* Update the current last PTE next pointer */
    MiSpecialPoolLastPte->u.List.NextEntry = PointerPte - MmSystemPteBase;

    /* PointerPte becomes the new last PTE */
    PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;
    MiSpecialPoolLastPte = PointerPte;

    /* Release the PFN database lock */
    KeReleaseQueuedSpinLock(LockQueuePfnLock, Irql);
}

VOID
NTAPI
MiTestSpecialPool()
{
    ULONG i;
    PVOID p1, p2[100];
    //PUCHAR p3;
    ULONG ByteSize;
    POOL_TYPE PoolType = PagedPool;

    // First allocate/free
    for (i=0; i<100; i++)
    {
        ByteSize = (100 * (i+1)) % (PAGE_SIZE - sizeof(POOL_HEADER));
        p1 = MmAllocateSpecialPool(ByteSize, 'TEST', PoolType, 0);
        DPRINT1("p1 %p size %d\n", p1, ByteSize);
        MmFreeSpecialPool(p1);
    }

    // Now allocate all at once, then free at once
    for (i=0; i<100; i++)
    {
        ByteSize = (100 * (i+1)) % (PAGE_SIZE - sizeof(POOL_HEADER));
        p2[i] = MmAllocateSpecialPool(ByteSize, 'TEST', PoolType, 0);
        DPRINT1("p2[%d] %p size %d\n", i, p1, ByteSize);
    }
    for (i=0; i<100; i++)
    {
        DPRINT1("Freeing %p\n", p2[i]);
        MmFreeSpecialPool(p2[i]);
    }

    // Overrun the buffer to test
    //ByteSize = 16;
    //p3 = MmAllocateSpecialPool(ByteSize, 'TEST', NonPagedPool, 0);
    //p3[ByteSize] = 0xF1; // This should cause an exception

    // Underrun the buffer to test
    //p3 = MmAllocateSpecialPool(ByteSize, 'TEST', NonPagedPool, 1);
    //p3--;
    //*p3 = 0xF1; // This should cause an exception

}

/* EOF */
