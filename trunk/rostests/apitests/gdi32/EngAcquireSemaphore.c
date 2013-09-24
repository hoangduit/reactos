/*
 * PROJECT:         ReactOS api tests
 * LICENSE:         GPL - See COPYING in the top level directory
 * PURPOSE:         Test for EngAcquireSemaphore
 * PROGRAMMERS:     Magnus Olsen
 */

#include <apitest.h>

#include <wingdi.h>
#include <winddi.h>

void Test_EngAcquireSemaphore()
{
    HSEMAPHORE hsem;
    PRTL_CRITICAL_SECTION lpcrit;

    hsem = EngCreateSemaphore();
    ok(hsem != NULL, "EngCreateSemaphore failed\n");
    if (!hsem) return;
    lpcrit = (PRTL_CRITICAL_SECTION)hsem;

    /* real data test */
    EngAcquireSemaphore(hsem);
//    ok(lpcrit->LockCount == -2); doesn't work on XP
    ok(lpcrit->RecursionCount == 1, "lpcrit->RecursionCount=%ld\n", lpcrit->RecursionCount);
    ok(lpcrit->OwningThread != 0, "lpcrit->OwningThread=%p\n", lpcrit->OwningThread);
    ok(lpcrit->LockSemaphore == 0, "lpcrit->LockSemaphore=%p\n", lpcrit->LockSemaphore);
    ok(lpcrit->SpinCount == 0, "lpcrit->SpinCount=%ld\n", lpcrit->SpinCount);

    ok(lpcrit->DebugInfo != NULL, "no DebugInfo\n");
    if (lpcrit->DebugInfo)
    {
        ok(lpcrit->DebugInfo->Type == 0, "DebugInfo->Type=%d\n", lpcrit->DebugInfo->Type);
        ok(lpcrit->DebugInfo->CreatorBackTraceIndex == 0, "DebugInfo->CreatorBackTraceIndex=%d\n", lpcrit->DebugInfo->CreatorBackTraceIndex);
        ok(lpcrit->DebugInfo->EntryCount == 0, "DebugInfo->EntryCount=%ld\n", lpcrit->DebugInfo->EntryCount);
        ok(lpcrit->DebugInfo->ContentionCount == 0, "DebugInfo->ContentionCount=%ld\n", lpcrit->DebugInfo->ContentionCount);
    }

    EngReleaseSemaphore(hsem);
    EngDeleteSemaphore(hsem);

    /* NULL pointer test */
    // Note NULL pointer test crash in Vista */
    // EngAcquireSemaphore(NULL);

    /* negtive pointer test */
    // Note negtive pointer test crash in Vista */
    // EngAcquireSemaphore((HSEMAPHORE)-1);

    /* try with deleted Semaphore */
    // Note deleted Semaphore pointer test does freze the whole program in Vista */
    // EngAcquireSemaphore(hsem);
}

START_TEST(EngAcquireSemaphore)
{
    Test_EngAcquireSemaphore();
}

