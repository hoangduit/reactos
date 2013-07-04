/*
 *    Virtual Workplace folder
 *
 *    Copyright 1997                Marcus Meissner
 *    Copyright 1998, 1999, 2002    Juergen Schmied
 *    Copyright 2009                Andrew Hill
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

#include <precomp.h>

WINE_DEFAULT_DEBUG_CHANNEL (shell);

/*
CDrivesFolder should create a CRegFolder to represent the virtual items that exist only in
the registry. The CRegFolder is aggregated by the CDrivesFolder.
The CDrivesFolderEnum class should enumerate only drives on the system. Since the CRegFolder
implementation of IShellFolder::EnumObjects enumerates the virtual items, the
CDrivesFolderEnum is only responsible for returning the physical items.

2. At least on my XP system, the drive pidls returned are of type PT_DRIVE1, not PT_DRIVE
3. The parsing name returned for my computer is incorrect. It should be "My Computer"
*/

/***********************************************************************
*   IShellFolder implementation
*/

class CDrivesFolderEnum :
    public IEnumIDListImpl
{
    public:
        CDrivesFolderEnum();
        ~CDrivesFolderEnum();
        HRESULT WINAPI Initialize(HWND hwndOwner, DWORD dwFlags);
        BOOL CreateMyCompEnumList(DWORD dwFlags);

        BEGIN_COM_MAP(CDrivesFolderEnum)
        COM_INTERFACE_ENTRY_IID(IID_IEnumIDList, IEnumIDList)
        END_COM_MAP()
};

/***********************************************************************
*   IShellFolder [MyComputer] implementation
*/

static const shvheader MyComputerSFHeader[] = {
    {IDS_SHV_COLUMN1, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 15},
    {IDS_SHV_COLUMN3, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 10},
    {IDS_SHV_COLUMN6, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 10},
    {IDS_SHV_COLUMN7, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 10},
};

#define MYCOMPUTERSHELLVIEWCOLUMNS 4

CDrivesFolderEnum::CDrivesFolderEnum()
{
}

CDrivesFolderEnum::~CDrivesFolderEnum()
{
}

HRESULT WINAPI CDrivesFolderEnum::Initialize(HWND hwndOwner, DWORD dwFlags)
{
    if (CreateMyCompEnumList(dwFlags) == FALSE)
        return E_FAIL;

    return S_OK;
}

/**************************************************************************
 *  CDrivesFolderEnum::CreateMyCompEnumList()
 */

