/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            lib/rossym/data.c
 * PURPOSE:         Definition of external variables
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@reactos.com)
 */

#include <precomp.h>

ROSSYM_CALLBACKS RosSymCallbacks;

VOID
RosSymInit(PROSSYM_CALLBACKS Callbacks)
{
  RosSymCallbacks = *Callbacks;
}

/* EOF */
