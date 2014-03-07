/*
 * Unit tests for imm32
 *
 * Copyright (c) 2008 Michael Jung
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>

#include "wine/test.h"
#include "winuser.h"
#include "wingdi.h"
#include "imm.h"
#include "ddk/imm.h"

#define NUMELEMS(array) (sizeof((array))/sizeof((array)[0]))

static BOOL (WINAPI *pImmAssociateContextEx)(HWND,HIMC,DWORD);
static BOOL (WINAPI *pImmIsUIMessageA)(HWND,UINT,WPARAM,LPARAM);
static UINT (WINAPI *pSendInput) (UINT, INPUT*, size_t);

/*
 * msgspy - record and analyse message traces sent to a certain window
 */
typedef struct _msgs {
    CWPSTRUCT    msg;
    BOOL         post;
} imm_msgs;

static struct _msg_spy {
    HWND         hwnd;
    HHOOK        get_msg_hook;
    HHOOK        call_wnd_proc_hook;
    imm_msgs     msgs[32];
    unsigned int i_msg;
} msg_spy;

typedef struct
{
    DWORD type;
    union
    {
        MOUSEINPUT      mi;
        KEYBDINPUT      ki;
        HARDWAREINPUT   hi;
    } u;
} TEST_INPUT;

static UINT (WINAPI *pSendInput) (UINT, INPUT*, size_t);

static LRESULT CALLBACK get_msg_filter(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (HC_ACTION == nCode) {
        MSG *msg = (MSG*)lParam;

        if ((msg->hwnd == msg_spy.hwnd || msg_spy.hwnd == NULL) &&
            (msg_spy.i_msg < NUMELEMS(msg_spy.msgs)))
        {
            msg_spy.msgs[msg_spy.i_msg].msg.hwnd    = msg->hwnd;
            msg_spy.msgs[msg_spy.i_msg].msg.message = msg->message;
            msg_spy.msgs[msg_spy.i_msg].msg.wParam  = msg->wParam;
            msg_spy.msgs[msg_spy.i_msg].msg.lParam  = msg->lParam;
            msg_spy.msgs[msg_spy.i_msg].post = TRUE;
            msg_spy.i_msg++;
        }
    }

    return CallNextHookEx(msg_spy.get_msg_hook, nCode, wParam, lParam);
}

static LRESULT CALLBACK call_wnd_proc_filter(int nCode, WPARAM wParam,
                                             LPARAM lParam)
{
    if (HC_ACTION == nCode) {
        CWPSTRUCT *cwp = (CWPSTRUCT*)lParam;

        if (((cwp->hwnd == msg_spy.hwnd || msg_spy.hwnd == NULL)) &&
            (msg_spy.i_msg < NUMELEMS(msg_spy.msgs)))
        {
            memcpy(&msg_spy.msgs[msg_spy.i_msg].msg, cwp, sizeof(msg_spy.msgs[0].msg));
            msg_spy.msgs[msg_spy.i_msg].post = FALSE;
            msg_spy.i_msg++;
        }
    }

    return CallNextHookEx(msg_spy.call_wnd_proc_hook, nCode, wParam, lParam);
}

static void msg_spy_pump_msg_queue(void) {
    MSG msg;

    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return;
}

static void msg_spy_flush_msgs(void) {
    msg_spy_pump_msg_queue();
    msg_spy.i_msg = 0;
}

static imm_msgs* msg_spy_find_next_msg(UINT message, UINT *start) {
    UINT i;

    msg_spy_pump_msg_queue();

    if (msg_spy.i_msg >= NUMELEMS(msg_spy.msgs))
        fprintf(stdout, "%s:%d: msg_spy: message buffer overflow!\n",
                __FILE__, __LINE__);

    for (i = *start; i < msg_spy.i_msg; i++)
        if (msg_spy.msgs[i].msg.message == message)
        {
            *start = i+1;
            return &msg_spy.msgs[i];
        }

    return NULL;
}

static imm_msgs* msg_spy_find_msg(UINT message) {
    UINT i = 0;

    return msg_spy_find_next_msg(message, &i);
}

static void msg_spy_init(HWND hwnd) {
    msg_spy.hwnd = hwnd;
    msg_spy.get_msg_hook =
            SetWindowsHookEx(WH_GETMESSAGE, get_msg_filter, GetModuleHandle(0),
                             GetCurrentThreadId());
    msg_spy.call_wnd_proc_hook =
            SetWindowsHookEx(WH_CALLWNDPROC, call_wnd_proc_filter,
                             GetModuleHandle(0), GetCurrentThreadId());
    msg_spy.i_msg = 0;

    msg_spy_flush_msgs();
}

