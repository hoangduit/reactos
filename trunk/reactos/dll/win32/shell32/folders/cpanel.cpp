/*
 * Control panel folder
 *
 * Copyright 2003 Martin Fuchs
 * Copyright 2009 Andrew Hill
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

/*
TODO:
1. The selected items list should not be stored in CControlPanelFolder, it should
    be a result returned by an internal method.
*/

#include <precomp.h>

WINE_DEFAULT_DEBUG_CHANNEL(shell);

/***********************************************************************
*   control panel implementation in shell namespace
*/

class CControlPanelEnum :
    public IEnumIDListImpl
{
    public:
        CControlPanelEnum();
        ~CControlPanelEnum();
        HRESULT WINAPI Initialize(DWORD dwFlags);
        BOOL RegisterCPanelApp(LPCSTR path);
        int RegisterRegistryCPanelApps(HKEY hkey_root, LPCSTR szRepPath);
        int RegisterCPanelFolders(HKEY hkey_root, LPCSTR szRepPath);
        BOOL CreateCPanelEnumList(DWORD dwFlags);

        BEGIN_COM_MAP(CControlPanelEnum)
        COM_INTERFACE_ENTRY_IID(IID_IEnumIDList, IEnumIDList)
        END_COM_MAP()
};

/***********************************************************************
*   IShellFolder [ControlPanel] implementation
*/

static const shvheader ControlPanelSFHeader[] = {
    {IDS_SHV_COLUMN8, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 15},/*FIXME*/
    {IDS_SHV_COLUMN9, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 200},/*FIXME*/
};

#define CONROLPANELSHELLVIEWCOLUMNS 2

CControlPanelEnum::CControlPanelEnum()
{
}

CControlPanelEnum::~CControlPanelEnum()
{
}

HRESULT WINAPI CControlPanelEnum::Initialize(DWORD dwFlags)
{
    if (CreateCPanelEnumList(dwFlags) == FALSE)
        return E_FAIL;
    return S_OK;
}

static LPITEMIDLIST _ILCreateCPanelApplet(LPCSTR pszName, LPCSTR pszDisplayName, LPCSTR pszComment, int iIconIdx)
{
    PIDLCPanelStruct *pCP;
    LPITEMIDLIST pidl;
    LPPIDLDATA pData;
    int cchName, cchDisplayName, cchComment, cbData;

    /* Calculate lengths of given strings */
    cchName = strlen(pszName);
    cchDisplayName = strlen(pszDisplayName);
    cchComment = strlen(pszComment);

    /* Allocate PIDL */
    cbData = sizeof(pidl->mkid.cb) + sizeof(pData->type) + sizeof(pData->u.cpanel) - sizeof(pData->u.cpanel.szName)
             + cchName + cchDisplayName + cchComment + 3;
    pidl = (LPITEMIDLIST)SHAlloc(cbData + sizeof(WORD));
    if (!pidl)
        return NULL;

    /* Copy data to allocated memory */
    pidl->mkid.cb = cbData;
    pData = (PIDLDATA *)pidl->mkid.abID;
    pData->type = PT_CPLAPPLET;

    pCP = &pData->u.cpanel;
    pCP->dummy = 0;
    pCP->iconIdx = iIconIdx;
    strcpy(pCP->szName, pszName);
    pCP->offsDispName = cchName + 1;
    strcpy(pCP->szName + pCP->offsDispName, pszDisplayName);
    pCP->offsComment = pCP->offsDispName + cchDisplayName + 1;
    strcpy(pCP->szName + pCP->offsComment, pszComment);

    /* Add PIDL NULL terminator */
    *(WORD*)(pCP->szName + pCP->offsComment + cchComment + 1) = 0;

    pcheck(pidl);

    return pidl;
}

/**************************************************************************
 *  _ILGetCPanelPointer()
 * gets a pointer to the control panel struct stored in the pidl
 */
static PIDLCPanelStruct *_ILGetCPanelPointer(LPCITEMIDLIST pidl)
{
    LPPIDLDATA pdata = _ILGetDataPointer(pidl);

    if (pdata && pdata->type == PT_CPLAPPLET)
        return (PIDLCPanelStruct *) & (pdata->u.cpanel);

    return NULL;
}

