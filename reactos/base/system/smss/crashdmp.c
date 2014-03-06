/*
 * PROJECT:         ReactOS Windows-Compatible Session Manager
 * LICENSE:         BSD 2-Clause License
 * FILE:            base/system/smss/smss.c
 * PURPOSE:         Main SMSS Code
 * PROGRAMMERS:     Alex Ionescu
 */

/* INCLUDES *******************************************************************/

#include "smss.h"
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

/* FUNCTIONS ******************************************************************/

BOOLEAN
NTAPI
SmpCheckForCrashDump(IN PUNICODE_STRING FileName)
{
    return FALSE;
}

     