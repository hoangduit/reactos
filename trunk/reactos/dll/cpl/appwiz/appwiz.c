/*
 * PROJECT:                 ReactOS Software Control Panel
 * FILE:                    dll/cpl/appwiz/appwiz.c
 * PURPOSE:                 ReactOS Software Control Panel
 * PROGRAMMERS:             Gero Kuehn (reactos.filter@gkware.com)
 *                          Dmitry Chapyshev (dmitry@reactos.org)
 */

#include <config.h>
#include <wine/port.h>

#include "appwiz.h"

WINE_DEFAULT_DEBUG_CHANNEL(appwiz);

HINSTANCE hApplet = NULL;

static LONG start_params(const WCHAR *params)
{
    static const WCHAR install_geckoW[] = {'i','n','s','t','a','l','l','_','g','e','c','k','o',0};
    static const WCHAR install_monoW[] = {'i','n','s','t','a','l','l','_','m','o','n','o',0};

    if(!params)
        return FALSE;

    if(!strcmpW(params, install_geckoW)) {
        install_addon(ADDON_GECKO);
        return TRUE;
    }

    if(!strcmpW(params, install_monoW)) {
        install_addon(ADDON_MONO);
        return TRUE;
    }

    WARN("unknown param %s\n", debugstr_w(params));
    return FALSE;
}

/* Control Panel Callback */
LONG CALLBACK
CPlApplet(HWND hwndCPl, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    CPLINFO *CPlInfo;

    switch (uMsg)
    {
        case CPL_INIT:
            return TRUE;

        case CPL_GETCOUNT:
            return 1;

        case CPL_STARTWPARMSW:
            return start_params((const WCHAR *)lParam2);

        case CPL_INQUIRE:
            CPlInfo = (CPLINFO*)lParam2;
            CPlInfo->lData = 0;
            CPlInfo->idIcon = IDI_CPLSYSTEM;
            CPlInfo->idName = IDS_CPLSYSTEMNAME;
            CPlInfo->idInfo = IDS_CPLSYSTEMDESCRIPTION;
            break;

        case CPL_DBLCLK:
            ShellExecuteW(NULL,
                          NULL,
                          L"rapps.exe",
                          NULL,
                          NULL,
                          1);
            break;
    }

    return FALSE;
}

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            CoInitialize(NULL);
            hApplet = hinstDLL;
            break;
    }

    return TRUE;
}