static void msg_spy_cleanup(void) {
    if (msg_spy.get_msg_hook)
        UnhookWindowsHookEx(msg_spy.get_msg_hook);
    if (msg_spy.call_wnd_proc_hook)
        UnhookWindowsHookEx(msg_spy.call_wnd_proc_hook);
    memset(&msg_spy, 0, sizeof(msg_spy));
}

/*
 * imm32 test cases - Issue some IMM commands on a dummy window and analyse the
 * messages being sent to this window in response.
 */
static const char wndcls[] = "winetest_imm32_wndcls";
static HWND hwnd;

static LRESULT WINAPI wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_IME_SETCONTEXT:
        case WM_NCCREATE:
        case WM_CREATE:
            return TRUE;
    }

    return DefWindowProcA(hwnd,msg,wParam,lParam);
}

static BOOL init(void) {
    WNDCLASSEX wc;
    HIMC imc;
    HMODULE hmod,huser;

    hmod = GetModuleHandleA("imm32.dll");
    huser = GetModuleHandleA("user32");
    pImmAssociateContextEx = (void*)GetProcAddress(hmod, "ImmAssociateContextEx");
    pImmIsUIMessageA = (void*)GetProcAddress(hmod, "ImmIsUIMessageA");
    pSendInput = (void*)GetProcAddress(huser, "SendInput");

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = wndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GetModuleHandle(0);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = wndcls;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExA(&wc))
        return FALSE;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, wndcls, "Wine imm32.dll test",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          240, 120, NULL, NULL, GetModuleHandle(0), NULL);
    if (!hwnd)
        return FALSE;

    imc = ImmGetContext(hwnd);
    if (!imc)
    {
        win_skip("IME support not implemented\n");
        return FALSE;
    }
    ImmReleaseContext(hwnd, imc);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    msg_spy_init(hwnd);

    return TRUE;
}

static void cleanup(void) {
    msg_spy_cleanup();
    if (hwnd)
        DestroyWindow(hwnd);
    UnregisterClass(wndcls, GetModuleHandle(0));
}

static void test_ImmNotifyIME(void) {
    static const char string[] = "wine";
    char resstr[16] = "";
    HIMC imc;
    BOOL ret;

    imc = ImmGetContext(hwnd);
    msg_spy_flush_msgs();

    ret = ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    ok(broken(!ret) ||
       ret, /* Vista+ */
       "Canceling an empty composition string should succeed.\n");
    ok(!msg_spy_find_msg(WM_IME_COMPOSITION), "Windows does not post "
       "WM_IME_COMPOSITION in response to NI_COMPOSITIONSTR / CPS_CANCEL, if "
       "the composition string being canceled is empty.\n");

    ImmSetCompositionString(imc, SCS_SETSTR, string, sizeof(string), NULL, 0);
    msg_spy_flush_msgs();

    ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    msg_spy_flush_msgs();

    /* behavior differs between win9x and NT */
    ret = ImmGetCompositionString(imc, GCS_COMPSTR, resstr, sizeof(resstr));
    ok(!ret, "After being cancelled the composition string is empty.\n");

    msg_spy_flush_msgs();

    ret = ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    ok(broken(!ret) ||
       ret, /* Vista+ */
       "Canceling an empty composition string should succeed.\n");
    ok(!msg_spy_find_msg(WM_IME_COMPOSITION), "Windows does not post "
       "WM_IME_COMPOSITION in response to NI_COMPOSITIONSTR / CPS_CANCEL, if "
       "the composition string being canceled is empty.\n");

    msg_spy_flush_msgs();
    ImmReleaseContext(hwnd, imc);
}

static void test_ImmGetCompositionString(void)
{
    HIMC imc;
    static const WCHAR string[] = {'w','i','n','e',0x65e5,0x672c,0x8a9e};
    char cstring[20];
    WCHAR wstring[20];
    DWORD len;
    DWORD alen,wlen;

    imc = ImmGetContext(hwnd);
    ImmSetCompositionStringW(imc, SCS_SETSTR, string, sizeof(string), NULL,0);
    alen = ImmGetCompositionStringA(imc, GCS_COMPSTR, cstring, 20);
    wlen = ImmGetCompositionStringW(imc, GCS_COMPSTR, wstring, 20);
    /* windows machines without any IME installed just return 0 above */
    if( alen && wlen)
    {
        len = ImmGetCompositionStringW(imc, GCS_COMPATTR, NULL, 0);
        ok(len*sizeof(WCHAR)==wlen,"GCS_COMPATTR(W) not returning correct count\n");
        len = ImmGetCompositionStringA(imc, GCS_COMPATTR, NULL, 0);
        ok(len==alen,"GCS_COMPATTR(A) not returning correct count\n");
    }
    ImmReleaseContext(hwnd, imc);
}

