/*
 *  ReactOS kernel
 *  Copyright (C) 1998, 1999, 2000, 2001 ReactOS Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * PROJECT:         ReactOS user32.dll
 * FILE:            lib/user32/misc/misc.c
 * PURPOSE:         Misc
 * PROGRAMMER:      Thomas Weidenmueller (w3seek@users.sourceforge.net)
 * UPDATE HISTORY:
 *      19-11-2003  Created
 */

/* INCLUDES ******************************************************************/

#include <user32.h>

#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(user32);

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
BOOL
WINAPI
RegisterLogonProcess(DWORD dwProcessId, BOOL bRegister)
{
    gfLogonProcess = NtUserxRegisterLogonProcess(dwProcessId, bRegister);

    if (gfLogonProcess)
    {
        NTSTATUS Status;
        USER_API_MESSAGE ApiMessage;

        ApiMessage.Data.RegisterLogonProcessRequest.ProcessId = dwProcessId;
        ApiMessage.Data.RegisterLogonProcessRequest.Register = bRegister;

        Status = CsrClientCallServer((PCSR_API_MESSAGE)&ApiMessage,
                                     NULL,
                                     CSR_CREATE_API_NUMBER(USERSRV_SERVERDLL_INDEX, UserpRegisterLogonProcess),
                                     sizeof(USER_REGISTER_LOGON_PROCESS));
        if (!NT_SUCCESS(Status))
        {
            ERR("Failed to register logon process with CSRSS\n");
            SetLastError(RtlNtStatusToDosError(Status));
            // return FALSE;
        }
    }

    return gfLogonProcess;
}

/*
 * @implemented
 */
BOOL
WINAPI
SetLogonNotifyWindow(HWND Wnd, HWINSTA WinSta)
{
    return NtUserSetLogonNotifyWindow(Wnd);
}

/*
 * @implemented
 */
BOOL WINAPI
UpdatePerUserSystemParameters(
    DWORD dwReserved,
    BOOL bEnable)
{
    return NtUserUpdatePerUserSystemParameters(dwReserved, bEnable);
}

PTHREADINFO
GetW32ThreadInfo(VOID)
{
    PTHREADINFO ti;

    ti = (PTHREADINFO)NtCurrentTeb()->Win32ThreadInfo;
    if (ti == NULL)
    {
        /* create the THREADINFO structure */
        NtUserGetThreadState(THREADSTATE_GETTHREADINFO);
        ti = (PTHREADINFO)NtCurrentTeb()->Win32ThreadInfo;
    }

    return ti;
}


/*
 * GetUserObjectSecurity
 *
 * Retrieves security information for user object specified
 * with handle 'hObject'. Descriptor returned in self-relative
 * format.
 *
 * Arguments:
 *  1) hObject - handle to an object to retrieve information for
 *  2) pSecurityInfo - type of information to retrieve
 *  3) pSecurityDescriptor - buffer which receives descriptor
 *  4) dwLength - size, in bytes, of buffer 'pSecurityDescriptor'
 *  5) pdwLengthNeeded - reseives actual size of descriptor
 *
 * Return Vaules:
 *  TRUE on success
 *  FALSE on failure, call GetLastError() for more information
 */
/*
 * @implemented
 */
BOOL
WINAPI
GetUserObjectSecurity(
    IN HANDLE hObject,
    IN PSECURITY_INFORMATION pSecurityInfo,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN DWORD dwLength,
    OUT PDWORD pdwLengthNeeded
)
{
DWORD dwWin32Error;
NTSTATUS Status;


    Status = NtQuerySecurityObject(
        hObject,            // Object Handle
        *pSecurityInfo,     // Security Information
        pSecurityDescriptor,// Security Descriptor
        dwLength,           // Buffer Length
        pdwLengthNeeded     // Actual Length
    );

    if ( ! NT_SUCCESS( Status ) ) {
        dwWin32Error = RtlNtStatusToDosError( Status );
        NtCurrentTeb()->LastErrorValue = dwWin32Error;
        return FALSE;
    }

    return TRUE;
}


