/*
 * provides new shell item service
 *
 * Copyright 2007 Johannes Anderwald (johannes.anderwald@reactos.org)
 * Copyright 2009 Andrew Hill
 * Copyright 2012 Rafal Harabien
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

#include "precomp.h"

WINE_DEFAULT_DEBUG_CHANNEL(shell);

CNewMenu::CNewMenu()
{
    m_wszPath = NULL;
    m_pItems = NULL;
    m_pLinkItem = NULL;
    m_pSite = NULL;
    m_hbmFolder = NULL;
    m_hbmLink = NULL;
}

CNewMenu::~CNewMenu()
{
    UnloadAllItems();
    if (m_hbmFolder)
        DeleteObject(m_hbmFolder);
    if (m_hbmLink)
        DeleteObject(m_hbmLink);
}

void CNewMenu::UnloadItem(SHELLNEW_ITEM *pItem)
{
    /* Note: free allows NULL as argument */
    free(pItem->pData);
    free(pItem->pwszDesc);
    free(pItem->pwszExt);

    if (pItem->hBitmap)
        DeleteObject(pItem->hBitmap);

    HeapFree(GetProcessHeap(), 0, pItem);
}

void CNewMenu::UnloadAllItems()
{
    SHELLNEW_ITEM *pCurItem;

    /* Unload normal items */
    while (m_pItems)
    {
        pCurItem = m_pItems;
        m_pItems = m_pItems->pNext;

        UnloadItem(pCurItem);
    }

    /* Unload link item */
    if (m_pLinkItem)
        UnloadItem(m_pLinkItem);
    m_pLinkItem = NULL;
}

static HBITMAP IconToBitmap(HICON hIcon)
{
    HDC hdc, hdcScr;
    HBITMAP hbm, hbmOld;
    RECT rc;

    hdcScr = GetDC(NULL);
    hdc = CreateCompatibleDC(hdcScr);
    SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXMENUCHECK), GetSystemMetrics(SM_CYMENUCHECK));
    hbm = CreateCompatibleBitmap(hdcScr, rc.right, rc.bottom);
    ReleaseDC(NULL, hdcScr);

    hbmOld = (HBITMAP)SelectObject(hdc, hbm);
    FillRect(hdc, &rc, (HBRUSH)(COLOR_MENU + 1));
    if (!DrawIconEx(hdc, 0, 0, hIcon, rc.right, rc.bottom, 0, NULL, DI_NORMAL))
        ERR("DrawIcon failed: %x\n", GetLastError());
    SelectObject(hdc, hbmOld);

    DeleteDC(hdc);

    return hbm;
}

CNewMenu::SHELLNEW_ITEM *CNewMenu::LoadItem(LPCWSTR pwszExt)
{
    HKEY hKey;
    WCHAR wszBuf[MAX_PATH];
    BYTE *pData = NULL;
    DWORD cbData;

    StringCbPrintfW(wszBuf, sizeof(wszBuf), L"%s\\ShellNew", pwszExt);

    TRACE("LoadItem Keyname %s Name %s\n", debugstr_w(pwszExt), debugstr_w(wszBuf));

    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, wszBuf, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        TRACE("Failed to open key\n");
        return NULL;
    }

    /* Find first valid value */
    struct
    {
        LPCWSTR pszName;
        SHELLNEW_TYPE Type;
        BOOL bNeedData;
        BOOL bStr;
    } Types[] = {
        {L"FileName", SHELLNEW_TYPE_FILENAME, TRUE, TRUE},
        {L"Command", SHELLNEW_TYPE_COMMAND, TRUE, TRUE},
        {L"Data", SHELLNEW_TYPE_DATA, TRUE, FALSE},
        {L"NullFile", SHELLNEW_TYPE_NULLFILE, FALSE},
        {NULL}
    };
    UINT i;

    for (i = 0; Types[i].pszName; ++i)
    {
        /* Note: We are using ANSI function because strings can be treated as data */
        cbData = 0;
        DWORD dwFlags = Types[i].bStr ? RRF_RT_REG_SZ : RRF_RT_ANY;
        DWORD dwType;
        if (RegGetValueW(hKey, NULL, Types[i].pszName, dwFlags, NULL, NULL, &cbData) == ERROR_SUCCESS)
        {
            if (Types[i].bNeedData && cbData > 0)
            {
                pData = (BYTE*)malloc(cbData);
                RegGetValueW(hKey, NULL, Types[i].pszName, dwFlags, &dwType, pData, &cbData);
                if (!Types[i].bStr && (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
                {
                    PBYTE pData2 = (PBYTE)malloc(cbData);
                    cbData = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pData, -1, (LPSTR)pData2, cbData, NULL, NULL);
                    free(pData);
                    pData = pData2;
                }
            }
            break;
        }
    }
    RegCloseKey(hKey);

    /* Was any key found? */
    if (!Types[i].pszName)
        return NULL;

    SHFILEINFO fi;
    if (!SHGetFileInfoW(pwszExt, FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME|SHGFI_ICON|SHGFI_SMALLICON))
        return NULL;

    /* Create new item */
    SHELLNEW_ITEM *pNewItem = static_cast<SHELLNEW_ITEM *>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SHELLNEW_ITEM)));
    if (!pNewItem)
    {
        free(pData);
        return NULL;
    }

    TRACE("new item %ls\n", fi.szTypeName);
    pNewItem->Type = Types[i].Type;
    pNewItem->pData = pData;
    pNewItem->cbData = pData ? cbData : 0;
    pNewItem->pwszExt = _wcsdup(pwszExt);
    pNewItem->pwszDesc = _wcsdup(fi.szTypeName);
    if (fi.hIcon)
        pNewItem->hBitmap = IconToBitmap(fi.hIcon);

    return pNewItem;
}