static void test_ImmSetCompositionString(void)
{
    HIMC imc;
    BOOL ret;

    SetLastError(0xdeadbeef);
    imc = ImmGetContext(hwnd);
    ok(imc != 0, "ImmGetContext() failed. Last error: %u\n", GetLastError());
    if (!imc)
        return;

    ret = ImmSetCompositionStringW(imc, SCS_SETSTR, NULL, 0, NULL, 0);
    ok(broken(!ret) ||
       ret, /* Vista+ */
       "ImmSetCompositionStringW() failed.\n");

    ret = ImmSetCompositionStringW(imc, SCS_SETSTR | SCS_CHANGEATTR,
        NULL, 0, NULL, 0);
    ok(!ret, "ImmSetCompositionStringW() succeeded.\n");

    ret = ImmSetCompositionStringW(imc, SCS_SETSTR | SCS_CHANGECLAUSE,
        NULL, 0, NULL, 0);
    ok(!ret, "ImmSetCompositionStringW() succeeded.\n");

    ret = ImmSetCompositionStringW(imc, SCS_CHANGEATTR | SCS_CHANGECLAUSE,
        NULL, 0, NULL, 0);
    ok(!ret, "ImmSetCompositionStringW() succeeded.\n");

    ret = ImmSetCompositionStringW(imc, SCS_SETSTR | SCS_CHANGEATTR | SCS_CHANGECLAUSE,
        NULL, 0, NULL, 0);
    ok(!ret, "ImmSetCompositionStringW() succeeded.\n");

    ImmReleaseContext(hwnd, imc);
}

static void test_ImmIME(void)
{
    HIMC imc;

    imc = ImmGetContext(hwnd);
    if (imc)
    {
        BOOL rc;
        rc = ImmConfigureIMEA(imc, NULL, IME_CONFIG_REGISTERWORD, NULL);
        ok (rc == 0, "ImmConfigureIMEA did not fail\n");
        rc = ImmConfigureIMEW(imc, NULL, IME_CONFIG_REGISTERWORD, NULL);
        ok (rc == 0, "ImmConfigureIMEW did not fail\n");
    }
    ImmReleaseContext(hwnd,imc);
}

static void test_ImmAssociateContextEx(void)
{
    HIMC imc;
    BOOL rc;

    if (!pImmAssociateContextEx) return;

    imc = ImmGetContext(hwnd);
    if (imc)
    {
        HIMC retimc, newimc;

        newimc = ImmCreateContext();
        ok(newimc != imc, "handles should not be the same\n");
        rc = pImmAssociateContextEx(NULL, NULL, 0);
        ok(!rc, "ImmAssociateContextEx succeeded\n");
        rc = pImmAssociateContextEx(hwnd, NULL, 0);
        ok(rc, "ImmAssociateContextEx failed\n");
        rc = pImmAssociateContextEx(NULL, imc, 0);
        ok(!rc, "ImmAssociateContextEx succeeded\n");

        rc = pImmAssociateContextEx(hwnd, imc, 0);
        ok(rc, "ImmAssociateContextEx failed\n");
        retimc = ImmGetContext(hwnd);
        ok(retimc == imc, "handles should be the same\n");
        ImmReleaseContext(hwnd,retimc);

        rc = pImmAssociateContextEx(hwnd, newimc, 0);
        ok(rc, "ImmAssociateContextEx failed\n");
        retimc = ImmGetContext(hwnd);
        ok(retimc == newimc, "handles should be the same\n");
        ImmReleaseContext(hwnd,retimc);

        rc = pImmAssociateContextEx(hwnd, NULL, IACE_DEFAULT);
        ok(rc, "ImmAssociateContextEx failed\n");
    }
    ImmReleaseContext(hwnd,imc);
}

typedef struct _igc_threadinfo {
    HWND hwnd;
    HANDLE event;
    HIMC himc;
} igc_threadinfo;


static DWORD WINAPI ImmGetContextThreadFunc( LPVOID lpParam)
{
    HIMC h1,h2;
    HWND hwnd2;
    COMPOSITIONFORM cf;
    POINT pt;
    igc_threadinfo *info= (igc_threadinfo*)lpParam;
    info->hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, wndcls, "Wine imm32.dll test",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          240, 120, NULL, NULL, GetModuleHandle(0), NULL);

    h1 = ImmGetContext(hwnd);
    todo_wine ok(info->himc == h1, "hwnd context changed in new thread\n");
    h2 = ImmGetContext(info->hwnd);
    todo_wine ok(h2 != h1, "new hwnd in new thread should have different context\n");
    info->himc = h2;
    ImmReleaseContext(hwnd,h1);

    hwnd2 = CreateWindowEx(WS_EX_CLIENTEDGE, wndcls, "Wine imm32.dll test",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          240, 120, NULL, NULL, GetModuleHandle(0), NULL);
    h1 = ImmGetContext(hwnd2);

    ok(h1 == h2, "Windows in same thread should have same default context\n");
    ImmReleaseContext(hwnd2,h1);
    ImmReleaseContext(info->hwnd,h2);
    DestroyWindow(hwnd2);

    /* priming for later tests */
    ImmSetCompositionWindow(h1, &cf);
    ImmSetStatusWindowPos(h1, &pt);

    SetEvent(info->event);
    Sleep(INFINITE);
    return 1;
}

