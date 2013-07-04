/*
 * COPYRIGHT:       GNU GPL, see COPYING in the top level directory
 * PROJECT:         ReactOS crt library
 * FILE:            lib/sdk/crt/printf/vfprintf.c
 * PURPOSE:         Implementation of vfprintf
 * PROGRAMMER:      Timo Kreuzer
 */

#include <stdio.h>
#include <stdarg.h>

int _cdecl streamout(FILE *stream, const char *format, va_list argptr);

int
_cdecl
vfprintf(FILE *file, const char *format, va_list argptr)
{
    int result;

    _lock_file(file);
    result = streamout(file, format, argptr);
    _unlock_file(file);

    return result;
}

