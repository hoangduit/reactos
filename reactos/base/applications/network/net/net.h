/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS net command 
 * PROGRAMMERS:     Magnus Olsen (greatlord@reactos.org)
 */

#ifndef _NET_PCH_
#define _NET_PCH_

#define WIN32_NO_STATUS

#include <stdarg.h>

#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <winsvc.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <lm.h>
#include <ndk/rtlfuncs.h>

#include "resource.h"

VOID
PrintResourceString(
    INT resID,
    ...);

VOID help(VOID);
INT unimplemented(INT argc, WCHAR **argv);

INT cmdAccounts(INT argc, WCHAR **argv);
INT cmdContinue(INT argc, WCHAR **argv);
INT cmdHelp(INT argc, WCHAR **argv);
INT cmdHelpMsg(INT argc, WCHAR **argv);
INT cmdPause(INT argc, WCHAR **argv);
INT cmdStart(INT argc, WCHAR **argv);
INT cmdStop(INT argc, WCHAR **argv);

#endif /* _NET_PCH_ */