static void test_ImmThreads(void)
{
    HIMC himc, otherHimc, h1;
    igc_threadinfo threadinfo;
    HANDLE hThread;
    DWORD dwThreadId;
    BOOL rc;
    LOGFONT lf;
    COMPOSITIONFORM cf;
    DWORD status, sentence;
    POINT pt;

    himc = ImmGetContext(hwnd);
    threadinfo.event = CreateEvent(NULL, TRUE, FALSE, NULL);
    threadinfo.himc = himc;
    hThread = CreateThread(NULL, 0, ImmGetContextThreadFunc, &threadinfo, 0, &dwThreadId );
    WaitForSingleObject(threadinfo.event, INFINITE);

    otherHimc = ImmGetContext(threadinfo.hwnd);

    todo_wine ok(himc != otherHimc, "Windows from other threads should have different himc\n");
    todo_wine ok(otherHimc == threadinfo.himc, "Context from other thread should not change in main thread\n");

    if (0) /* FIXME: Causes wine to hang */
    {
    h1 = ImmAssociateContext(hwnd,otherHimc);
    ok(h1 == NULL, "Should fail to be able to Associate a default context from a different thread\n");
    h1 = ImmGetContext(hwnd);
    ok(h1 == himc, "Context for window should remain unchanged\n");
    ImmReleaseContext(hwnd,h1);
    }


    /* OpenStatus */
    rc = ImmSetOpenStatus(himc, TRUE);
    ok(rc != 0, "ImmSetOpenStatus failed\n");
    rc = ImmGetOpenStatus(himc);
    ok(rc != 0, "ImmGetOpenStatus failed\n");
    rc = ImmSetOpenStatus(himc, FALSE);
    ok(rc != 0, "ImmSetOpenStatus failed\n");
    rc = ImmGetOpenStatus(himc);
    ok(rc == 0, "ImmGetOpenStatus failed\n");

    rc = ImmSetOpenStatus(otherHimc, TRUE);
    todo_wine ok(rc == 0, "ImmSetOpenStatus should fail\n");
    rc = ImmGetOpenStatus(otherHimc);
    todo_wine ok(rc == 0, "ImmGetOpenStatus failed\n");
    rc = ImmSetOpenStatus(otherHimc, FALSE);
    todo_wine ok(rc == 0, "ImmSetOpenStatus should fail\n");
    rc = ImmGetOpenStatus(otherHimc);
    ok(rc == 0, "ImmGetOpenStatus failed\n");

    /* CompositionFont */
    rc = ImmGetCompositionFont(himc, &lf);
    ok(rc != 0, "ImmGetCompositionFont failed\n");
    rc = ImmSetCompositionFont(himc, &lf);
    ok(rc != 0, "ImmSetCompositionFont failed\n");

    rc = ImmGetCompositionFont(otherHimc, &lf);
    ok(rc != 0 || broken(rc == 0), "ImmGetCompositionFont failed\n");
    rc = ImmSetCompositionFont(otherHimc, &lf);
    todo_wine ok(rc == 0, "ImmSetCompositionFont should fail\n");

    /* CompositionWindow */
    rc = ImmSetCompositionWindow(himc, &cf);
    ok(rc != 0, "ImmSetCompositionWindow failed\n");
    rc = ImmGetCompositionWindow(himc, &cf);
    ok(rc != 0, "ImmGetCompositionWindow failed\n");

    rc = ImmSetCompositionWindow(otherHimc, &cf);
    todo_wine ok(rc == 0, "ImmSetCompositionWindow should fail\n");
    rc = ImmGetCompositionWindow(otherHimc, &cf);
    ok(rc != 0 || broken(rc == 0), "ImmGetCompositionWindow failed\n");

    /* ConversionStatus */
    rc = ImmGetConversionStatus(himc, &status, &sentence);
    ok(rc != 0, "ImmGetConversionStatus failed\n");
    rc = ImmSetConversionStatus(himc, status, sentence);
    ok(rc != 0, "ImmSetConversionStatus failed\n");

    rc = ImmGetConversionStatus(otherHimc, &status, &sentence);
    ok(rc != 0 || broken(rc == 0), "ImmGetConversionStatus failed\n");
    rc = ImmSetConversionStatus(otherHimc, status, sentence);
    todo_wine ok(rc == 0, "ImmSetConversionStatus should fail\n");

    /* StatusWindowPos */
    rc = ImmSetStatusWindowPos(himc, &pt);
    ok(rc != 0, "ImmSetStatusWindowPos failed\n");
    rc = ImmGetStatusWindowPos(himc, &pt);
    ok(rc != 0, "ImmGetStatusWindowPos failed\n");

    rc = ImmSetStatusWindowPos(otherHimc, &pt);
    todo_wine ok(rc == 0, "ImmSetStatusWindowPos should fail\n");
    rc = ImmGetStatusWindowPos(otherHimc, &pt);
    ok(rc != 0 || broken(rc == 0), "ImmGetStatusWindowPos failed\n");

    ImmReleaseContext(threadinfo.hwnd,otherHimc);
    ImmReleaseContext(hwnd,himc);

    DestroyWindow(threadinfo.hwnd);
    TerminateThread(hThread, 1);

    himc = ImmGetContext(GetDesktopWindow());
    todo_wine ok(himc == NULL, "Should not be able to get himc from other process window\n");
}