BOOL
CNewMenu::LoadAllItems()
{
    DWORD dwIndex = 0;
    WCHAR wszName[MAX_PATH];
    SHELLNEW_ITEM *pNewItem;
    SHELLNEW_ITEM *pCurItem = NULL;

    /* If there are any unload them */
    UnloadAllItems();

    /* Enumerate all extesions */
    while (RegEnumKeyW(HKEY_CLASSES_ROOT, dwIndex++, wszName, _countof(wszName)) == ERROR_SUCCESS)
    {
        if (wszName[0] != L'.')
            continue;

        pNewItem = LoadItem(wszName);
        if (pNewItem)
        {
            if (wcsicmp(pNewItem->pwszExt, L".lnk") == 0)
            {
                /* Link handler */
                m_pLinkItem = pNewItem;
            }
            else
            {
                /* Add at the end of list */
                if (pCurItem)
                {
                    pCurItem->pNext = pNewItem;
                    pCurItem = pNewItem;
                }
                else
                    pCurItem = m_pItems = pNewItem;
            }
        }
    }

    if (!m_pLinkItem)
    {
        m_pLinkItem = static_cast<SHELLNEW_ITEM *>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SHELLNEW_ITEM)));
        if (m_pLinkItem)
        {
            m_pLinkItem->Type = SHELLNEW_TYPE_NULLFILE;
            m_pLinkItem->pwszDesc = _wcsdup(L"Link");
            m_pLinkItem->pwszExt = _wcsdup(L".lnk");
        }
    }

    if (m_pItems == NULL)
        return FALSE;
    else
        return TRUE;
}

