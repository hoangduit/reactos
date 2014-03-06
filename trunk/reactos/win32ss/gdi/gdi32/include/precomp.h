/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS System Libraries
 * FILE:            lib/gdi32/include/precomp.h
 * PURPOSE:         User-Mode Win32 GDI Library Header
 * PROGRAMMER:      Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/

/* Definitions */
#define WIN32_NO_STATUS
#define _INC_WINDOWS
#define COM_NO_WINDOWS_H
#define NTOS_MODE_USER

#include <stdio.h>

/* SDK/DDK/NDK Headers. */
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <objbase.h>
#include <ndk/rtlfuncs.h>
#include <wingdi.h>
#define _ENGINE_EXPORT_
#include <winddi.h>
#include <d3dnthal.h>
#include <prntfont.h>
#include <winddiui.h>
#include <winspool.h>

#include <pseh/pseh2.h>

#include <ddraw.h>
#include <ddrawi.h>
#include <ddrawint.h>
#include <ddrawgdi.h>
#include <d3dhal.h>

/* Public Win32K Headers */
#include <ntgdityp.h>
#include <ntgdi.h>
#include <ntgdihdl.h>

/* Private GDI32 Header */
#include "gdi32p.h"

/* Deprecated NTGDI calls which shouldn't exist */
#include <ntgdibad.h>

/* EOF */