static void test_ImmIsUIMessage(void)
{
    struct test
    {
        UINT msg;
        BOOL ret;
    };

    static const struct test tests[] =
    {
        { WM_MOUSEMOVE,            FALSE },
        { WM_IME_STARTCOMPOSITION, TRUE  },
        { WM_IME_ENDCOMPOSITION,   TRUE  },
        { WM_IME_COMPOSITION,      TRUE  },
        { WM_IME_SETCONTEXT,       TRUE  },
        { WM_IME_NOTIFY,           TRUE  },
        { WM_IME_CONTROL,          FALSE },
        { WM_IME_COMPOSITIONFULL,  TRUE  },
        { WM_IME_SELECT,           TRUE  },
        { WM_IME_CHAR,             FALSE },
        { 0x287 /* FIXME */,       TRUE  },
        { WM_IME_REQUEST,          FALSE },
        { WM_IME_KEYDOWN,          FALSE },
        { WM_IME_KEYUP,            FALSE },
        { 0, FALSE } /* mark the end */
    };

    const struct test *test;
    BOOL ret;

    if (!pImmIsUIMessageA) return;

    for (test = tests; test->msg; test++)
    {
        msg_spy_flush_msgs();
        ret = pImmIsUIMessageA(NULL, test->msg, 0, 0);
        ok(ret == test->ret, "ImmIsUIMessageA returned %x for %x\n", ret, test->msg);
        ok(!msg_spy_find_msg(test->msg), "Windows does not send 0x%x for NULL hwnd\n", test->msg);

        ret = pImmIsUIMessageA(hwnd, test->msg, 0, 0);
        ok(ret == test->ret, "ImmIsUIMessageA returned %x for %x\n", ret, test->msg);
        if (ret)
            ok(msg_spy_find_msg(test->msg) != NULL, "Windows does send 0x%x\n", test->msg);
        else
            ok(!msg_spy_find_msg(test->msg), "Windows does not send 0x%x\n", test->msg);
    }
}

static void test_ImmGetContext(void)
{
    HIMC himc;
    DWORD err;

    SetLastError(0xdeadbeef);
    himc = ImmGetContext((HWND)0xffffffff);
    err = GetLastError();
    ok(himc == NULL, "ImmGetContext succeeded\n");
    ok(err == ERROR_INVALID_WINDOW_HANDLE, "got %u\n", err);

    himc = ImmGetContext(hwnd);
    ok(himc != NULL, "ImmGetContext failed\n");
    ok(ImmReleaseContext(hwnd, himc), "ImmReleaseContext failed\n");
}

static void test_ImmGetDescription(void)
{
    HKL hkl;
    WCHAR japime[] = { 'E', '0', '0', '1', '0', '4', '1', '1', 0 };
    WCHAR descW[100];
    CHAR descA[100];
    UINT ret, lret;

    /* FIXME: invalid keyboard layouts should not pass */
    ret = ImmGetDescriptionW(NULL, NULL, 0);
    todo_wine ok(!ret, "ImmGetDescriptionW failed, expected 0 received %d.\n", ret);

    /* load a language with valid IMM descriptions */
    hkl = LoadKeyboardLayoutW(japime, KLF_ACTIVATE);
    todo_wine ok(hkl != 0, "LoadKeyboardLayoutW failed, expected != 0.\n");

    ret = ImmGetDescriptionW(hkl, NULL, 0);
    if(!ret)
    {
        win_skip("ImmGetDescriptionW is not working for current loaded keyboard.\n");
        return;
    }

    ret = ImmGetDescriptionW(hkl, descW, 0);
    ok(ret, "ImmGetDescriptionW failed, expected != 0 received 0.\n");

    lret = ImmGetDescriptionW(hkl, descW, ret + 1);
    ok(lret, "ImmGetDescriptionW failed, expected != 0 received 0.\n");
    ok(lret == ret, "ImmGetDescriptionW failed to return the correct amount of data. Expected %d, got %d.\n", ret, lret);

    lret = ImmGetDescriptionA(hkl, descA, ret + 1);
    ok(lret, "ImmGetDescriptionA failed, expected != 0 received 0.\n");
    todo_wine ok(lret == ret, "ImmGetDescriptionA failed to return the correct amount of data. Expected %d, got %d.\n", ret, lret);

    ret /= 2; /* try to copy partially */
    lret = ImmGetDescriptionW(hkl, descW, ret + 1);
    ok(lret, "ImmGetDescriptionW failed, expected != 0 received 0.\n");
    ok(lret == ret, "ImmGetDescriptionW failed to return the correct amount of data. Expected %d, got %d.\n", ret, lret);

    ret = ImmGetDescriptionW(hkl, descW, 1);
    ok(!ret, "ImmGetDescriptionW failed, expected 0 received %d.\n", ret);

    UnloadKeyboardLayout(hkl);
}