BOOL CControlPanelEnum::RegisterCPanelApp(LPCSTR path)
{
    LPITEMIDLIST pidl;
    CPlApplet* applet;
    CPanel panel;
    CPLINFO info;
    unsigned i;
    int iconIdx;

    char displayName[MAX_PATH];
    char comment[MAX_PATH];

    WCHAR wpath[MAX_PATH];

    MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);

    panel.first = NULL;
    applet = Control_LoadApplet(0, wpath, &panel);

    if (applet)
    {
        for (i = 0; i < applet->count; ++i)
        {
            WideCharToMultiByte(CP_ACP, 0, applet->info[i].szName, -1, displayName, MAX_PATH, 0, 0);
            WideCharToMultiByte(CP_ACP, 0, applet->info[i].szInfo, -1, comment, MAX_PATH, 0, 0);

            applet->proc(0, CPL_INQUIRE, i, (LPARAM)&info);

            if (info.idIcon > 0)
                iconIdx = -info.idIcon; /* negative icon index instead of icon number */
            else
                iconIdx = 0;

            pidl = _ILCreateCPanelApplet(path, displayName, comment, iconIdx);

            if (pidl)
                AddToEnumList(pidl);
        }
        Control_UnloadApplet(applet);
    }
    return TRUE;
}

int CControlPanelEnum::RegisterRegistryCPanelApps(HKEY hkey_root, LPCSTR szRepPath)
{
    char name[MAX_PATH];
    char value[MAX_PATH];
    HKEY hkey;

    int cnt = 0;

    if (RegOpenKeyA(hkey_root, szRepPath, &hkey) == ERROR_SUCCESS)
    {
        int idx = 0;

        for(; ; idx++)
        {
            DWORD nameLen = MAX_PATH;
            DWORD valueLen = MAX_PATH;

            if (RegEnumValueA(hkey, idx, name, &nameLen, NULL, NULL, (LPBYTE)&value, &valueLen) != ERROR_SUCCESS)
                break;

            if (RegisterCPanelApp(value))
                ++cnt;
        }
        RegCloseKey(hkey);
    }

    return cnt;
}

int CControlPanelEnum::RegisterCPanelFolders(HKEY hkey_root, LPCSTR szRepPath)
{
    char name[MAX_PATH];
    HKEY hkey;

    int cnt = 0;

    if (RegOpenKeyA(hkey_root, szRepPath, &hkey) == ERROR_SUCCESS)
    {
        int idx = 0;
        for (; ; idx++)
        {
            if (RegEnumKeyA(hkey, idx, name, MAX_PATH) != ERROR_SUCCESS)
                break;

            if (*name == '{')
            {
                LPITEMIDLIST pidl = _ILCreateGuidFromStrA(name);

                if (pidl && AddToEnumList(pidl))
                    ++cnt;
            }
        }

        RegCloseKey(hkey);
    }

    return cnt;
}

/**************************************************************************
 *  CControlPanelEnum::CreateCPanelEnumList()
 */
BOOL CControlPanelEnum::CreateCPanelEnumList(DWORD dwFlags)
{
    CHAR szPath[MAX_PATH];
    WIN32_FIND_DATAA wfd;
    HANDLE hFile;

    TRACE("(%p)->(flags=0x%08x)\n", this, dwFlags);

    /* enumerate control panel folders */
    if (dwFlags & SHCONTF_FOLDERS)
        RegisterCPanelFolders(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace");

    /* enumerate the control panel applets */
    if (dwFlags & SHCONTF_NONFOLDERS)
    {
        LPSTR p;

        GetSystemDirectoryA(szPath, MAX_PATH);
        p = PathAddBackslashA(szPath);
        strcpy(p, "*.cpl");

        TRACE("-- (%p)-> enumerate SHCONTF_NONFOLDERS of %s\n", this, debugstr_a(szPath));
        hFile = FindFirstFileA(szPath, &wfd);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(dwFlags & SHCONTF_INCLUDEHIDDEN) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
                    continue;

                if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    strcpy(p, wfd.cFileName);
                    if (strcmp(wfd.cFileName, "ncpa.cpl"))
                        RegisterCPanelApp(szPath);
                }
            } while(FindNextFileA(hFile, &wfd));
            FindClose(hFile);
        }

        RegisterRegistryCPanelApps(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls");
        RegisterRegistryCPanelApps(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls");
    }
    return TRUE;
}

CControlPanelFolder::CControlPanelFolder()
{
    pidlRoot = NULL;    /* absolute pidl */
    dwAttributes = 0;        /* attributes returned by GetAttributesOf FIXME: use it */
    apidl = NULL;
    cidl = 0;
}

CControlPanelFolder::~CControlPanelFolder()
{
    TRACE("-- destroying IShellFolder(%p)\n", this);
    SHFree(pidlRoot);
}

HRESULT WINAPI CControlPanelFolder::FinalConstruct()
{
    pidlRoot = _ILCreateControlPanel();    /* my qualified pidl */
    if (pidlRoot == NULL)
        return E_OUTOFMEMORY;
    return S_OK;
}

