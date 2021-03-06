/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS Win32k subsystem
 * PURPOSE:          Caret functions
 * FILE:             subsystems/win32/win32k/ntuser/caret.c
 * PROGRAMER:        Thomas Weidenmueller (w3seek@users.sourceforge.net)
 */

#include <win32k.h>
DBG_DEFAULT_CHANNEL(UserCaret);

/* DEFINES *****************************************************************/

#define MIN_CARETBLINKRATE 100
#define MAX_CARETBLINKRATE 10000

/* FUNCTIONS *****************************************************************/

static
BOOL FASTCALL
co_IntHideCaret(PTHRDCARETINFO CaretInfo)
{
   PWND pWnd;
   if(CaretInfo->hWnd && CaretInfo->Visible && CaretInfo->Showing)
   {
      pWnd = UserGetWindowObject(CaretInfo->hWnd);
      co_IntSendMessage(CaretInfo->hWnd, WM_SYSTIMER, IDCARETTIMER, 0);
      CaretInfo->Showing = 0;
      IntNotifyWinEvent(EVENT_OBJECT_HIDE, pWnd, OBJID_CARET, CHILDID_SELF, 0);
      return TRUE;
   }
   return FALSE;
}

BOOL FASTCALL
co_IntDestroyCaret(PTHREADINFO Win32Thread)
{
   PUSER_MESSAGE_QUEUE ThreadQueue;
   PWND pWnd;
   ThreadQueue = (PUSER_MESSAGE_QUEUE)Win32Thread->MessageQueue;

   if(!ThreadQueue || !ThreadQueue->CaretInfo)
      return FALSE;

   pWnd = ValidateHwndNoErr(ThreadQueue->CaretInfo->hWnd);
   co_IntHideCaret(ThreadQueue->CaretInfo);
   ThreadQueue->CaretInfo->Bitmap = (HBITMAP)0;
   ThreadQueue->CaretInfo->hWnd = (HWND)0;
   ThreadQueue->CaretInfo->Size.cx = ThreadQueue->CaretInfo->Size.cy = 0;
   ThreadQueue->CaretInfo->Showing = 0;
   ThreadQueue->CaretInfo->Visible = 0;
   if (pWnd)
   {
      IntNotifyWinEvent(EVENT_OBJECT_DESTROY, pWnd, OBJID_CARET, CHILDID_SELF, 0);
   }
   return TRUE;
}

BOOL FASTCALL
IntSetCaretBlinkTime(UINT uMSeconds)
{
   /* Don't save the new value to the registry! */

   /* Windows doesn't do this check */
   if((uMSeconds < MIN_CARETBLINKRATE) || (uMSeconds > MAX_CARETBLINKRATE))
   {
      EngSetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }

   gpsi->dtCaretBlink = uMSeconds;

   return TRUE;
}

BOOL FASTCALL
co_IntSetCaretPos(int X, int Y)
{
   PTHREADINFO pti;
   PWND pWnd;
   PUSER_MESSAGE_QUEUE ThreadQueue;

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   if(ThreadQueue->CaretInfo->hWnd)
   {
      pWnd = UserGetWindowObject(ThreadQueue->CaretInfo->hWnd);
      if(ThreadQueue->CaretInfo->Pos.x != X || ThreadQueue->CaretInfo->Pos.y != Y)
      {
         co_IntHideCaret(ThreadQueue->CaretInfo);
         ThreadQueue->CaretInfo->Showing = 0;
         ThreadQueue->CaretInfo->Pos.x = X;
         ThreadQueue->CaretInfo->Pos.y = Y;
         co_IntSendMessage(ThreadQueue->CaretInfo->hWnd, WM_SYSTIMER, IDCARETTIMER, 0);
         IntSetTimer(pWnd, IDCARETTIMER, gpsi->dtCaretBlink, NULL, TMRF_SYSTEM);
         IntNotifyWinEvent(EVENT_OBJECT_LOCATIONCHANGE, pWnd, OBJID_CARET, CHILDID_SELF, 0);
      }
      return TRUE;
   }

   return FALSE;
}