static void test_ImmDefaultHwnd(void)
{
    HIMC imc1, imc2, imc3;
    HWND def1, def3;
    HWND hwnd;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Wine imm32.dll test",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          240, 120, NULL, NULL, GetModuleHandle(0), NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);

    imc1 = ImmGetContext(hwnd);
    if (!imc1)
    {
        win_skip("IME support not implemented\n");
        return;
    }

    def1 = ImmGetDefaultIMEWnd(hwnd);

    imc2 = ImmCreateContext();
    ImmSetOpenStatus(imc2, TRUE);

    imc3 = ImmGetContext(hwnd);
    def3 = ImmGetDefaultIMEWnd(hwnd);

    ok(def3 == def1, "Default IME window should not change\n");
    ok(imc1 == imc3, "IME context should not change\n");
    ImmSetOpenStatus(imc2, FALSE);

    ImmReleaseContext(hwnd, imc1);
    ImmReleaseContext(hwnd, imc3);
    ImmDestroyContext(imc2);
    DestroyWindow(hwnd);
}

static void test_ImmGetIMCLockCount(void)
{
    HIMC imc;
    DWORD count, ret, i;
    INPUTCONTEXT *ic;

    imc = ImmCreateContext();
    count = ImmGetIMCLockCount(imc);
    ok(count == 0, "expect 0, returned %d\n", count);
    ic = ImmLockIMC(imc);
    ok(ic != NULL, "ImmLockIMC failed!\n");
    count = ImmGetIMCLockCount(imc);
    ok(count == 1, "expect 1, returned %d\n", count);
    ret = ImmUnlockIMC(imc);
    ok(ret == TRUE, "expect TRUE, ret %d\n", ret);
    count = ImmGetIMCLockCount(imc);
    ok(count == 0, "expect 0, returned %d\n", count);
    ret = ImmUnlockIMC(imc);
    ok(ret == TRUE, "expect TRUE, ret %d\n", ret);
    count = ImmGetIMCLockCount(imc);
    ok(count == 0, "expect 0, returned %d\n", count);

    for (i = 0; i < GMEM_LOCKCOUNT * 2; i++)
    {
        ic = ImmLockIMC(imc);
        ok(ic != NULL, "ImmLockIMC failed!\n");
    }
    count = ImmGetIMCLockCount(imc);
    todo_wine ok(count == GMEM_LOCKCOUNT, "expect GMEM_LOCKCOUNT, returned %d\n", count);

    for (i = 0; i < GMEM_LOCKCOUNT - 1; i++)
        ImmUnlockIMC(imc);
    count = ImmGetIMCLockCount(imc);
    todo_wine ok(count == 1, "expect 1, returned %d\n", count);
    ImmUnlockIMC(imc);
    count = ImmGetIMCLockCount(imc);
    todo_wine ok(count == 0, "expect 0, returned %d\n", count);

    ImmDestroyContext(imc);
}

static void test_ImmGetIMCCLockCount(void)
{
    HIMCC imcc;
    DWORD count, g_count, ret, i;
    VOID *p;

    imcc = ImmCreateIMCC(sizeof(CANDIDATEINFO));
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 0, "expect 0, returned %d\n", count);
    ImmLockIMCC(imcc);
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 1, "expect 1, returned %d\n", count);
    ret = ImmUnlockIMCC(imcc);
    ok(ret == FALSE, "expect FALSE, ret %d\n", ret);
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 0, "expect 0, returned %d\n", count);
    ret = ImmUnlockIMCC(imcc);
    ok(ret == FALSE, "expect FALSE, ret %d\n", ret);
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 0, "expect 0, returned %d\n", count);

    p = ImmLockIMCC(imcc);
    ok(GlobalHandle(p) == imcc, "expect %p, returned %p\n", imcc, GlobalHandle(p));

    for (i = 0; i < GMEM_LOCKCOUNT * 2; i++)
    {
        ImmLockIMCC(imcc);
        count = ImmGetIMCCLockCount(imcc);
        g_count = GlobalFlags(imcc) & GMEM_LOCKCOUNT;
        ok(count == g_count, "count %d, g_count %d\n", count, g_count);
    }
    count = ImmGetIMCCLockCount(imcc);
    ok(count == GMEM_LOCKCOUNT, "expect GMEM_LOCKCOUNT, returned %d\n", count);

    for (i = 0; i < GMEM_LOCKCOUNT - 1; i++)
        GlobalUnlock(imcc);
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 1, "expect 1, returned %d\n", count);
    GlobalUnlock(imcc);
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 0, "expect 0, returned %d\n", count);

    ImmDestroyIMCC(imcc);
}