/**************************************************************************
*    CControlPanelFolder::ParseDisplayName
*/
HRESULT WINAPI CControlPanelFolder::ParseDisplayName(
    HWND hwndOwner,
    LPBC pbc,
    LPOLESTR lpszDisplayName,
    DWORD *pchEaten,
    PIDLIST_RELATIVE *ppidl,
    DWORD *pdwAttributes)
{
    WCHAR szElement[MAX_PATH];
    LPCWSTR szNext = NULL;
    LPITEMIDLIST pidlTemp = NULL;
    HRESULT hr = S_OK;
    CLSID clsid;

    TRACE ("(%p)->(HWND=%p,%p,%p=%s,%p,pidl=%p,%p)\n",
           this, hwndOwner, pbc, lpszDisplayName, debugstr_w(lpszDisplayName),
           pchEaten, ppidl, pdwAttributes);

    if (!lpszDisplayName || !ppidl)
        return E_INVALIDARG;

    *ppidl = 0;

    if (pchEaten)
        *pchEaten = 0;        /* strange but like the original */

    if (lpszDisplayName[0] == ':' && lpszDisplayName[1] == ':')
    {
        szNext = GetNextElementW (lpszDisplayName, szElement, MAX_PATH);
        TRACE ("-- element: %s\n", debugstr_w (szElement));
        CLSIDFromString (szElement + 2, &clsid);
        pidlTemp = _ILCreateGuid (PT_GUID, clsid);
    }
    else if( (pidlTemp = SHELL32_CreatePidlFromBindCtx(pbc, lpszDisplayName)) )
    {
        *ppidl = pidlTemp;
        return S_OK;
    }

    if (SUCCEEDED(hr) && pidlTemp)
    {
        if (szNext && *szNext)
        {
            hr = SHELL32_ParseNextElement(this, hwndOwner, pbc,
                                          &pidlTemp, (LPOLESTR) szNext, pchEaten, pdwAttributes);
        }
        else
        {
            if (pdwAttributes && *pdwAttributes)
                hr = SHELL32_GetItemAttributes(this,
                                               pidlTemp, pdwAttributes);
        }
    }

    *ppidl = pidlTemp;

    TRACE ("(%p)->(-- ret=0x%08x)\n", this, hr);

    return hr;
}

/**************************************************************************
*        CControlPanelFolder::EnumObjects
*/
HRESULT WINAPI CControlPanelFolder::EnumObjects(
    HWND hwndOwner,
    DWORD dwFlags,
    LPENUMIDLIST *ppEnumIDList)
{
    CComObject<CControlPanelEnum>            *theEnumerator;
    CComPtr<IEnumIDList>                    result;
    HRESULT                                    hResult;

    TRACE ("(%p)->(HWND=%p flags=0x%08x pplist=%p)\n", this, hwndOwner, dwFlags, ppEnumIDList);

    if (ppEnumIDList == NULL)
        return E_POINTER;
    *ppEnumIDList = NULL;
    ATLTRY (theEnumerator = new CComObject<CControlPanelEnum>);
    if (theEnumerator == NULL)
        return E_OUTOFMEMORY;
    hResult = theEnumerator->QueryInterface(IID_PPV_ARG(IEnumIDList, &result));
    if (FAILED (hResult))
    {
        delete theEnumerator;
        return hResult;
    }
    hResult = theEnumerator->Initialize (dwFlags);
    if (FAILED (hResult))
        return hResult;
    *ppEnumIDList = result.Detach ();

    TRACE ("-- (%p)->(new ID List: %p)\n", this, *ppEnumIDList);

    return S_OK;
}

/**************************************************************************
*        CControlPanelFolder::BindToObject
*/
HRESULT WINAPI CControlPanelFolder::BindToObject(
    PCUIDLIST_RELATIVE pidl,
    LPBC pbcReserved,
    REFIID riid,
    LPVOID *ppvOut)
{
    TRACE("(%p)->(pidl=%p,%p,%s,%p)\n", this, pidl, pbcReserved, shdebugstr_guid(&riid), ppvOut);

    return SHELL32_BindToChild(pidlRoot, NULL, pidl, riid, ppvOut);
}

/**************************************************************************
*    CControlPanelFolder::BindToStorage
*/
HRESULT WINAPI CControlPanelFolder::BindToStorage(
    PCUIDLIST_RELATIVE pidl,
    LPBC pbcReserved,
    REFIID riid,
    LPVOID *ppvOut)
{
    FIXME("(%p)->(pidl=%p,%p,%s,%p) stub\n", this, pidl, pbcReserved, shdebugstr_guid(&riid), ppvOut);

    *ppvOut = NULL;
    return E_NOTIMPL;
}

