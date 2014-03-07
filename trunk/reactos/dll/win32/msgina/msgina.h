#ifndef _MSGINA_H
#define _MSGINA_H

#include <stdarg.h>

#define WIN32_NO_STATUS
#define _INC_WINDOWS
#define COM_NO_WINDOWS_H

#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <winwlx.h>
#include <ndk/rtlfuncs.h>
#include <ntsecapi.h>

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(msgina);

#include "resource.h"

/* Values for GINA_CONTEXT.AutoLogonState */
#define AUTOLOGON_CHECK_REGISTRY 1
#define AUTOLOGON_ONCE           2
#define AUTOLOGON_DISABLED       3

typedef struct
{
    HANDLE hWlx;
    LPWSTR station;
    PWLX_DISPATCH_VERSION_1_3 pWlxFuncs;
    HANDLE hDllInstance;
    HWND hStatusWindow;
    HANDLE LsaHandle;
    ULONG AuthenticationPackage;
    DWORD AutoLogonState;
    BOOL bDisableCAD;
    BOOL bAutoAdminLogon;
    BOOL bDontDisplayLastUserName;
    BOOL bShutdownWithoutLogon;

    INT nShutdownAction;

    /* Information to be filled during logon */
    WCHAR UserName[256];
    WCHAR Domain[256];
    WCHAR Password[256];
    SYSTEMTIME LogonTime;
    HANDLE UserToken;
    PLUID pAuthenticationId;
    PDWORD pdwOptions;
    PWLX_MPR_NOTIFY_INFO pMprNotifyInfo;
    PVOID *pProfile;

    /* Current logo to display */
    HBITMAP hBitmap;
} GINA_CONTEXT, *PGINA_CONTEXT;

extern HINSTANCE hDllInstance;

typedef BOOL (*PFGINA_INITIALIZE)(PGINA_CONTEXT);
typedef BOOL (*PFGINA_DISPLAYSTATUSMESSAGE)(PGINA_CONTEXT, HDESK, DWORD, PWSTR, PWSTR);
typedef BOOL (*PFGINA_REMOVESTATUSMESSAGE)(PGINA_CONTEXT);
typedef VOID (*PFGINA_DISPLAYSASNOTICE)(PGINA_CONTEXT);
typedef INT (*PFGINA_LOGGEDONSAS)(PGINA_CONTEXT, DWORD);
typedef INT (*PFGINA_LOGGEDOUTSAS)(PGINA_CONTEXT);
typedef INT (*PFGINA_LOCKEDSAS)(PGINA_CONTEXT);
typedef VOID (*PFGINA_DISPLAYLOCKEDNOTICE)(PGINA_CONTEXT);

typedef struct _GINA_UI
{
    PFGINA_INITIALIZE Initialize;
    PFGINA_DISPLAYSTATUSMESSAGE DisplayStatusMessage;
    PFGINA_REMOVESTATUSMESSAGE RemoveStatusMessage;
    PFGINA_DISPLAYSASNOTICE DisplaySASNotice;
    PFGINA_LOGGEDONSAS LoggedOnSAS;
    PFGINA_LOGGEDOUTSAS LoggedOutSAS;
    PFGINA_LOCKEDSAS LockedSAS;
    PFGINA_DISPLAYLOCKEDNOTICE DisplayLockedNotice;
} GINA_UI, *PGINA_UI;

/* lsa.c */

NTSTATUS
ConnectToLsa(
    PGINA_CONTEXT pgContext);

NTSTATUS
MyLogonUser(
    HANDLE LsaHandle,
    ULONG AuthenticationPackage,
    LPWSTR lpszUsername,
    LPWSTR lpszDomain,
    LPWSTR lpszPassword,
    PHANDLE phToken,
    PNTSTATUS SubStatus);

/* msgina.c */

BOOL
DoAdminUnlock(
    IN PGINA_CONTEXT pgContext,
    IN PWSTR UserName,
    IN PWSTR Domain,
    IN PWSTR Password);

BOOL
DoLoginTasks(
    IN OUT PGINA_CONTEXT pgContext,
    IN PWSTR UserName,
    IN PWSTR Domain,
    IN PWSTR Password);

#endif /* _MSGINA_H */