BOOL CDrivesFolderEnum::CreateMyCompEnumList(DWORD dwFlags)
{
    BOOL bRet = TRUE;
    static const WCHAR MyComputer_NameSpaceW[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\Namespace";

    TRACE("(%p)->(flags=0x%08x)\n", this, dwFlags);

    /* enumerate the folders */
    if (dwFlags & SHCONTF_FOLDERS)
    {
        WCHAR wszDriveName[] = {'A', ':', '\\', '\0'};
        DWORD dwDrivemap = GetLogicalDrives();
        HKEY hKey;
        UINT i;

        while (bRet && wszDriveName[0] <= 'Z')
        {
            if(dwDrivemap & 0x00000001L)
                bRet = AddToEnumList(_ILCreateDrive(wszDriveName));
            wszDriveName[0]++;
            dwDrivemap = dwDrivemap >> 1;
        }

        TRACE("-- (%p)-> enumerate (mycomputer shell extensions)\n", this);
        for (i = 0; i < 2; i++)
        {
            if (bRet && ERROR_SUCCESS == RegOpenKeyExW(i == 0 ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                    MyComputer_NameSpaceW, 0, KEY_READ, &hKey))
            {
                WCHAR wszBuf[50];
                DWORD dwSize, j = 0;
                LONG ErrorCode;
                LPITEMIDLIST pidl;

                while (bRet)
                {
                    dwSize = sizeof(wszBuf) / sizeof(wszBuf[0]);
                    ErrorCode = RegEnumKeyExW(hKey, j, wszBuf, &dwSize, 0, NULL, NULL, NULL);
                    if (ERROR_SUCCESS == ErrorCode)
                    {
                        if (wszBuf[0] != L'{')
                        {
                            dwSize = sizeof(wszBuf);
                            RegGetValueW(hKey, wszBuf, NULL, RRF_RT_REG_SZ, NULL, wszBuf, &dwSize);
                        }

                        /* FIXME: shell extensions - the type should be PT_SHELLEXT (tested) */
                        pidl = _ILCreateGuidFromStrW(wszBuf);
                        if (pidl != NULL)
                            bRet = AddToEnumList(pidl);
                        else
                            ERR("Invalid MyComputer namespace extesion: %s\n", wszBuf);
                        j++;
                    }
                    else if (ERROR_NO_MORE_ITEMS == ErrorCode)
                        break;
                    else
                        bRet = FALSE;
                }
                RegCloseKey(hKey);
            }
        }
    }
    return bRet;
}

CDrivesFolder::CDrivesFolder()
{
    pidlRoot = NULL;
    sName = NULL;
}

CDrivesFolder::~CDrivesFolder()
{
    TRACE ("-- destroying IShellFolder(%p)\n", this);
    SHFree(pidlRoot);
}

HRESULT WINAPI CDrivesFolder::FinalConstruct()
{
    DWORD dwSize;
    WCHAR szName[MAX_PATH];
    WCHAR wszMyCompKey[256];
    INT i;

    pidlRoot = _ILCreateMyComputer();    /* my qualified pidl */
    if (pidlRoot == NULL)
        return E_OUTOFMEMORY;

    i = swprintf(wszMyCompKey, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\");
    StringFromGUID2(CLSID_MyComputer, wszMyCompKey + i, sizeof(wszMyCompKey) / sizeof(wszMyCompKey[0]) - i);
    dwSize = sizeof(szName);
    if (RegGetValueW(HKEY_CURRENT_USER, wszMyCompKey,
                     NULL, RRF_RT_REG_SZ, NULL, szName, &dwSize) == ERROR_SUCCESS)
    {
        sName = (LPWSTR)SHAlloc((wcslen(szName) + 1) * sizeof(WCHAR));
        if (sName)
            wcscpy(sName, szName);
        TRACE("sName %s\n", debugstr_w(sName));
    }
    return S_OK;
}

/**************************************************************************
*    CDrivesFolder::ParseDisplayName
*/
HRESULT WINAPI CDrivesFolder::ParseDisplayName(HWND hwndOwner, LPBC pbc, LPOLESTR lpszDisplayName,
        DWORD * pchEaten, LPITEMIDLIST * ppidl, DWORD * pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;
    LPCWSTR szNext = NULL;
    WCHAR szElement[MAX_PATH];
    LPITEMIDLIST pidlTemp = NULL;
    CLSID clsid;

    TRACE("(%p)->(HWND=%p,%p,%p=%s,%p,pidl=%p,%p)\n", this,
          hwndOwner, pbc, lpszDisplayName, debugstr_w (lpszDisplayName),
          pchEaten, ppidl, pdwAttributes);

    *ppidl = 0;
    if (pchEaten)
        *pchEaten = 0;        /* strange but like the original */

    /* handle CLSID paths */
    if (lpszDisplayName[0] == ':' && lpszDisplayName[1] == ':')
    {
        szNext = GetNextElementW (lpszDisplayName, szElement, MAX_PATH);
        TRACE ("-- element: %s\n", debugstr_w (szElement));
        CLSIDFromString (szElement + 2, &clsid);
        pidlTemp = _ILCreateGuid (PT_GUID, clsid);
    }
    /* do we have an absolute path name ? */
    else if (PathGetDriveNumberW (lpszDisplayName) >= 0 &&
             lpszDisplayName[2] == (WCHAR) '\\')
    {
        szNext = GetNextElementW (lpszDisplayName, szElement, MAX_PATH);
        /* make drive letter uppercase to enable PIDL comparison */
        szElement[0] = toupper(szElement[0]);
        pidlTemp = _ILCreateDrive (szElement);
    }

    if (szNext && *szNext)
    {
        hr = SHELL32_ParseNextElement (this, hwndOwner, pbc, &pidlTemp,
                                       (LPOLESTR) szNext, pchEaten, pdwAttributes);
    }
    else
    {
        if (pdwAttributes && *pdwAttributes)
            SHELL32_GetItemAttributes (this,
                                       pidlTemp, pdwAttributes);
        hr = S_OK;
    }

    *ppidl = pidlTemp;

    TRACE ("(%p)->(-- ret=0x%08x)\n", this, hr);

    return hr;
}

/**************************************************************************
*        CDrivesFolder::EnumObjects
*/
HRESULT WINAPI CDrivesFolder::EnumObjects(HWND hwndOwner, DWORD dwFlags, LPENUMIDLIST *ppEnumIDList)
{
    CComObject<CDrivesFolderEnum> *theEnumerator;
    CComPtr<IEnumIDList>          result;
    HRESULT                       hResult;

    TRACE ("(%p)->(HWND=%p flags=0x%08x pplist=%p)\n", this, hwndOwner, dwFlags, ppEnumIDList);

    if (ppEnumIDList == NULL)
        return E_POINTER;

    *ppEnumIDList = NULL;
    ATLTRY(theEnumerator = new CComObject<CDrivesFolderEnum>);

    if (theEnumerator == NULL)
        return E_OUTOFMEMORY;

    hResult = theEnumerator->QueryInterface(IID_IEnumIDList, (void **)&result);
    if (FAILED(hResult))
    {
        delete theEnumerator;
        return hResult;
    }

    hResult = theEnumerator->Initialize(hwndOwner, dwFlags);
    if (FAILED(hResult))
        return hResult;
    *ppEnumIDList = result.Detach();

    TRACE("-- (%p)->(new ID List: %p)\n", this, *ppEnumIDList);

    return S_OK;
}

/**************************************************************************
*        CDrivesFolder::BindToObject
*/
HRESULT WINAPI CDrivesFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID *ppvOut)
{
    TRACE("(%p)->(pidl=%p,%p,%s,%p)\n", this,
          pidl, pbcReserved, shdebugstr_guid(&riid), ppvOut);

    return SHELL32_BindToChild(pidlRoot, NULL, pidl, riid, ppvOut);
}

/**************************************************************************
*    CDrivesFolder::BindToStorage
*/
HRESULT WINAPI CDrivesFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID *ppvOut)
{
    FIXME("(%p)->(pidl=%p,%p,%s,%p) stub\n", this,
          pidl, pbcReserved, shdebugstr_guid (&riid), ppvOut);

    *ppvOut = NULL;
    return E_NOTIMPL;
}