/**************************************************************************
*     CControlPanelFolder::CompareIDs
*/

HRESULT WINAPI CControlPanelFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
    int nReturn;

    TRACE("(%p)->(0x%08lx,pidl1=%p,pidl2=%p)\n", this, lParam, pidl1, pidl2);
    nReturn = SHELL32_CompareIDs(this, lParam, pidl1, pidl2);
    TRACE("-- %i\n", nReturn);
    return nReturn;
}

/**************************************************************************
*    CControlPanelFolder::CreateViewObject
*/
HRESULT WINAPI CControlPanelFolder::CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvOut)
{
    CComPtr<IShellView>                    pShellView;
    HRESULT hr = E_INVALIDARG;

    TRACE("(%p)->(hwnd=%p,%s,%p)\n", this, hwndOwner, shdebugstr_guid(&riid), ppvOut);

    if (ppvOut) {
        *ppvOut = NULL;

        if (IsEqualIID(riid, IID_IDropTarget)) {
            WARN("IDropTarget not implemented\n");
            hr = E_NOTIMPL;
        } else if (IsEqualIID(riid, IID_IContextMenu)) {
            WARN("IContextMenu not implemented\n");
            hr = E_NOTIMPL;
        } else if (IsEqualIID(riid, IID_IShellView)) {
            hr = IShellView_Constructor((IShellFolder *)this, &pShellView);
            if (pShellView) {
                hr = pShellView->QueryInterface(riid, ppvOut);
            }
        }
    }
    TRACE("--(%p)->(interface=%p)\n", this, ppvOut);
    return hr;
}

/**************************************************************************
*  CControlPanelFolder::GetAttributesOf
*/
HRESULT WINAPI CControlPanelFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD * rgfInOut)
{
    HRESULT hr = S_OK;

    TRACE("(%p)->(cidl=%d apidl=%p mask=%p (0x%08x))\n",
          this, cidl, apidl, rgfInOut, rgfInOut ? *rgfInOut : 0);

    if (!rgfInOut)
        return E_INVALIDARG;
    if (cidl && !apidl)
        return E_INVALIDARG;

    if (*rgfInOut == 0)
        *rgfInOut = ~0;

    while(cidl > 0 && *apidl)
    {
        pdump(*apidl);
        SHELL32_GetItemAttributes(this, *apidl, rgfInOut);
        apidl++;
        cidl--;
    }
    /* make sure SFGAO_VALIDATE is cleared, some apps depend on that */
    *rgfInOut &= ~SFGAO_VALIDATE;

    TRACE("-- result=0x%08x\n", *rgfInOut);
    return hr;
}

/**************************************************************************
*    CControlPanelFolder::GetUIObjectOf
*
* PARAMETERS
*  HWND           hwndOwner, //[in ] Parent window for any output
*  UINT           cidl,      //[in ] array size
*  LPCITEMIDLIST* apidl,     //[in ] simple pidl array
*  REFIID         riid,      //[in ] Requested Interface
*  UINT*          prgfInOut, //[   ] reserved
*  LPVOID*        ppvObject) //[out] Resulting Interface
*
*/
HRESULT WINAPI CControlPanelFolder::GetUIObjectOf(HWND hwndOwner,
        UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    LPITEMIDLIST pidl;
    IUnknown *pObj = NULL;
    HRESULT hr = E_INVALIDARG;

    TRACE("(%p)->(%p,%u,apidl=%p,%s,%p,%p)\n",
          this, hwndOwner, cidl, apidl, shdebugstr_guid(&riid), prgfInOut, ppvOut);

    if (ppvOut) {
        *ppvOut = NULL;

        if (IsEqualIID(riid, IID_IContextMenu) && (cidl >= 1)) {
            // TODO
            // create a seperate item struct
            //
            pObj = (IContextMenu *)this;
            this->apidl = apidl;
            this->cidl = cidl;
            pObj->AddRef();
            hr = S_OK;
        } else if (IsEqualIID(riid, IID_IDataObject) && (cidl >= 1)) {
            hr = IDataObject_Constructor(hwndOwner, pidlRoot, apidl, cidl, (IDataObject **)&pObj);
        } else if (IsEqualIID(riid, IID_IExtractIconA) && (cidl == 1)) {
            pidl = ILCombine(pidlRoot, apidl[0]);
            pObj = (LPUNKNOWN) IExtractIconA_Constructor(pidl);
            SHFree(pidl);
            hr = S_OK;
        } else if (IsEqualIID(riid, IID_IExtractIconW) && (cidl == 1)) {
            pidl = ILCombine(pidlRoot, apidl[0]);
            pObj = (LPUNKNOWN) IExtractIconW_Constructor(pidl);
            SHFree(pidl);
            hr = S_OK;
        } else if ((IsEqualIID(riid, IID_IShellLinkW) || IsEqualIID(riid, IID_IShellLinkA))
                   && (cidl == 1)) {
            pidl = ILCombine(pidlRoot, apidl[0]);
            hr = IShellLink_ConstructFromFile(NULL, riid, pidl, (LPVOID*)&pObj);
            SHFree(pidl);
        } else {
            hr = E_NOINTERFACE;
        }

        if (SUCCEEDED(hr) && !pObj)
            hr = E_OUTOFMEMORY;

        *ppvOut = pObj;
    }
    TRACE("(%p)->hr=0x%08x\n", this, hr);
    return hr;
}

