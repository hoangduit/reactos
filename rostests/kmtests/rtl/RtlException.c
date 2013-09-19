/*
 * PROJECT:         ReactOS kernel-mode tests
 * LICENSE:         GPLv2+ - See COPYING in the top level directory
 * PURPOSE:         Kernel-Mode Test Suite Exception test
 * PROGRAMMER:      Thomas Faber <thfabba@gmx.de>
 */

#include <kmt_test.h>

START_TEST(RtlException)
{
    NTSTATUS ExceptionStatus;
    PCHAR Buffer[128];

    /* Access a valid pointer - must not trigger SEH */
    KmtStartSeh()
        RtlFillMemory(Buffer, sizeof(Buffer), 0x12);
    KmtEndSeh(STATUS_SUCCESS);

    /* Read from a NULL pointer - must cause an access violation */
    KmtStartSeh()
        (void)*(volatile CHAR *)NULL;
    KmtEndSeh(STATUS_ACCESS_VIOLATION);

    /* Write to a NULL pointer - must cause an access violation */
    KmtStartSeh()
        *(volatile CHAR *)NULL = 5;
    KmtEndSeh(STATUS_ACCESS_VIOLATION);

    /* TODO: Find where MmBadPointer is defined - gives an unresolved external */
#if 0 //def KMT_KERNEL_MODE
    /* Read from MmBadPointer - must cause an access violation */
    KmtStartSeh()
        (void)*(volatile CHAR *)MmBadPointer;
    KmtEndSeh(STATUS_ACCESS_VIOLATION);

    /* Write to MmBadPointer - must cause an access violation */
    KmtStartSeh()
        *(volatile CHAR *)MmBadPointer = 5;
    KmtEndSeh(STATUS_ACCESS_VIOLATION);
#endif

    /* We cannot test this in kernel mode easily - the stack is just "somewhere"
     * in system space, and there's no guard page below it */
#if CORE_6640_IS_FIXED
#ifdef KMT_USER_MODE
    /* Overflow the stack - must cause a special exception */
    KmtStartSeh()
        PCHAR Pointer;

        while (1)
        {
            Pointer = _alloca(1024);
            *Pointer = 5;
        }
    KmtEndSeh(STATUS_STACK_OVERFLOW);
#endif
#endif /* CORE_6640_IS_FIXED */
}