/**************************************************************************
*     CDrivesFolder::CompareIDs
*/

HRESULT WINAPI CDrivesFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int nReturn;

    TRACE("(%p)->(0x%08lx,pidl1=%p,pidl2=%p)\n", this, lParam, pidl1, pidl2);
    nReturn = SHELL32_CompareIDs (this, lParam, pidl1, pidl2);
    TRACE("-- %i\n", nReturn);
    return nReturn;
}

/**************************************************************************
*    CDrivesFolder::CreateViewObject
*/
HRESULT WINAPI CDrivesFolder::CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvOut)
{
    LPSHELLVIEW pShellView;
    HRESULT hr = E_INVALIDARG;

    TRACE("(%p)->(hwnd=%p,%s,%p)\n", this,
          hwndOwner, shdebugstr_guid (&riid), ppvOut);

    if (!ppvOut)
        return hr;

    *ppvOut = NULL;

    if (IsEqualIID(riid, IID_IDropTarget))
    {
        WARN("IDropTarget not implemented\n");
        hr = E_NOTIMPL;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        WARN("IContextMenu not implemented\n");
        hr = E_NOTIMPL;
    }
    else if (IsEqualIID(riid, IID_IShellView))
    {
        hr = IShellView_Constructor ((IShellFolder *)this, &pShellView);
        if (pShellView)
        {
            hr = pShellView->QueryInterface(riid, ppvOut);
            pShellView->Release();
        }
    }
    TRACE ("-- (%p)->(interface=%p)\n", this, ppvOut);
    return hr;
}