/**************************************************************************
*    CControlPanelFolder::GetDisplayNameOf
*/
HRESULT WINAPI CControlPanelFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, DWORD dwFlags, LPSTRRET strRet)
{
    CHAR szName[MAX_PATH];
    WCHAR wszName[MAX_PATH+1]; /* +1 for potential backslash */
    PIDLCPanelStruct *pCPanel;
    HRESULT hr;

    *szName = '\0';

    TRACE("(%p)->(pidl=%p,0x%08x,%p)\n", this, pidl, dwFlags, strRet);
    pdump(pidl);

    if (!pidl)
        return S_FALSE;

    pCPanel = _ILGetCPanelPointer(pidl);

    if (pCPanel)
    {
        /* copy display name from pidl - it was retrived from applet before;
           SHGDN_FORPARSING does not need special handling */
        lstrcpyA(szName, pCPanel->szName + pCPanel->offsDispName);
    }
    /* take names of special folders only if it's only this folder */
    else if (_ILIsSpecialFolder(pidl))
    {
        BOOL bSimplePidl = _ILIsPidlSimple(pidl);
        SFGAOF Attr = SFGAO_FILESYSTEM;

        SHELL32_GetItemAttributes(this, pidl, &Attr);
        if (Attr & SFGAO_FILESYSTEM)
        {
            hr = SHELL32_GetDisplayNameOfChild(this, pidl, dwFlags, wszName, sizeof(wszName));
            if (FAILED(hr))
                return hr;
        }
        else if (bSimplePidl)
        {
            _ILSimpleGetTextW(pidl, wszName, MAX_PATH);    /* append my own path */
        }
        else
        {
            FIXME("special pidl\n");
            if (dwFlags & SHGDN_FORPARSING)
            {
                /* go deeper if needed */
                int cchName;

                PathAddBackslashW(wszName);
                cchName = wcslen(wszName);

                hr = SHELL32_GetDisplayNameOfChild(this, pidl, dwFlags, wszName + cchName, MAX_PATH + 1 - cchName);
                if (FAILED(hr))
                    return hr;
            }
        }

        if (!WideCharToMultiByte(CP_ACP, 0, wszName, -1, szName, MAX_PATH, NULL, NULL))
            szName[0] = '\0';
    }

    strRet->uType = STRRET_CSTR;
    lstrcpynA(strRet->cStr, szName, MAX_PATH);

    TRACE("--(%p)->(%s)\n", this, szName);
    return S_OK;
}

/**************************************************************************
*  CControlPanelFolder::SetNameOf
*  Changes the name of a file object or subfolder, possibly changing its item
*  identifier in the process.
*
* PARAMETERS
*  HWND          hwndOwner,  //[in ] Owner window for output
*  LPCITEMIDLIST pidl,       //[in ] simple pidl of item to change
*  LPCOLESTR     lpszName,   //[in ] the items new display name
*  DWORD         dwFlags,    //[in ] SHGNO formatting flags
*  LPITEMIDLIST* ppidlOut)   //[out] simple pidl returned
*/
HRESULT WINAPI CControlPanelFolder::SetNameOf(HWND hwndOwner, PCUITEMID_CHILD pidl,    /*simple pidl */
        LPCOLESTR lpName, DWORD dwFlags, PITEMID_CHILD *pPidlOut)
{
    FIXME("(%p)->(%p,pidl=%p,%s,%u,%p)\n", this, hwndOwner, pidl, debugstr_w(lpName), dwFlags, pPidlOut);
    return E_FAIL;
}