BOOL FASTCALL
IntSwitchCaretShowing(PVOID Info)
{
   PTHREADINFO pti;
   PUSER_MESSAGE_QUEUE ThreadQueue;

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   if(ThreadQueue->CaretInfo->hWnd)
   {
      ThreadQueue->CaretInfo->Showing = (ThreadQueue->CaretInfo->Showing ? 0 : 1);
      MmCopyToCaller(Info, ThreadQueue->CaretInfo, sizeof(THRDCARETINFO));
      return TRUE;
   }

   return FALSE;
}

#if 0 // Unused
static
VOID FASTCALL
co_IntDrawCaret(HWND hWnd)
{
   PTHREADINFO pti;
   PUSER_MESSAGE_QUEUE ThreadQueue;

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   if(ThreadQueue->CaretInfo->hWnd && ThreadQueue->CaretInfo->Visible &&
         ThreadQueue->CaretInfo->Showing)
   {
      co_IntSendMessage(ThreadQueue->CaretInfo->hWnd, WM_SYSTIMER, IDCARETTIMER, 0);
      ThreadQueue->CaretInfo->Showing = 1;
   }
}
#endif



BOOL FASTCALL co_UserHideCaret(PWND Window OPTIONAL)
{
   PTHREADINFO pti;
   PUSER_MESSAGE_QUEUE ThreadQueue;

   if (Window) ASSERT_REFS_CO(Window);

   if(Window && Window->head.pti->pEThread != PsGetCurrentThread())
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   if(Window && ThreadQueue->CaretInfo->hWnd != Window->head.h)
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   if(ThreadQueue->CaretInfo->Visible)
   {
      PWND pwnd = UserGetWindowObject(ThreadQueue->CaretInfo->hWnd);
      IntKillTimer(pwnd, IDCARETTIMER, TRUE);

      co_IntHideCaret(ThreadQueue->CaretInfo);
      ThreadQueue->CaretInfo->Visible = 0;
      ThreadQueue->CaretInfo->Showing = 0;
   }

   return TRUE;
}


BOOL FASTCALL co_UserShowCaret(PWND Window OPTIONAL)
{
   PTHREADINFO pti;
   PUSER_MESSAGE_QUEUE ThreadQueue;
   PWND pWnd = NULL;

   if (Window) ASSERT_REFS_CO(Window);

   if(Window && Window->head.pti->pEThread != PsGetCurrentThread())
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   if(Window && ThreadQueue->CaretInfo->hWnd != Window->head.h)
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   if (!ThreadQueue->CaretInfo->Visible)
   {
      ThreadQueue->CaretInfo->Visible = 1;
      pWnd = ValidateHwndNoErr(ThreadQueue->CaretInfo->hWnd);
      if (!ThreadQueue->CaretInfo->Showing && pWnd)
      {
         co_IntSendMessage(ThreadQueue->CaretInfo->hWnd, WM_SYSTIMER, IDCARETTIMER, 0);
         IntNotifyWinEvent(EVENT_OBJECT_SHOW, pWnd, OBJID_CARET, OBJID_CARET, 0);
      }
      IntSetTimer(pWnd, IDCARETTIMER, gpsi->dtCaretBlink, NULL, TMRF_SYSTEM);
   }
   return TRUE;
}


/* SYSCALLS *****************************************************************/