static void test_ImmDestroyContext(void)
{
    HIMC imc;
    DWORD ret, count;
    INPUTCONTEXT *ic;

    imc = ImmCreateContext();
    count = ImmGetIMCLockCount(imc);
    ok(count == 0, "expect 0, returned %d\n", count);
    ic = ImmLockIMC(imc);
    ok(ic != NULL, "ImmLockIMC failed!\n");
    count = ImmGetIMCLockCount(imc);
    ok(count == 1, "expect 1, returned %d\n", count);
    ret = ImmDestroyContext(imc);
    ok(ret == TRUE, "Destroy a locked IMC should success!\n");
    ic = ImmLockIMC(imc);
    todo_wine ok(ic == NULL, "Lock a destroyed IMC should fail!\n");
    ret = ImmUnlockIMC(imc);
    todo_wine ok(ret == FALSE, "Unlock a destroyed IMC should fail!\n");
    count = ImmGetIMCLockCount(imc);
    todo_wine ok(count == 0, "Get lock count of a destroyed IMC should return 0!\n");
    SetLastError(0xdeadbeef);
    ret = ImmDestroyContext(imc);
    todo_wine ok(ret == FALSE, "Destroy a destroyed IMC should fail!\n");
    ret = GetLastError();
    todo_wine ok(ret == ERROR_INVALID_HANDLE, "wrong last error %08x!\n", ret);
}

static void test_ImmDestroyIMCC(void)
{
    HIMCC imcc;
    DWORD ret, count, size;
    VOID *p;

    imcc = ImmCreateIMCC(sizeof(CANDIDATEINFO));
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 0, "expect 0, returned %d\n", count);
    p = ImmLockIMCC(imcc);
    ok(p != NULL, "ImmLockIMCC failed!\n");
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 1, "expect 1, returned %d\n", count);
    size = ImmGetIMCCSize(imcc);
    ok(size == sizeof(CANDIDATEINFO), "returned %d\n", size);
    p = ImmDestroyIMCC(imcc);
    ok(p == NULL, "Destroy a locked IMCC should success!\n");
    p = ImmLockIMCC(imcc);
    ok(p == NULL, "Lock a destroyed IMCC should fail!\n");
    ret = ImmUnlockIMCC(imcc);
    ok(ret == FALSE, "Unlock a destroyed IMCC should return FALSE!\n");
    count = ImmGetIMCCLockCount(imcc);
    ok(count == 0, "Get lock count of a destroyed IMCC should return 0!\n");
    size = ImmGetIMCCSize(imcc);
    ok(size == 0, "Get size of a destroyed IMCC should return 0!\n");
    SetLastError(0xdeadbeef);
    p = ImmDestroyIMCC(imcc);
    ok(p != NULL, "returned NULL\n");
    ret = GetLastError();
    ok(ret == ERROR_INVALID_HANDLE, "wrong last error %08x!\n", ret);
}

static void test_ImmMessages(void)
{
    CANDIDATEFORM cf;
    imm_msgs *msg;
    HWND defwnd;
    HIMC imc;
    UINT idx = 0;

    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Wine imm32.dll test",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          240, 120, NULL, NULL, GetModuleHandle(0), NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    defwnd = ImmGetDefaultIMEWnd(hwnd);
    imc = ImmGetContext(hwnd);

    ImmSetOpenStatus(imc, TRUE);
    msg_spy_flush_msgs();
    SendMessage(defwnd, WM_IME_CONTROL, IMC_GETCANDIDATEPOS, (LPARAM)&cf );
    do
    {
        msg = msg_spy_find_next_msg(WM_IME_CONTROL,&idx);
        if (msg) ok(!msg->post, "Message should not be posted\n");
    } while (msg);
    msg_spy_flush_msgs();
    ImmSetOpenStatus(imc, FALSE);
    ImmReleaseContext(hwnd, imc);
    DestroyWindow(hwnd);
}