UINT
CNewMenu::InsertShellNewItems(HMENU hMenu, UINT idCmdFirst, UINT Pos)
{
    MENUITEMINFOW mii;
    WCHAR wszBuf[256];
    UINT idCmd = idCmdFirst;

    if (m_pItems == NULL)
    {
        if (!LoadAllItems())
            return 0;
    }

    ZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    /* Insert new folder action */
    if (!LoadStringW(shell32_hInstance, FCIDM_SHVIEW_NEWFOLDER, wszBuf, _countof(wszBuf)))
        wszBuf[0] = 0;
    mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
    mii.fType = MFT_STRING;
    mii.dwTypeData = wszBuf;
    mii.cch = wcslen(mii.dwTypeData);
    mii.wID = idCmd;
    if (m_hbmFolder)
    {
        mii.fMask |= MIIM_CHECKMARKS;
        mii.hbmpChecked = mii.hbmpUnchecked = m_hbmFolder;
    }
    if (InsertMenuItemW(hMenu, Pos++, TRUE, &mii))
        ++idCmd;

    /* Insert new shortcut action */
    if (!LoadStringW(shell32_hInstance, FCIDM_SHVIEW_NEWLINK, wszBuf, _countof(wszBuf)))
        wszBuf[0] = 0;
    mii.dwTypeData = wszBuf;
    mii.cch = wcslen(mii.dwTypeData);
    mii.wID = idCmd;
    if (m_hbmLink)
    {
        mii.fMask |= MIIM_CHECKMARKS;
        mii.hbmpChecked = mii.hbmpUnchecked = m_hbmLink;
    }
    if (InsertMenuItemW(hMenu, Pos++, TRUE, &mii))
        ++idCmd;

    /* Insert seperator for custom new action */
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.fType = MFT_SEPARATOR;
    mii.wID = -1;
    InsertMenuItemW(hMenu, Pos++, TRUE, &mii);

    /* Insert rest of items */
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;

    SHELLNEW_ITEM *pCurItem = m_pItems;
    while (pCurItem)
    {
        TRACE("szDesc %s\n", debugstr_w(pCurItem->pwszDesc));
        mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
        mii.dwTypeData = pCurItem->pwszDesc;
        mii.cch = wcslen(mii.dwTypeData);
        mii.wID = idCmd;
        if (pCurItem->hBitmap)
        {
            mii.fMask |= MIIM_CHECKMARKS;
            mii.hbmpChecked = mii.hbmpUnchecked = pCurItem->hBitmap;
        }
        if (InsertMenuItemW(hMenu, Pos++, TRUE, &mii))
            ++idCmd;
        pCurItem = pCurItem->pNext;
    }

    return idCmd - idCmdFirst;
}

CNewMenu::SHELLNEW_ITEM *CNewMenu::FindItemFromIdOffset(UINT IdOffset)
{
    if (IdOffset == 0)
        return NULL; /* Folder */

    if (IdOffset == 1)
        return m_pLinkItem; /* shortcut */

    /* Find shell new item */
    SHELLNEW_ITEM *pItem = m_pItems;
    for (UINT i = 2; pItem; ++i)
    {
        if (i == IdOffset)
            break;

        pItem = pItem->pNext;
    }

    return pItem;
}

HRESULT CNewMenu::CreateNewFolder(IShellView *psv)
{
    WCHAR wszName[MAX_PATH];
    CComPtr<ISFHelper> psfhlp; 
    CComPtr<IFolderView> pFolderView;
    CComPtr<IShellFolder> pParentFolder;
    HRESULT hr;

    //if (m_pSite == NULL)
    //    return E_FAIL;

    /* Get current folder */
    hr = IUnknown_QueryService(psv, SID_IFolderView, IID_PPV_ARG(IFolderView, &pFolderView));
    if (FAILED(hr))
        return hr;

    hr = pFolderView->GetFolder(IID_PPV_ARG(IShellFolder, &pParentFolder));
    if (FAILED(hr))
        return hr;

    hr = pParentFolder->QueryInterface(IID_PPV_ARG(ISFHelper, &psfhlp));
    if (FAILED(hr))
        return hr;

    LPITEMIDLIST pidl;

    /* Get unique name and create a folder */
    hr = psfhlp->GetUniqueName(wszName, _countof(wszName));
    if (hr != S_OK)
        return hr;
    hr = psfhlp->AddFolder(0, wszName, &pidl);
    if (hr != S_OK)
    {
        WCHAR wszBuf[256];
        StringCbPrintfW(wszBuf, sizeof(wszBuf), L"Cannot create folder: %s", wszName);
        MessageBoxW(NULL, wszBuf, L"Cannot create folder", MB_OK|MB_ICONERROR);
        return hr;
    }

    /* Do a labeledit */
    psv->Refresh();
    psv->SelectItem(pidl, SVSI_DESELECTOTHERS | SVSI_EDIT | SVSI_ENSUREVISIBLE |
                          SVSI_FOCUSED | SVSI_SELECT);

    SHFree(pidl);
    return S_OK;
}