/*
 * SetUserObjectSecurity
 *
 * Sets new security descriptor to user object specified by
 * handle 'hObject'. Descriptor must be in self-relative format.
 *
 * Arguments:
 *  1) hObject - handle to an object to set information for
 *  2) pSecurityInfo - type of information to apply
 *  3) pSecurityDescriptor - buffer which descriptor to set
 *
 * Return Vaules:
 *  TRUE on success
 *  FALSE on failure, call GetLastError() for more information
 */
/*
 * @implemented
 */
BOOL
WINAPI
SetUserObjectSecurity(
    IN HANDLE hObject,
    IN PSECURITY_INFORMATION pSecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
)
{
DWORD dwWin32Error;
NTSTATUS Status;


    Status = NtSetSecurityObject(
        hObject,            // Object Handle
        *pSecurityInfo,     // Security Information
        pSecurityDescriptor // Security Descriptor
    );

    if ( ! NT_SUCCESS( Status ) ) {
        dwWin32Error = RtlNtStatusToDosError( Status );
        NtCurrentTeb()->LastErrorValue = dwWin32Error;
        return FALSE;
    }

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
EndTask(
	HWND    hWnd,
	BOOL fShutDown,
	BOOL fForce)
{
    SendMessageW(hWnd, WM_CLOSE, 0, 0);

    if (IsWindow(hWnd))
    {
        if (fForce)
            return DestroyWindow(hWnd);
        else
            return FALSE;
    }

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
IsGUIThread(
    BOOL bConvert)
{
  PTHREADINFO ti = (PTHREADINFO)NtCurrentTeb()->Win32ThreadInfo;
  if (ti == NULL)
  {
    if(bConvert)
    {
      NtUserGetThreadState(THREADSTATE_GETTHREADINFO);
      if ((PTHREADINFO)NtCurrentTeb()->Win32ThreadInfo) return TRUE;
      else
         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return FALSE;
  }
  else
    return TRUE;
}

BOOL
FASTCALL
TestWindowProcess(PWND Wnd)
{
   if (Wnd->head.pti == (PTHREADINFO)NtCurrentTeb()->Win32ThreadInfo)
      return TRUE;
   else
      return (NtUserQueryWindow(Wnd->head.h, QUERY_WINDOW_UNIQUE_PROCESS_ID) ==
              (DWORD_PTR)NtCurrentTeb()->ClientId.UniqueProcess );
}

BOOL
FASTCALL
TestState(PWND pWnd, UINT Flag)
{
    UINT bit;
    bit = 1 << LOWORD(Flag);
    switch(HIWORD(Flag))
    {
       case 0: 
          return (pWnd->state & bit); 
       case 1:
          return (pWnd->state2 & bit);
       case 2:
          return (pWnd->ExStyle2 & bit);
    }
    return FALSE;
}

PUSER_HANDLE_ENTRY
FASTCALL
GetUser32Handle(HANDLE handle)
{
    INT Index;
    USHORT generation;

    if (!handle) return NULL;

    Index = (((UINT_PTR)handle & 0xffff) - FIRST_USER_HANDLE) >> 1;

    if (Index < 0 || Index >= gHandleTable->nb_handles)
        return NULL;

    if (!gHandleEntries[Index].type || !gHandleEntries[Index].ptr)
        return NULL;

    generation = (UINT_PTR)handle >> 16;

    if (generation == gHandleEntries[Index].generation || !generation || generation == 0xffff)
        return &gHandleEntries[Index];

    return NULL;
}

/*
 * Decide whether an object is located on the desktop or shared heap
 */
static const BOOL g_ObjectHeapTypeShared[TYPE_CTYPES] =
{
    FALSE, /* TYPE_FREE (not used) */
    FALSE, /* TYPE_WINDOW */
    TRUE,  /* TYPE_MENU  FALSE */
    TRUE,  /* TYPE_CURSOR */
    TRUE,  /* TYPE_SETWINDOWPOS */
    FALSE, /* TYPE_HOOK */
    FALSE, /* TYPE_CLIPDATA */
    FALSE, /* TYPE_CALLPROC */
    TRUE,  /* TYPE_ACCELTABLE */
    FALSE, /* TYPE_DDEACCESS */
    FALSE, /* TYPE_DDECONV */
    FALSE, /* TYPE_DDEXACT */
    TRUE,  /* TYPE_MONITOR */
    FALSE, /* TYPE_KBDLAYOUT */
    FALSE, /* TYPE_KBDFILE */
    TRUE,   /* TYPE_WINEVENTHOOK */
    FALSE, /* TYPE_TIMER */
    FALSE, /* TYPE_INPUTCONTEXT */
    FALSE, /* TYPE_HIDDATA */
    FALSE, /* TYPE_DEVICEINFO */
    FALSE, /* TYPE_TOUCHINPUTINFO */
    FALSE, /* TYPE_GESTUREINFOOBJ */
};

//
// Validate Handle and return the pointer to the object.
//
PVOID
FASTCALL
ValidateHandle(HANDLE handle, UINT uType)
{
  PVOID ret;
  PUSER_HANDLE_ENTRY pEntry;

  ASSERT(uType < TYPE_CTYPES);

  pEntry = GetUser32Handle(handle);

  if (pEntry && uType == 0)
      uType = pEntry->type;

// Must have an entry and must be the same type!
  if ( (!pEntry) ||
        (pEntry->type != uType) ||
        !pEntry->ptr ||
        (pEntry->flags & HANDLEENTRY_INDESTROY) )
  {
     switch ( uType )
     {  // Test (with wine too) confirms these results!
        case TYPE_WINDOW:
          SetLastError(ERROR_INVALID_WINDOW_HANDLE);
          break;
        case TYPE_MENU:
          SetLastError(ERROR_INVALID_MENU_HANDLE);
          break;
        case TYPE_CURSOR:
          SetLastError(ERROR_INVALID_CURSOR_HANDLE);
          break;
        case TYPE_SETWINDOWPOS:
          SetLastError(ERROR_INVALID_DWP_HANDLE);
          break;
        case TYPE_HOOK:
          SetLastError(ERROR_INVALID_HOOK_HANDLE);
          break;
        case TYPE_ACCELTABLE:
          SetLastError(ERROR_INVALID_ACCEL_HANDLE);
          break;
        default:
          SetLastError(ERROR_INVALID_HANDLE);
          break;
    }
    return NULL;
  }

  if (g_ObjectHeapTypeShared[uType])
    ret = SharedPtrToUser(pEntry->ptr);
  else
    ret = DesktopPtrToUser(pEntry->ptr);

  return ret;
}

//
// Validate Handle and return the pointer to the object.
//
PVOID
FASTCALL
ValidateHandleNoErr(HANDLE handle, UINT uType)
{
  PVOID ret;
  PUSER_HANDLE_ENTRY pEntry;

  ASSERT(uType < TYPE_CTYPES);

  pEntry = GetUser32Handle(handle);

  if (pEntry && uType == 0)
      uType = pEntry->type;

// Must have an entry and must be the same type!
  if ( (!pEntry) || (pEntry->type != uType) || !pEntry->ptr )
    return NULL;

  if (g_ObjectHeapTypeShared[uType])
    ret = SharedPtrToUser(pEntry->ptr);
  else
    ret = DesktopPtrToUser(pEntry->ptr);

  return ret;
}

//
// Validate a callproc handle and return the pointer to the object.
//
PCALLPROCDATA
FASTCALL
ValidateCallProc(HANDLE hCallProc)
{
  PUSER_HANDLE_ENTRY pEntry;

  PCALLPROCDATA CallProc = ValidateHandle(hCallProc, TYPE_CALLPROC);

  pEntry = GetUser32Handle(hCallProc);

  if (CallProc != NULL && pEntry->ppi == g_ppi)
     return CallProc;

  return NULL;
}


//
// Validate a window handle and return the pointer to the object.
//
PWND
FASTCALL
ValidateHwnd(HWND hwnd)
{
    PCLIENTINFO ClientInfo = GetWin32ClientInfo();
    ASSERT(ClientInfo != NULL);

    /* See if the window is cached */
    if (hwnd && hwnd == ClientInfo->CallbackWnd.hWnd)
        return ClientInfo->CallbackWnd.pWnd;

    return ValidateHandle((HANDLE)hwnd, TYPE_WINDOW);
}

//
// Validate a window handle and return the pointer to the object.
//
PWND
FASTCALL
ValidateHwndNoErr(HWND hwnd)
{
    PWND Wnd;
    PCLIENTINFO ClientInfo = GetWin32ClientInfo();
    ASSERT(ClientInfo != NULL);

    /* See if the window is cached */
    if (hwnd == ClientInfo->CallbackWnd.hWnd)
        return ClientInfo->CallbackWnd.pWnd;

    Wnd = ValidateHandleNoErr((HANDLE)hwnd, TYPE_WINDOW);
    if (Wnd != NULL)
    {
        return Wnd;
    }

    return NULL;
}

PWND
FASTCALL
GetThreadDesktopWnd(VOID)
{
    PWND Wnd = GetThreadDesktopInfo()->spwnd;
    if (Wnd != NULL)
        Wnd = DesktopPtrToUser(Wnd);
    return Wnd;
}

//
// Validate a window handle and return the pointer to the object.
//
PWND
FASTCALL
ValidateHwndOrDesk(HWND hwnd)
{
    if (hwnd == HWND_DESKTOP)
        return GetThreadDesktopWnd();

    return ValidateHwnd(hwnd);
}

/*
 * @implemented
 */
DWORD WINAPI WCSToMBEx(WORD CodePage,LPWSTR UnicodeString,LONG UnicodeSize,LPSTR *MBString,LONG MBSize,BOOL Allocate)
{
	DWORD Size;
	if (UnicodeSize == -1)
	{
		UnicodeSize = wcslen(UnicodeString)+1;
	}
	if (MBSize == -1)
	{
		if (!Allocate)
		{
			return 0;
		}
		MBSize = UnicodeSize * 2;
	}
	if (Allocate)
	{
		LPSTR SafeString = RtlAllocateHeap(GetProcessHeap(), 0, MBSize);
        if (SafeString == NULL)
            return 0;
        *MBString = SafeString;
	}
	if (CodePage == 0)
	{
		RtlUnicodeToMultiByteN(*MBString,MBSize,&Size,UnicodeString,UnicodeSize);
	}
	else
	{
		WideCharToMultiByte(CodePage,0,UnicodeString,UnicodeSize,*MBString,MBSize,0,0);
	}
	return UnicodeSize;
}

/*
 * @implemented
 */
DWORD WINAPI MBToWCSEx(WORD CodePage,LPSTR MBString,LONG MBSize,LPWSTR *UnicodeString,LONG UnicodeSize,BOOL Allocate)
{
	DWORD Size;
	if (MBSize == -1)
	{
		MBSize = strlen(MBString)+1;
	}
	if (UnicodeSize == -1)
	{
		if (!Allocate)
		{
			return 0;
		}
		UnicodeSize = MBSize;
	}
	if (Allocate)
	{
		LPWSTR SafeString = RtlAllocateHeap(GetProcessHeap(), 0, UnicodeSize);
        if (SafeString == NULL)
            return 0;
        *UnicodeString = SafeString;
	}
	UnicodeSize *= sizeof(WCHAR);
	if (CodePage == 0)
	{
		RtlMultiByteToUnicodeN(*UnicodeString,UnicodeSize,&Size,MBString,MBSize);
	}
	else
	{
		Size = MultiByteToWideChar(CodePage,0,MBString,MBSize,*UnicodeString,UnicodeSize);
	}
	return Size;
}