/**************************************************************************
*  CDrivesFolder::GetAttributesOf
*/
HRESULT WINAPI CDrivesFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, DWORD * rgfInOut)
{
    static const DWORD dwComputerAttributes =
        SFGAO_CANRENAME | SFGAO_CANDELETE | SFGAO_HASPROPSHEET | SFGAO_DROPTARGET |
        SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_CANLINK;
    static const DWORD dwControlPanelAttributes =
        SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_CANLINK;
    static const DWORD dwDriveAttributes =
        SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_FILESYSANCESTOR |
        SFGAO_DROPTARGET | SFGAO_HASPROPSHEET | SFGAO_CANRENAME | SFGAO_CANLINK;

    TRACE ("(%p)->(cidl=%d apidl=%p mask=%p (0x%08x))\n",
           this, cidl, apidl, rgfInOut, rgfInOut ? *rgfInOut : 0);

    if (cidl && !apidl)
        return E_INVALIDARG;

    if (*rgfInOut == 0)
        *rgfInOut = ~0;

    /* FIXME: always add SFGAO_CANLINK */
    if(cidl == 0)
        *rgfInOut &= dwComputerAttributes;
    else
    {
        for (UINT i = 0; i < cidl; ++i)
        {
            if (_ILIsDrive(apidl[i]))
                *rgfInOut &= dwDriveAttributes;
            else if (_ILIsControlPanel(apidl[i]))
                *rgfInOut &= dwControlPanelAttributes;
            else
            {
                pdump(apidl[i]);
                SHELL32_GetItemAttributes(this, apidl[i], rgfInOut);
            }
        }
    }

    /* make sure SFGAO_VALIDATE is cleared, some apps depend on that */
    *rgfInOut &= ~SFGAO_VALIDATE;

    TRACE ("-- result=0x%08x\n", *rgfInOut);
    return S_OK;
}

/**************************************************************************
*    CDrivesFolder::GetUIObjectOf
*
* PARAMETERS
*  hwndOwner [in]  Parent window for any output
*  cidl      [in]  array size
*  apidl     [in]  simple pidl array
*  riid      [in]  Requested Interface
*  prgfInOut [   ] reserved
*  ppvObject [out] Resulting Interface
*
*/
HRESULT WINAPI CDrivesFolder::GetUIObjectOf(HWND hwndOwner,
    UINT cidl, LPCITEMIDLIST *apidl,
    REFIID riid, UINT *prgfInOut, LPVOID *ppvOut)
{
    LPITEMIDLIST pidl;
    IUnknown *pObj = NULL;
    HRESULT hr = E_INVALIDARG;

    TRACE("(%p)->(%p,%u,apidl=%p,%s,%p,%p)\n", this,
          hwndOwner, cidl, apidl, shdebugstr_guid (&riid), prgfInOut, ppvOut);

    if (!ppvOut)
        return hr;

    *ppvOut = NULL;

    if (IsEqualIID (riid, IID_IContextMenu) && (cidl >= 1))
    {
        hr = CDefFolderMenu_Create2(pidlRoot, hwndOwner, cidl, apidl, (IShellFolder*)this, NULL, 0, NULL, (IContextMenu**)&pObj);
    }
    else if (IsEqualIID (riid, IID_IDataObject) && (cidl >= 1))
    {
        hr = IDataObject_Constructor (hwndOwner,
                                      pidlRoot, apidl, cidl, (IDataObject **)&pObj);
    }
    else if (IsEqualIID (riid, IID_IExtractIconA) && (cidl == 1))
    {
        pidl = ILCombine (pidlRoot, apidl[0]);
        pObj = (LPUNKNOWN) IExtractIconA_Constructor (pidl);
        SHFree (pidl);
        hr = S_OK;
    }
    else if (IsEqualIID (riid, IID_IExtractIconW) && (cidl == 1))
    {
        pidl = ILCombine (pidlRoot, apidl[0]);
        pObj = (LPUNKNOWN) IExtractIconW_Constructor (pidl);
        SHFree (pidl);
        hr = S_OK;
    }
    else if (IsEqualIID (riid, IID_IDropTarget) && (cidl >= 1))
    {
        hr = this->QueryInterface(IID_IDropTarget,
                                  (LPVOID *) &pObj);
    }
    else if ((IsEqualIID(riid, IID_IShellLinkW) ||
              IsEqualIID(riid, IID_IShellLinkA)) && (cidl == 1))
    {
        pidl = ILCombine (pidlRoot, apidl[0]);
        hr = IShellLink_ConstructFromFile(NULL, riid, pidl, (LPVOID*) &pObj);
        SHFree (pidl);
    }
    else
        hr = E_NOINTERFACE;

    if (SUCCEEDED(hr) && !pObj)
        hr = E_OUTOFMEMORY;

    *ppvOut = pObj;
    TRACE ("(%p)->hr=0x%08x\n", this, hr);
    return hr;
}