BOOL
APIENTRY
NtUserCreateCaret(
   HWND hWnd,
   HBITMAP hBitmap,
   int nWidth,
   int nHeight)
{
   PWND Window;
   PTHREADINFO pti;
   PUSER_MESSAGE_QUEUE ThreadQueue;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserCreateCaret\n");
   UserEnterExclusive();

   if(!(Window = UserGetWindowObject(hWnd)))
   {
      RETURN(FALSE);
   }

   if(Window->head.pti->pEThread != PsGetCurrentThread())
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      RETURN(FALSE);
   }

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   if (ThreadQueue->CaretInfo->Visible)
   {
      IntKillTimer(Window, IDCARETTIMER, TRUE);
      co_IntHideCaret(ThreadQueue->CaretInfo);
   }

   ThreadQueue->CaretInfo->hWnd = hWnd;
   if(hBitmap)
   {
      ThreadQueue->CaretInfo->Bitmap = hBitmap;
      ThreadQueue->CaretInfo->Size.cx = ThreadQueue->CaretInfo->Size.cy = 0;
   }
   else
   {
      if (nWidth == 0)
      {
          nWidth = UserGetSystemMetrics(SM_CXBORDER);
      }
      if (nHeight == 0)
      {
          nHeight = UserGetSystemMetrics(SM_CYBORDER);
      }
      ThreadQueue->CaretInfo->Bitmap = (HBITMAP)0;
      ThreadQueue->CaretInfo->Size.cx = nWidth;
      ThreadQueue->CaretInfo->Size.cy = nHeight;
   }
   ThreadQueue->CaretInfo->Visible = 0;
   ThreadQueue->CaretInfo->Showing = 0;
   IntNotifyWinEvent(EVENT_OBJECT_CREATE, Window, OBJID_CARET, CHILDID_SELF, 0);
   RETURN(TRUE);

CLEANUP:
   TRACE("Leave NtUserCreateCaret, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

UINT
APIENTRY
NtUserGetCaretBlinkTime(VOID)
{
   UINT ret;

   UserEnterShared();

   ret = gpsi->dtCaretBlink;

   UserLeave();

   return ret;
}

BOOL
APIENTRY
NtUserGetCaretPos(
   LPPOINT lpPoint)
{
   PTHREADINFO pti;
   PUSER_MESSAGE_QUEUE ThreadQueue;
   NTSTATUS Status;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserGetCaretPos\n");
   UserEnterShared();

   pti = PsGetCurrentThreadWin32Thread();
   ThreadQueue = pti->MessageQueue;

   Status = MmCopyToCaller(lpPoint, &(ThreadQueue->CaretInfo->Pos), sizeof(POINT));
   if(!NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      RETURN(FALSE);
   }

   RETURN(TRUE);

CLEANUP:
   TRACE("Leave NtUserGetCaretPos, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

BOOL
APIENTRY
NtUserShowCaret(HWND hWnd OPTIONAL)
{
   PWND Window = NULL;
   USER_REFERENCE_ENTRY Ref;
   DECLARE_RETURN(BOOL);
   BOOL ret;

   TRACE("Enter NtUserShowCaret\n");
   UserEnterExclusive();

   if(hWnd && !(Window = UserGetWindowObject(hWnd)))
   {
      RETURN(FALSE);
   }

   if (Window) UserRefObjectCo(Window, &Ref);

   ret = co_UserShowCaret(Window);

   if (Window) UserDerefObjectCo(Window);

   RETURN(ret);

CLEANUP:
   TRACE("Leave NtUserShowCaret, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

BOOL
APIENTRY
NtUserHideCaret(HWND hWnd OPTIONAL)
{
   PWND Window = NULL;
   USER_REFERENCE_ENTRY Ref;
   DECLARE_RETURN(BOOL);
   BOOL ret;

   TRACE("Enter NtUserHideCaret\n");
   UserEnterExclusive();

   if(hWnd && !(Window = UserGetWindowObject(hWnd)))
   {
      RETURN(FALSE);
   }

   if (Window) UserRefObjectCo(Window, &Ref);

   ret = co_UserHideCaret(Window);

   if (Window) UserDerefObjectCo(Window);

   RETURN(ret);

CLEANUP:
   TRACE("Leave NtUserHideCaret, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}
