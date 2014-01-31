/*
 * Provides default file shell extension
 *
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

#ifndef _FILE_DEF_EXT_H_
#define _FILE_DEF_EXT_H_

class CFileVersionInfo
{
    private:
        PVOID m_pInfo;
        WORD m_wLang, m_wCode;
        WCHAR m_wszLang[64];
        
        typedef struct _LANGANDCODEPAGE_
        {
            WORD wLang;
            WORD wCode;
        } LANGANDCODEPAGE, *LPLANGANDCODEPAGE;

    public:
        inline CFileVersionInfo():
            m_pInfo(NULL), m_wLang(0), m_wCode(0)
        {
            m_wszLang[0] = L'\0';
        }

        inline ~CFileVersionInfo()
        {
            if (m_pInfo)
                HeapFree(GetProcessHeap(), 0, m_pInfo);
        }

        BOOL Load(LPCWSTR pwszPath);
        LPCWSTR GetString(LPCWSTR pwszName);
        VS_FIXEDFILEINFO *GetFixedInfo();
        LPCWSTR GetLangName();
};

class CFileDefExt :
	public CComCoClass<CFileDefExt, &CLSID_ShellFileDefExt>,
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public IShellExtInit,
	public IContextMenu,
	public IShellPropSheetExt,
	public IObjectWithSite
{
private:
    VOID InitOpensWithField(HWND hwndDlg);
    BOOL InitFileType(HWND hwndDlg);
    BOOL InitFilePath(HWND hwndDlg);
    static BOOL GetFileTimeString(LPFILETIME lpFileTime, LPWSTR pwszResult, UINT cchResult);
    BOOL InitFileAttr(HWND hwndDlg);
    BOOL InitGeneralPage(HWND hwndDlg);
    BOOL SetVersionLabel(HWND hwndDlg, DWORD idCtrl, LPCWSTR pwszName);
    BOOL AddVersionString(HWND hwndDlg, LPCWSTR pwszName);
    BOOL InitVersionPage(HWND hwndDlg);
    static INT_PTR CALLBACK GeneralPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK VersionPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL CountFolderAndFiles(LPWSTR pwszBuf, UINT cchBufMax);

	WCHAR m_wszPath[MAX_PATH];
	CFileVersionInfo m_VerInfo;
	BOOL m_bDir;

	DWORD m_cFiles;
    DWORD m_cFolders;
    ULARGE_INTEGER m_DirSize;

public:
	CFileDefExt();
	~CFileDefExt();

	// IShellExtInit
	virtual HRESULT STDMETHODCALLTYPE Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

    // IContextMenu
	virtual HRESULT WINAPI QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	virtual HRESULT WINAPI InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
	virtual HRESULT WINAPI GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax);

	// IShellPropSheetExt
	virtual HRESULT WINAPI AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
	virtual HRESULT WINAPI ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplacePage, LPARAM lParam);

    // IObjectWithSite
	virtual HRESULT WINAPI SetSite(IUnknown *punk);
	virtual HRESULT WINAPI GetSite(REFIID iid, void **ppvSite);

DECLARE_REGISTRY_RESOURCEID(IDR_FILEDEFEXT)
DECLARE_NOT_AGGREGATABLE(CFileDefExt)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFileDefExt)
	COM_INTERFACE_ENTRY_IID(IID_IShellExtInit, IShellExtInit)
	COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
	COM_INTERFACE_ENTRY_IID(IID_IShellPropSheetExt, IShellPropSheetExt)
	COM_INTERFACE_ENTRY_IID(IID_IObjectWithSite, IObjectWithSite)
END_COM_MAP()
};

#endif /* _FILE_DEF_EXT_H_ */