/**************************************************************************
*    CDrivesFolder::GetDisplayNameOf
*/
HRESULT WINAPI CDrivesFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, LPSTRRET strRet)
{
    LPWSTR pszPath;
    HRESULT hr = S_OK;

    TRACE ("(%p)->(pidl=%p,0x%08x,%p)\n", this, pidl, dwFlags, strRet);
    pdump (pidl);

    if (!strRet)
        return E_INVALIDARG;

    pszPath = (LPWSTR)CoTaskMemAlloc((MAX_PATH + 1) * sizeof(WCHAR));
    if (!pszPath)
        return E_OUTOFMEMORY;

    pszPath[0] = 0;

    if (!pidl->mkid.cb)
    {
        /* parsing name like ::{...} */
        pszPath[0] = ':';
        pszPath[1] = ':';
        SHELL32_GUIDToStringW(CLSID_MyComputer, &pszPath[2]);
    }
    else if (_ILIsPidlSimple(pidl))
    {
        /* take names of special folders only if its only this folder */
        if (_ILIsSpecialFolder(pidl))
        {
            GUID const *clsid;

            clsid = _ILGetGUIDPointer (pidl);
            if (clsid)
            {
                if (GET_SHGDN_FOR (dwFlags) & SHGDN_FORPARSING)
                {
                    static const WCHAR clsidW[] = L"CLSID\\";
                    static const WCHAR shellfolderW[] = L"\\shellfolder";
                    static const WCHAR wantsForParsingW[] = L"WantsForParsing";
                    BOOL bWantsForParsing = FALSE;
                    WCHAR szRegPath[100];
                    LONG r;

                    /*
                     * We can only get a filesystem path from a shellfolder
                     * if the value WantsFORPARSING exists in
                     *      CLSID\\{...}\\shellfolder
                     * exception: the MyComputer folder has this keys not
                     *            but like any filesystem backed
                     *            folder it needs these behaviour
                     *
                     * Get the "WantsFORPARSING" flag from the registry
                     */

                    wcscpy(szRegPath, clsidW);
                    SHELL32_GUIDToStringW(*clsid, &szRegPath[6]);
                    wcscat(szRegPath, shellfolderW);
                    r = SHGetValueW(HKEY_CLASSES_ROOT, szRegPath,
                                    wantsForParsingW, NULL, NULL, NULL);
                    if (r == ERROR_SUCCESS)
                        bWantsForParsing = TRUE;

                    if ((GET_SHGDN_RELATION (dwFlags) == SHGDN_NORMAL) &&
                            bWantsForParsing)
                    {
                        /*
                         * We need the filesystem path to the destination folder
                         * Only the folder itself can know it
                         */
                        hr = SHELL32_GetDisplayNameOfChild (this, pidl,
                                                            dwFlags, pszPath, MAX_PATH);
                    }
                    else
                    {
                        LPWSTR p = pszPath;

                        /* parsing name like ::{...} */
                        p[0] = ':';
                        p[1] = ':';
                        p += 2;
                        p += SHELL32_GUIDToStringW(CLSID_MyComputer, p);

                        /* \:: */
                        p[0] = '\\';
                        p[1] = ':';
                        p[2] = ':';
                        p += 3;
                        SHELL32_GUIDToStringW(*clsid, p);
                    }
                }
                else
                {
                    /* user friendly name */

                    if (_ILIsMyComputer(pidl) && sName)
                        wcscpy(pszPath, sName);
                    else
                        HCR_GetClassNameW (*clsid, pszPath, MAX_PATH);

                    TRACE("pszPath %s\n", debugstr_w(pszPath));
                }
            }
            else
            {
                /* append my own path */
                _ILSimpleGetTextW(pidl, pszPath, MAX_PATH);
            }
        }
        else if (_ILIsDrive(pidl))
        {

            _ILSimpleGetTextW(pidl, pszPath, MAX_PATH);    /* append my own path */
            /* long view "lw_name (C:)" */
            if (!(dwFlags & SHGDN_FORPARSING))
            {
                WCHAR wszDrive[18] = {0};
                DWORD dwVolumeSerialNumber, dwMaximumComponentLength, dwFileSystemFlags;
                static const WCHAR wszOpenBracket[] = {' ', '(', 0};
                static const WCHAR wszCloseBracket[] = {')', 0};

                lstrcpynW(wszDrive, pszPath, 4);
                pszPath[0] = L'\0';
                GetVolumeInformationW(wszDrive, pszPath,
                                      MAX_PATH - 7,
                                      &dwVolumeSerialNumber,
                                      &dwMaximumComponentLength, &dwFileSystemFlags, NULL, 0);
                pszPath[MAX_PATH-1] = L'\0';
                if (!wcslen(pszPath))
                {
                    UINT DriveType, ResourceId;
                    DriveType = GetDriveTypeW(wszDrive);
                    switch(DriveType)
                    {
                        case DRIVE_FIXED:
                            ResourceId = IDS_DRIVE_FIXED;
                            break;
                        case DRIVE_REMOTE:
                            ResourceId = IDS_DRIVE_NETWORK;
                            break;
                        case DRIVE_CDROM:
                            ResourceId = IDS_DRIVE_CDROM;
                            break;
                        default:
                            ResourceId = 0;
                    }
                    if (ResourceId)
                    {
                        dwFileSystemFlags = LoadStringW(shell32_hInstance, ResourceId, pszPath, MAX_PATH);
                        if (dwFileSystemFlags > MAX_PATH - 7)
                            pszPath[MAX_PATH-7] = L'\0';
                    }
                }
                wcscat (pszPath, wszOpenBracket);
                wszDrive[2] = L'\0';
                wcscat (pszPath, wszDrive);
                wcscat (pszPath, wszCloseBracket);
            }
        }
        else
        {
            /* Neither a shell namespace extension nor a drive letter. */
            ERR("Wrong pidl type\n");
            CoTaskMemFree(pszPath);
            return E_INVALIDARG;
        }
    }
    else
    {
        /* Complex pidl. Let the child folder do the work */
        hr = SHELL32_GetDisplayNameOfChild(this, pidl, dwFlags, pszPath, MAX_PATH);
    }

    if (SUCCEEDED(hr))
    {
        strRet->uType = STRRET_WSTR;
        strRet->pOleStr = pszPath;
    }
    else
        CoTaskMemFree(pszPath);

    TRACE("-- (%p)->(%s)\n", this, strRet->uType == STRRET_CSTR ? strRet->cStr : debugstr_w(strRet->pOleStr));
    return hr;
}