static LRESULT CALLBACK processkey_wnd_proc( HWND hWnd, UINT msg, WPARAM wParam,
        LPARAM lParam )
{
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void test_ime_processkey(void)
{
    WCHAR classNameW[] = {'P','r','o','c','e','s','s', 'K','e','y','T','e','s','t','C','l','a','s','s',0};
    WCHAR windowNameW[] = {'P','r','o','c','e','s','s', 'K','e','y',0};

    MSG msg;
    WNDCLASSW wclass;
    HANDLE hInstance = GetModuleHandleW(NULL);
    TEST_INPUT inputs[2];
    HIMC imc;
    INT rc;
    HWND hWndTest;

    wclass.lpszClassName = classNameW;
    wclass.style         = CS_HREDRAW | CS_VREDRAW;
    wclass.lpfnWndProc   = processkey_wnd_proc;
    wclass.hInstance     = hInstance;
    wclass.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wclass.hCursor       = LoadCursor( NULL, IDC_ARROW);
    wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wclass.lpszMenuName  = 0;
    wclass.cbClsExtra    = 0;
    wclass.cbWndExtra    = 0;
    if(!RegisterClassW(&wclass)){
        win_skip("Failed to register window.\n");
        return;
    }

    /* create the test window that will receive the keystrokes */
    hWndTest = CreateWindowW(wclass.lpszClassName, windowNameW,
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 100, 100,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hWndTest, SW_SHOW);
    SetWindowPos(hWndTest, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    SetForegroundWindow(hWndTest);
    UpdateWindow(hWndTest);

    imc = ImmGetContext(hWndTest);
    if (!imc)
    {
        win_skip("IME not supported\n");
        DestroyWindow(hWndTest);
        return;
    }

    rc = ImmSetOpenStatus(imc, TRUE);
    if (rc != TRUE)
    {
        win_skip("Unable to open IME\n");
        ImmReleaseContext(hWndTest, imc);
        DestroyWindow(hWndTest);
        return;
    }

    /* flush pending messages */
    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);

    SetFocus(hWndTest);

    /* init input data that never changes */
    inputs[1].type = inputs[0].type = INPUT_KEYBOARD;
    inputs[1].u.ki.dwExtraInfo = inputs[0].u.ki.dwExtraInfo = 0;
    inputs[1].u.ki.time = inputs[0].u.ki.time = 0;

    /* Pressing a key */
    inputs[0].u.ki.wVk = 0x41;
    inputs[0].u.ki.wScan = 0x1e;
    inputs[0].u.ki.dwFlags = 0x0;

    pSendInput(1, (INPUT*)inputs, sizeof(INPUT));

    while(PeekMessageW(&msg, hWndTest, 0, 0, PM_NOREMOVE)) {
        if(msg.message != WM_KEYDOWN)
            PeekMessageW(&msg, hWndTest, 0, 0, PM_REMOVE);
        else
        {
            ok(msg.wParam != VK_PROCESSKEY,"Incorrect ProcessKey Found\n");
            PeekMessageW(&msg, hWndTest, 0, 0, PM_REMOVE);
            if(msg.wParam == VK_PROCESSKEY)
                trace("ProcessKey was correctly found\n");
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    inputs[0].u.ki.wVk = 0x41;
    inputs[0].u.ki.wScan = 0x1e;
    inputs[0].u.ki.dwFlags = KEYEVENTF_KEYUP;

    pSendInput(1, (INPUT*)inputs, sizeof(INPUT));

    while(PeekMessageW(&msg, hWndTest, 0, 0, PM_NOREMOVE)) {
        if(msg.message != WM_KEYUP)
            PeekMessageW(&msg, hWndTest, 0, 0, PM_REMOVE);
        else
        {
            ok(msg.wParam != VK_PROCESSKEY,"Incorrect ProcessKey Found\n");
            PeekMessageW(&msg, hWndTest, 0, 0, PM_REMOVE);
            ok(msg.wParam != VK_PROCESSKEY,"ProcessKey should still not be Found\n");
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    ImmReleaseContext(hWndTest, imc);
    ImmSetOpenStatus(imc, FALSE);
    DestroyWindow(hWndTest);
}

START_TEST(imm32) {
    if (init())
    {
        test_ImmNotifyIME();
        test_ImmGetCompositionString();
        test_ImmSetCompositionString();
        test_ImmIME();
        test_ImmAssociateContextEx();
        test_ImmThreads();
        test_ImmIsUIMessage();
        test_ImmGetContext();
        test_ImmGetDescription();
        test_ImmDefaultHwnd();
        test_ImmGetIMCLockCount();
        test_ImmGetIMCCLockCount();
        test_ImmDestroyContext();
        test_ImmDestroyIMCC();
        msg_spy_cleanup();
        /* Reinitialize the hooks to capture all windows */
        msg_spy_init(NULL);
        test_ImmMessages();
        msg_spy_cleanup();
        if (pSendInput)
            test_ime_processkey();
        else win_skip("SendInput is not available\n");
    }
    cleanup();
}
