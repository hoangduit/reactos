/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS net command 
 * PROGRAMMERS:     Magnus Olsen (greatlord@reactos.org)
 */

#ifndef _NET_PCH_
#define _NET_PCH_

#include <stdarg.h>

#include <windef.h>
#include <winbase.h>
#include <winsvc.h>
#include <stdio.h>

VOID help(VOID);
INT unimplemented(INT argc, WCHAR **argv);

INT cmdContinue(INT argc, WCHAR **argv);
INT cmdHelp(INT argc, WCHAR **argv);
INT cmdHelpMsg(INT argc, WCHAR **argv);
INT cmdPause(INT argc, WCHAR **argv);
INT cmdStart(INT argc, WCHAR **argv);
INT cmdStop(INT argc, WCHAR **argv);

#endif /* _NET_PCH_ */