HRESULT WINAPI CControlPanelFolder::GetDefaultSearchGUID(GUID *pguid)
{
    FIXME("(%p)\n", this);
    return E_NOTIMPL;
}

HRESULT WINAPI CControlPanelFolder::EnumSearches(IEnumExtraSearch **ppenum)
{
    FIXME("(%p)\n", this);
    return E_NOTIMPL;
}

HRESULT WINAPI CControlPanelFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    TRACE("(%p)\n", this);

    if (pSort) *pSort = 0;
    if (pDisplay) *pDisplay = 0;
    return S_OK;
}

HRESULT WINAPI CControlPanelFolder::GetDefaultColumnState(UINT iColumn, DWORD *pcsFlags)
{
    TRACE("(%p)\n", this);

    if (!pcsFlags || iColumn >= CONROLPANELSHELLVIEWCOLUMNS) return E_INVALIDARG;
    *pcsFlags = ControlPanelSFHeader[iColumn].pcsFlags;
    return S_OK;
}

HRESULT WINAPI CControlPanelFolder::GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    FIXME("(%p)\n", this);
    return E_NOTIMPL;
}

HRESULT WINAPI CControlPanelFolder::GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS *psd)
{
    HRESULT hr;

    TRACE("(%p)->(%p %i %p)\n", this, pidl, iColumn, psd);

    if (!psd || iColumn >= CONROLPANELSHELLVIEWCOLUMNS)
        return E_INVALIDARG;

    if (!pidl) {
        psd->fmt = ControlPanelSFHeader[iColumn].fmt;
        psd->cxChar = ControlPanelSFHeader[iColumn].cxChar;
        psd->str.uType = STRRET_CSTR;
        LoadStringA(shell32_hInstance, ControlPanelSFHeader[iColumn].colnameid, psd->str.cStr, MAX_PATH);
        return S_OK;
    } else {
        psd->str.cStr[0] = 0x00;
        psd->str.uType = STRRET_CSTR;
        switch(iColumn) {
            case 0:        /* name */
                hr = GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_INFOLDER, &psd->str);
                break;
            case 1:        /* comment */
                _ILGetFileType(pidl, psd->str.cStr, MAX_PATH);
                break;
        }
        hr = S_OK;
    }

    return hr;
}

HRESULT WINAPI CControlPanelFolder::MapColumnToSCID(UINT column, SHCOLUMNID *pscid)
{
    FIXME("(%p)\n", this);
    return E_NOTIMPL;
}

/************************************************************************
 *    CControlPanelFolder::GetClassID
 */
HRESULT WINAPI CControlPanelFolder::GetClassID(CLSID *lpClassId)
{
    TRACE("(%p)\n", this);

    if (!lpClassId)
        return E_POINTER;
    *lpClassId = CLSID_ControlPanel;

    return S_OK;
}

/************************************************************************
 *    CControlPanelFolder::Initialize
 *
 * NOTES: it makes no sense to change the pidl
 */
HRESULT WINAPI CControlPanelFolder::Initialize(LPCITEMIDLIST pidl)
{
    if (pidlRoot)
        SHFree((LPVOID)pidlRoot);

    pidlRoot = ILClone(pidl);
    return S_OK;
}

/**************************************************************************
 *    CControlPanelFolder::GetCurFolder
 */
HRESULT WINAPI CControlPanelFolder::GetCurFolder(LPITEMIDLIST * pidl)
{
    TRACE("(%p)->(%p)\n", this, pidl);

    if (!pidl)
        return E_POINTER;
    *pidl = ILClone(pidlRoot);
    return S_OK;
}

HRESULT CPanel_GetIconLocationW(LPCITEMIDLIST pidl, LPWSTR szIconFile, UINT cchMax, int* piIndex)
{
    PIDLCPanelStruct* pcpanel = _ILGetCPanelPointer(pidl);

    if (!pcpanel)
        return E_INVALIDARG;

    MultiByteToWideChar(CP_ACP, 0, pcpanel->szName, -1, szIconFile, cchMax);
    *piIndex = (int)pcpanel->iconIdx != -1 ? pcpanel->iconIdx : 0;

    return S_OK;
}


/**************************************************************************
* IShellExecuteHookW Implementation
*/

