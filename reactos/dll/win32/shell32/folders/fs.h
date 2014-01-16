/*
 * file system folder
 *
 * Copyright 1997             Marcus Meissner
 * Copyright 1998, 1999, 2002 Juergen Schmied
 * Copyright 2009             Andrew Hill
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

#ifndef _CFSFOLDER_H_
#define _CFSFOLDER_H_

WCHAR *BuildPathsList(LPCWSTR wszBasePath, int cidl, LPCITEMIDLIST *pidls);

class CFSFolder :
    public CComCoClass<CFSFolder, &CLSID_ShellFSFolder>,
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IShellFolder2,
    public IPersistFolder3,
    public IDropTarget,
    public ISFHelper
{
    private:
        CLSID *pclsid;

        /* both paths are parsible from the desktop */
        LPWSTR sPathTarget;     /* complete path to target used for enumeration and ChangeNotify */

        LPITEMIDLIST pidlRoot; /* absolute pidl */

        UINT cfShellIDList;    /* clipboardformat for IDropTarget */
        BOOL fAcceptFmt;       /* flag for pending Drop */
        BOOL QueryDrop (DWORD dwKeyState, LPDWORD pdwEffect);
        void SF_RegisterClipFmt();
        BOOL GetUniqueFileName(LPWSTR pwszBasePath, LPCWSTR pwszExt, LPWSTR pwszTarget, BOOL bShortcut);
        static DWORD _DoDropThreadProc(LPVOID lpParameter);
        virtual HRESULT WINAPI _DoDrop(IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);

    public:
        CFSFolder();
        ~CFSFolder();
        

        // IShellFolder
        virtual HRESULT WINAPI ParseDisplayName(HWND hwndOwner, LPBC pbc, LPOLESTR lpszDisplayName, DWORD *pchEaten, LPITEMIDLIST *ppidl, DWORD *pdwAttributes);
        virtual HRESULT WINAPI EnumObjects(HWND hwndOwner, DWORD dwFlags, LPENUMIDLIST *ppEnumIDList);
        virtual HRESULT WINAPI BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID *ppvOut);
        virtual HRESULT WINAPI BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID *ppvOut);
        virtual HRESULT WINAPI CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
        virtual HRESULT WINAPI CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID *ppvOut);
        virtual HRESULT WINAPI GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, DWORD *rgfInOut);
        virtual HRESULT WINAPI GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
        virtual HRESULT WINAPI GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, LPSTRRET strRet);
        virtual HRESULT WINAPI SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR lpName, DWORD dwFlags, LPITEMIDLIST *pPidlOut);

        /* ShellFolder2 */
        virtual HRESULT WINAPI GetDefaultSearchGUID(GUID *pguid);
        virtual HRESULT WINAPI EnumSearches(IEnumExtraSearch **ppenum);
        virtual HRESULT WINAPI GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
        virtual HRESULT WINAPI GetDefaultColumnState(UINT iColumn, DWORD *pcsFlags);
        virtual HRESULT WINAPI GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
        virtual HRESULT WINAPI GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *psd);
        virtual HRESULT WINAPI MapColumnToSCID(UINT column, SHCOLUMNID *pscid);

        // IPersist
        virtual HRESULT WINAPI GetClassID(CLSID *lpClassId);

        // IPersistFolder
        virtual HRESULT WINAPI Initialize(LPCITEMIDLIST pidl);

        // IPersistFolder2
        virtual HRESULT WINAPI GetCurFolder(LPITEMIDLIST * pidl);

        // IPersistFolder3
        virtual HRESULT WINAPI InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfti);
        virtual HRESULT WINAPI GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *ppfti);

        // IDropTarget
        virtual HRESULT WINAPI DragEnter(IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
        virtual HRESULT WINAPI DragOver(DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
        virtual HRESULT WINAPI DragLeave();
        virtual HRESULT WINAPI Drop(IDataObject *pDataObject, DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);

        // ISFHelper
        virtual HRESULT WINAPI GetUniqueName(LPWSTR pwszName, UINT uLen);
        virtual HRESULT WINAPI AddFolder(HWND hwnd, LPCWSTR pwszName, LPITEMIDLIST *ppidlOut);
        virtual HRESULT WINAPI DeleteItems(UINT cidl, LPCITEMIDLIST *apidl);
        virtual HRESULT WINAPI CopyItems(IShellFolder *pSFFrom, UINT cidl, LPCITEMIDLIST *apidl, bool bCopy);

        DECLARE_REGISTRY_RESOURCEID(IDR_SHELLFSFOLDER)
        DECLARE_NOT_AGGREGATABLE(CFSFolder)

        DECLARE_PROTECT_FINAL_CONSTRUCT()

        BEGIN_COM_MAP(CFSFolder)
        COM_INTERFACE_ENTRY_IID(IID_IShellFolder2, IShellFolder2)
        COM_INTERFACE_ENTRY_IID(IID_IShellFolder, IShellFolder)
        COM_INTERFACE_ENTRY_IID(IID_IPersistFolder, IPersistFolder)
        COM_INTERFACE_ENTRY_IID(IID_IPersistFolder2, IPersistFolder2)
        COM_INTERFACE_ENTRY_IID(IID_IPersistFolder3, IPersistFolder3)
        COM_INTERFACE_ENTRY_IID(IID_IPersist, IPersist)
        COM_INTERFACE_ENTRY_IID(IID_IDropTarget, IDropTarget)
        COM_INTERFACE_ENTRY_IID(IID_ISFHelper, ISFHelper)
        END_COM_MAP()
};

struct _DoDropData {
    CFSFolder *This;
    IDataObject *pDataObject;
    IAsyncOperation *pAsyncOperation;
    DWORD dwKeyState;
    POINTL pt; 
    DWORD pdwEffect;
};

#endif // _CFSFOLDER_H_