HRESULT CNewMenu::CreateNewItem(SHELLNEW_ITEM *pItem, LPCMINVOKECOMMANDINFO lpcmi, IShellView *psv)
{
    LPITEMIDLIST pidl;
    STRRET strTemp;
    WCHAR wszBuf[MAX_PATH];
    WCHAR wszPath[MAX_PATH];
    CComPtr<IFolderView> pFolderView;
    CComPtr<IShellFolder> pParentFolder;
    CComPtr<IPersistFolder2> psf;
    HRESULT hr;

    /* Get current folder */
    hr = IUnknown_QueryService(psv, SID_IFolderView, IID_PPV_ARG(IFolderView, &pFolderView));
    if (FAILED(hr))
        return hr;

    hr = pFolderView->GetFolder(IID_PPV_ARG(IShellFolder, &pParentFolder));
    if (FAILED(hr))
        return hr;

    if (pParentFolder->QueryInterface(IID_PPV_ARG(IPersistFolder2, &psf)) != S_OK)
    {
        ERR("Failed to get interface IID_IPersistFolder2\n");
        return E_FAIL;
    }

    if (psf->GetCurFolder(&pidl) != S_OK)
    {
        ERR("IPersistFolder2_GetCurFolder failed\n");
        return E_FAIL;
    }

    /* Get folder path */
    if (pParentFolder == NULL || pParentFolder->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strTemp) != S_OK)
    {
        ERR("IShellFolder_GetDisplayNameOf failed\n");
        return E_FAIL;
    }
    StrRetToBufW(&strTemp, pidl, wszPath, _countof(wszPath));

    switch (pItem->Type)
    {
        case SHELLNEW_TYPE_COMMAND:
        {
            LPWSTR Ptr, pwszCmd;
            WCHAR wszTemp[MAX_PATH];
            STARTUPINFOW si;
            PROCESS_INFORMATION pi;

            if (!ExpandEnvironmentStringsW((LPWSTR)pItem->pData, wszBuf, MAX_PATH))
            {
                TRACE("ExpandEnvironmentStrings failed\n");
                break;
            }

            /* Expand command parameter, FIXME: there can be more modifiers */
            Ptr = wcsstr(wszBuf, L"%1");
            if (Ptr)
            {
                Ptr[1] = 's';
                StringCbPrintfW(wszTemp, sizeof(wszTemp), wszBuf, wszPath);
                pwszCmd = wszTemp;
            }
            else
                pwszCmd = wszBuf;

            /* Create process */
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            if (CreateProcessW(NULL, pwszCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } else
                ERR("Failed to create process\n");
            break;
        }
        case SHELLNEW_TYPE_DATA:
        case SHELLNEW_TYPE_FILENAME:
        case SHELLNEW_TYPE_NULLFILE:
        {
            BOOL bSuccess = TRUE;
            LPWSTR pwszFilename = NULL;
            size_t cchFilenameMax = 0;

            /* Build new file name */
            LoadStringW(shell32_hInstance, FCIDM_SHVIEW_NEW, wszBuf, _countof(wszBuf));
            StringCchCatExW(wszPath, _countof(wszPath), L"\\", &pwszFilename, &cchFilenameMax, 0);
            StringCchPrintfW(pwszFilename, cchFilenameMax, L"%s %s%s", wszBuf, pItem->pwszDesc, pItem->pwszExt);

            /* Find unique name */
            for (UINT i = 2; PathFileExistsW(wszPath); ++i)
            {
                StringCchPrintfW(pwszFilename, cchFilenameMax, L"%s %s (%u)%s", wszBuf, pItem->pwszDesc, i, pItem->pwszExt);
                TRACE("New Filename %ls\n", pwszFilename);
            }

            /* Create new file */
            HANDLE hFile = CreateFileW(wszPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                if (pItem->Type == SHELLNEW_TYPE_DATA)
                {
                    /* Write a content */
                    DWORD cbWritten;
                    WriteFile(hFile, pItem->pData, pItem->cbData, &cbWritten, NULL);
                }

                /* Close file now */
                CloseHandle(hFile);
            } else
                bSuccess = FALSE;

            if (pItem->Type == SHELLNEW_TYPE_FILENAME)
            {
                /* Copy file */
                if (!CopyFileW((LPWSTR)pItem->pData, wszPath, FALSE))
                    ERR("Copy file failed: %ls\n", (LPWSTR)pItem->pData);
            }

            /* Show message if we failed */
            if (bSuccess)
            {
                TRACE("Notifying fs %s\n", debugstr_w(wszPath));
                SHChangeNotify(SHCNE_CREATE, SHCNF_PATHW, (LPCVOID)wszPath, NULL);
                psv->Refresh();

                LPITEMIDLIST pidl;
                hr = _ILCreateFromPathW(wszPath, &pidl);
                if (SUCCEEDED(hr))
                {
                    psv->SelectItem(pidl, SVSI_DESELECTOTHERS|SVSI_EDIT|SVSI_ENSUREVISIBLE|SVSI_FOCUSED|SVSI_SELECT);
                    ILFree(pidl);
                }
            }
            else
            {
                StringCbPrintfW(wszBuf, sizeof(wszBuf), L"Cannot create file: %s", pwszFilename);
                MessageBoxW(NULL, wszBuf, L"Cannot create file", MB_OK|MB_ICONERROR); // FIXME
            }
            break;
        }
        case SHELLNEW_TYPE_INVALID:
            ERR("Invalid type\n");
            break;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CNewMenu::SetSite(IUnknown *pUnkSite)
{
    m_pSite = pUnkSite;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CNewMenu::GetSite(REFIID riid, void **ppvSite)
{
    if (ppvSite == NULL)
        return E_POINTER;
    *ppvSite = m_pSite;
    if (m_pSite != NULL)
        m_pSite->AddRef();
    return S_OK;
}

HRESULT
WINAPI
CNewMenu::QueryContextMenu(HMENU hMenu,
                           UINT indexMenu,
                           UINT idCmdFirst,
                           UINT idCmdLast,
                           UINT uFlags)
{
    WCHAR wszNew[200];
    MENUITEMINFOW mii;
    UINT cItems = 0;

    TRACE("%p %p %u %u %u %u\n", this,
          hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

    if (!LoadStringW(shell32_hInstance, FCIDM_SHVIEW_NEW, wszNew, _countof(wszNew)))
        return E_FAIL;

    m_hSubMenu = CreateMenu();
    if (!m_hSubMenu)
        return E_FAIL;

    cItems = InsertShellNewItems(m_hSubMenu, idCmdFirst, 0);

    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.wID = -1;
    mii.dwTypeData = wszNew;
    mii.cch = wcslen(mii.dwTypeData);
    mii.fState = MFS_ENABLED;
    mii.hSubMenu = m_hSubMenu;

    if (!InsertMenuItemW(hMenu, indexMenu, TRUE, &mii))
        return E_FAIL;

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, cItems);
}

HRESULT
WINAPI
CNewMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    IShellBrowser *lpSB = NULL;
    CComPtr<IShellView> lpSV = NULL;
    HRESULT hr = E_FAIL;

    /* Note: CWM_GETISHELLBROWSER returns shell browser without adding reference */
    lpSB = (LPSHELLBROWSER)SendMessageA(lpici->hwnd, CWM_GETISHELLBROWSER, 0, 0);
    if (lpSB)
        lpSB->QueryActiveShellView(&lpSV);

    if (LOWORD(lpici->lpVerb) == 0)
        hr = CreateNewFolder(lpSV);
    else
    {
        SHELLNEW_ITEM *pItem = FindItemFromIdOffset(LOWORD(lpici->lpVerb));
        if (pItem)
            hr = CreateNewItem(pItem, lpici, lpSV);
    }

    TRACE("CNewMenu::InvokeCommand %x\n", hr);
    return hr;
}

HRESULT
WINAPI
CNewMenu::GetCommandString(UINT_PTR idCmd,
                           UINT uType,
                           UINT *pwReserved,
                           LPSTR pszName,
                           UINT cchMax)
{
    FIXME("%p %lu %u %p %p %u\n", this,
          idCmd, uType, pwReserved, pszName, cchMax );

    return E_NOTIMPL;
}

HRESULT
WINAPI
CNewMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return S_OK;
}

HRESULT WINAPI
CNewMenu::Initialize(LPCITEMIDLIST pidlFolder,
                     IDataObject *pdtobj, HKEY hkeyProgID)
{
    /* Load folder and shortcut icons */
    HICON hIcon = (HICON)LoadImage(shell32_hInstance, MAKEINTRESOURCE(IDI_SHELL_FOLDER), IMAGE_ICON, 0, 0, LR_SHARED);
    m_hbmFolder = hIcon ? IconToBitmap(hIcon) : NULL;
    hIcon = (HICON)LoadImage(shell32_hInstance, MAKEINTRESOURCE(IDI_SHELL_SHORTCUT), IMAGE_ICON, 0, 0, LR_SHARED);
    m_hbmLink = hIcon ? IconToBitmap(hIcon) : NULL;

    return S_OK;
}