static HRESULT
ExecuteAppletFromCLSID(LPOLESTR pOleStr)
{
    WCHAR wszBuf[128], wszCmd[MAX_PATH];
    DWORD cbCmd = sizeof(wszCmd);

    StringCbPrintfW(wszBuf, sizeof(wszBuf), L"CLSID\\%s\\shell\\open\\command", pOleStr);

    if (RegGetValueW(HKEY_CLASSES_ROOT, wszBuf, NULL, RRF_RT_REG_SZ, NULL, (PVOID)wszCmd, &cbCmd) != ERROR_SUCCESS)
    {
        ERR("RegGetValueW(%ls) failed with %u\n", wszBuf, GetLastError());
        return E_FAIL;
    }

    if (!ExpandEnvironmentStringsW(wszCmd, wszBuf, _countof(wszBuf)))
        return E_FAIL;

    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcessW(NULL, wszBuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        return E_FAIL;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return S_OK;
}

EXTERN_C void WINAPI Control_RunDLLW(HWND hWnd, HINSTANCE hInst, LPCWSTR cmd, DWORD nCmdShow);

HRESULT WINAPI CControlPanelFolder::ExecuteFromIdList(LPCITEMIDLIST pidl)
{
    PIDLCPanelStruct *pCPanel = _ILGetCPanelPointer(ILFindLastID(pidl));

    if (!pCPanel)
    {
        /* Is it GUID to control panel applet? */
        IID *piid = _ILGetGUIDPointer(ILFindLastID(pidl));
        if (!piid)
            return E_INVALIDARG;

        /* Start it */
        LPOLESTR pOleStr;
        if (StringFromCLSID(*piid, &pOleStr) == S_OK)
        {
            HRESULT hr = ExecuteAppletFromCLSID(pOleStr);
            CoTaskMemFree(pOleStr);
            return hr;
        }

        ERR("Cannot open cpanel applet\n");
        return E_INVALIDARG;
    }

    /* Build control panel applet cmd
       Note: we pass the applet name to Control_RunDLL to distinguish between multiple applets in one .cpl file */
    WCHAR wszCmd[2*MAX_PATH];
    WCHAR wszAppletName[MAX_PATH];

    if(!MultiByteToWideChar(CP_ACP, 0, pCPanel->szName + pCPanel->offsDispName, -1, wszAppletName, MAX_PATH))
        return E_FAIL;

    StringCbPrintfW(wszCmd, sizeof(wszCmd), L"rundll32 shell32.dll,Control_RunDLL \"%hs\",\"%ls\"", pCPanel->szName, wszAppletName);

    /* Start the applet */
    TRACE("Run cpl %ls\n", wszCmd);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcessW(NULL, wszCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        return E_FAIL;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return S_OK;
}

HRESULT WINAPI CControlPanelFolder::Execute(LPSHELLEXECUTEINFOW psei)
{
    TRACE("(%p)->execute(%p)\n", this, psei);

    if (!psei)
        return E_INVALIDARG;

    if (!(psei->fMask & SEE_MASK_IDLIST))
    {
        FIXME("no idlist given!\n");
        return E_FAIL;
    }

    return ExecuteFromIdList((LPCITEMIDLIST)psei->lpIDList);
}

/**************************************************************************
* IShellExecuteHookA Implementation
*/

HRESULT WINAPI CControlPanelFolder::Execute(LPSHELLEXECUTEINFOA psei)
{
    TRACE("(%p)->execute(%p)\n", this, psei);

    if (!psei)
        return E_INVALIDARG;

    if (!(psei->fMask & SEE_MASK_IDLIST))
    {
        FIXME("no idlist given!\n");
        return E_FAIL;
    }

    return ExecuteFromIdList((LPCITEMIDLIST)psei->lpIDList);
}

/**************************************************************************
* IContextMenu2 Implementation
*/

/**************************************************************************
* ICPanel_IContextMenu_QueryContextMenu()
*/
HRESULT WINAPI CControlPanelFolder::QueryContextMenu(
    HMENU hMenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags)
{
    WCHAR szBuffer[30] = {0};
    ULONG Count = 1;

    TRACE("(%p)->(hmenu=%p indexmenu=%x cmdfirst=%x cmdlast=%x flags=%x )\n",
          this, hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

    if (LoadStringW(shell32_hInstance, IDS_OPEN, szBuffer, sizeof(szBuffer) / sizeof(WCHAR)))
    {
        szBuffer[(sizeof(szBuffer)/sizeof(WCHAR))-1] = L'\0';
        _InsertMenuItemW(hMenu, indexMenu++, TRUE, IDS_OPEN, MFT_STRING, szBuffer, MFS_DEFAULT); //FIXME identifier
        Count++;
    }

    if (LoadStringW(shell32_hInstance, IDS_CREATELINK, szBuffer, sizeof(szBuffer) / sizeof(WCHAR)))
    {
        if (Count)
        {
            _InsertMenuItemW(hMenu, indexMenu++, TRUE, idCmdFirst + Count, MFT_SEPARATOR, NULL, MFS_ENABLED);
        }
        szBuffer[(sizeof(szBuffer)/sizeof(WCHAR))-1] = L'\0';

        _InsertMenuItemW(hMenu, indexMenu++, TRUE, IDS_CREATELINK, MFT_STRING, szBuffer, MFS_ENABLED); //FIXME identifier
        Count++;
    }
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, Count);
}

/**************************************************************************
* ICPanel_IContextMenu_InvokeCommand()
*/
HRESULT WINAPI CControlPanelFolder::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    SHELLEXECUTEINFOW sei;
    WCHAR szPath[MAX_PATH];
    char szTarget[MAX_PATH];
    STRRET strret;
    WCHAR* pszPath;
    INT Length, cLength;
    PIDLCPanelStruct *pcpanel;
    CComPtr<IPersistFile>                ppf;
    CComPtr<IShellLinkA>                isl;
    HRESULT hResult;

    TRACE("(%p)->(invcom=%p verb=%p wnd=%p)\n", this, lpcmi, lpcmi->lpVerb, lpcmi->hwnd);

    if (lpcmi->lpVerb == MAKEINTRESOURCEA(IDS_OPEN)) //FIXME
    {
        ZeroMemory(&sei, sizeof(sei));
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.lpIDList = ILCombine(pidlRoot, apidl[0]);
        sei.hwnd = lpcmi->hwnd;
        sei.nShow = SW_SHOWNORMAL;
        sei.lpVerb = L"open";
ERR("here\n");
        return Execute(&sei);
    }
    else if (lpcmi->lpVerb == MAKEINTRESOURCEA(IDS_CREATELINK)) //FIXME
    {
        if (!SHGetSpecialFolderPathW(NULL, szPath, CSIDL_DESKTOPDIRECTORY, FALSE))
            return E_FAIL;

        pszPath = PathAddBackslashW(szPath);
        if (!pszPath)
            return E_FAIL;

        if (GetDisplayNameOf(apidl[0], SHGDN_FORPARSING, &strret) != S_OK)
            return E_FAIL;

        Length =  MAX_PATH - (pszPath - szPath);
        cLength = strlen(strret.cStr);
        if (Length < cLength + 5)
        {
            FIXME("\n");
            return E_FAIL;
        }

        if (MultiByteToWideChar(CP_ACP, 0, strret.cStr, cLength + 1, pszPath, Length))
        {
            pszPath += cLength;
            Length -= cLength;
        }

        if (Length > 10)
        {
            wcscpy(pszPath, L" - ");
            cLength = LoadStringW(shell32_hInstance, IDS_LNK_FILE, &pszPath[3], Length - 4) + 3;
            if (cLength + 5 > Length)
                cLength = Length - 5;
            Length -= cLength;
            pszPath += cLength;
        }
        wcscpy(pszPath, L".lnk");

        pcpanel = _ILGetCPanelPointer(ILFindLastID(apidl[0]));
        if (pcpanel)
        {
            strncpy(szTarget, pcpanel->szName, MAX_PATH);
        }
        else
        {
            FIXME("Couldn't retrieve pointer to cpl structure\n");
            return E_FAIL;
        }
        hResult = CShellLink::_CreatorClass::CreateInstance(NULL, IID_PPV_ARG(IShellLinkA, &isl));
        if (SUCCEEDED(hResult))
        {
            isl->SetPath(szTarget);
            if (SUCCEEDED(isl->QueryInterface(IID_PPV_ARG(IPersistFile,&ppf))))
                ppf->Save(szPath, TRUE);
        }
        return NOERROR;
    }
    return S_OK;
}

/**************************************************************************
 *  ICPanel_IContextMenu_GetCommandString()
 *
 */
HRESULT WINAPI CControlPanelFolder::GetCommandString(
    UINT_PTR idCommand,
    UINT uFlags,
    UINT* lpReserved,
    LPSTR lpszName,
    UINT uMaxNameLen)
{
    TRACE("(%p)->(idcom=%lx flags=%x %p name=%p len=%x)\n", this, idCommand, uFlags, lpReserved, lpszName, uMaxNameLen);

    FIXME("unknown command string\n");
    return E_FAIL;
}

/**************************************************************************
* ICPanel_IContextMenu_HandleMenuMsg()
*/
HRESULT WINAPI CControlPanelFolder::HandleMenuMsg(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    TRACE("ICPanel_IContextMenu_HandleMenuMsg (%p)->(msg=%x wp=%lx lp=%lx)\n", this, uMsg, wParam, lParam);

    return E_NOTIMPL;
}
