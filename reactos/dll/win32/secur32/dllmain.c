/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS system libraries
 * FILE:            lib/secur32/lsa.c
 * PURPOSE:         Client-side LSA functions
 * UPDATE HISTORY:
 *                  Created 05/08/00
 */

#include "precomp.h"

/* GLOBALS *******************************************************************/

HANDLE Secur32Heap;

/* FUNCTIONS *****************************************************************/

BOOL
WINAPI
DllMain(HINSTANCE hInstance,
        ULONG Reason,
        PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            Secur32Heap = RtlCreateHeap(0, NULL, 0, 4096, NULL, NULL);
            if (Secur32Heap == 0)
            {
                return FALSE;
            }
            break;

        case DLL_PROCESS_DETACH:
            if (!RtlDestroyHeap(Secur32Heap))
            {
                return FALSE;
            }
            break;
    }

    return TRUE;
}