/**************************************************************************
*  CDrivesFolder::SetNameOf
*  Changes the name of a file object or subfolder, possibly changing its item
*  identifier in the process.
*
* PARAMETERS
*  hwndOwner  [in]   Owner window for output
*  pidl       [in]   simple pidl of item to change
*  lpszName   [in]   the items new display name
*  dwFlags    [in]   SHGNO formatting flags
*  ppidlOut   [out]  simple pidl returned
*/
HRESULT WINAPI CDrivesFolder::SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl,
                                        LPCOLESTR lpName, DWORD dwFlags, LPITEMIDLIST *pPidlOut)
{
    LPWSTR sName;
    HKEY hKey;
    UINT length;
    WCHAR szName[30];

    TRACE("(%p)->(%p,pidl=%p,%s,%u,%p)\n", this,
          hwndOwner, pidl, debugstr_w (lpName), dwFlags, pPidlOut);

    if (_ILIsDrive(pidl))
    {
        if (_ILSimpleGetTextW(pidl, szName, _countof(szName)))
            SetVolumeLabelW(szName, lpName);
        if (pPidlOut)
            *pPidlOut = _ILCreateDrive(szName);
        return S_OK;
    }


    if (pPidlOut != NULL)
        *pPidlOut = _ILCreateMyComputer();

    length = (wcslen(lpName) + 1) * sizeof(WCHAR);
    sName = (LPWSTR)SHAlloc(length);

    if (!sName)
        return E_OUTOFMEMORY;

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}",
                      0,
                      KEY_WRITE,
                      &hKey) != ERROR_SUCCESS)
    {
        WARN("Error: failed to open registry key\n");
    }
    else
    {
        RegSetValueExW(hKey, NULL, 0, REG_SZ, (const LPBYTE)lpName, length);
        RegCloseKey(hKey);
    }

    wcscpy(sName, lpName);
    SHFree(this->sName);
    this->sName = sName;
    TRACE("result %s\n", debugstr_w(sName));
    return S_OK;
}

HRESULT WINAPI CDrivesFolder::GetDefaultSearchGUID(GUID * pguid)
{
    FIXME ("(%p)\n", this);
    return E_NOTIMPL;
}

HRESULT WINAPI CDrivesFolder::EnumSearches(IEnumExtraSearch ** ppenum)
{
    FIXME ("(%p)\n", this);
    return E_NOTIMPL;
}

HRESULT WINAPI CDrivesFolder::GetDefaultColumn (DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    TRACE ("(%p)\n", this);

    if (pSort)
        *pSort = 0;
    if (pDisplay)
        *pDisplay = 0;
    return S_OK;
}

HRESULT WINAPI CDrivesFolder::GetDefaultColumnState(UINT iColumn, DWORD * pcsFlags)
{
    TRACE ("(%p)\n", this);

    if (!pcsFlags || iColumn >= MYCOMPUTERSHELLVIEWCOLUMNS)
        return E_INVALIDARG;
    *pcsFlags = MyComputerSFHeader[iColumn].pcsFlags;
    return S_OK;
}

HRESULT WINAPI CDrivesFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID * pscid, VARIANT * pv)
{
    FIXME ("(%p)\n", this);
    return E_NOTIMPL;
}

/* FIXME: drive size >4GB is rolling over */
HRESULT WINAPI CDrivesFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *psd)
{
    HRESULT hr;

    TRACE ("(%p)->(%p %i %p)\n", this, pidl, iColumn, psd);

    if (!psd || iColumn >= MYCOMPUTERSHELLVIEWCOLUMNS)
        return E_INVALIDARG;

    if (!pidl)
    {
        psd->fmt = MyComputerSFHeader[iColumn].fmt;
        psd->cxChar = MyComputerSFHeader[iColumn].cxChar;
        psd->str.uType = STRRET_CSTR;
        LoadStringA(shell32_hInstance, MyComputerSFHeader[iColumn].colnameid,
                    psd->str.cStr, MAX_PATH);
        return S_OK;
    }
    else
    {
        char szPath[MAX_PATH];
        ULARGE_INTEGER ulBytes;

        psd->str.cStr[0] = 0x00;
        psd->str.uType = STRRET_CSTR;
        switch (iColumn)
        {
            case 0:        /* name */
                hr = GetDisplayNameOf(pidl,
                                      SHGDN_NORMAL | SHGDN_INFOLDER, &psd->str);
                break;
            case 1:        /* type */
                _ILGetFileType(pidl, psd->str.cStr, MAX_PATH);
                break;
            case 2:        /* total size */
                if (_ILIsDrive(pidl))
                {
                    _ILSimpleGetText (pidl, szPath, MAX_PATH);
                    GetDiskFreeSpaceExA (szPath, NULL, &ulBytes, NULL);
                    StrFormatByteSizeA (ulBytes.LowPart, psd->str.cStr, MAX_PATH);
                }
                break;
            case 3:        /* free size */
                if (_ILIsDrive(pidl))
                {
                    _ILSimpleGetText (pidl, szPath, MAX_PATH);
                    GetDiskFreeSpaceExA (szPath, &ulBytes, NULL, NULL);
                    StrFormatByteSizeA (ulBytes.LowPart, psd->str.cStr, MAX_PATH);
                }
                break;
        }
        hr = S_OK;
    }

    return hr;
}

HRESULT WINAPI CDrivesFolder::MapColumnToSCID(UINT column, SHCOLUMNID * pscid)
{
    FIXME("(%p)\n", this);
    return E_NOTIMPL;
}

/************************************************************************
 *    CDrivesFolder::GetClassID
 */
HRESULT WINAPI CDrivesFolder::GetClassID(CLSID *lpClassId)
{
    TRACE ("(%p)\n", this);

    if (!lpClassId)
        return E_POINTER;

    *lpClassId = CLSID_MyComputer;
    return S_OK;
}

/************************************************************************
 *    CDrivesFolder::Initialize
 *
 * NOTES: it makes no sense to change the pidl
 */
HRESULT WINAPI CDrivesFolder::Initialize(LPCITEMIDLIST pidl)
{
    TRACE ("(%p)->(%p)\n", this, pidl);

    if (pidlRoot)
        SHFree((LPVOID)pidlRoot);

    pidlRoot = ILClone(pidl);
    return S_OK;
}

/**************************************************************************
 *    CDrivesFolder::GetCurFolder
 */
HRESULT WINAPI CDrivesFolder::GetCurFolder(LPITEMIDLIST *pidl)
{
    TRACE("(%p)->(%p)\n", this, pidl);

    if (!pidl)
        return E_POINTER;

    *pidl = ILClone(pidlRoot);
    return S_OK;
}
