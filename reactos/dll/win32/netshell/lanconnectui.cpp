#include "precomp.h"

/// CLASSID
/// {7007ACC5-3202-11D1-AAD2-00805FC1270E}
/// open network properties and wlan properties

typedef enum
{
    NET_TYPE_CLIENT = 1,
    NET_TYPE_SERVICE = 2,
    NET_TYPE_PROTOCOL = 3
} NET_TYPE;

typedef struct
{
    NET_TYPE Type;
    DWORD dwCharacteristics;
    LPWSTR szHelp;
    INetCfgComponent  *pNCfgComp;
    UINT NumPropDialogOpen;
} NET_ITEM, *PNET_ITEM;

class CNetConnectionPropertyUi final :
    public INetConnectionPropertyUi2,
    public INetLanConnectionUiInfo,
    public INetConnectionConnectUi
{
    public:
        CNetConnectionPropertyUi();

        // IUnknown
        virtual HRESULT WINAPI QueryInterface(REFIID riid, LPVOID *ppvOut);
        virtual ULONG WINAPI AddRef();
        virtual ULONG WINAPI Release();

        // INetConnectionPropertyUi2
        virtual HRESULT WINAPI AddPages(HWND hwndParent, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
        virtual HRESULT WINAPI GetIcon(DWORD dwSize, HICON *phIcon);

        // INetLanConnectionUiInfo
        virtual HRESULT WINAPI GetDeviceGuid(GUID *pGuid);

        // INetConnectionConnectUi
        virtual HRESULT WINAPI SetConnection(INetConnection* pCon);
        virtual HRESULT WINAPI Connect(HWND hwndParent, DWORD dwFlags);
        virtual HRESULT WINAPI Disconnect(HWND hwndParent, DWORD dwFlags);

    private:
        BOOL GetINetCfgComponent(INetCfg *pNCfg, INetCfgComponent ** pOut);
        VOID EnumComponents(HWND hDlgCtrl, INetCfg *pNCfg, const GUID *CompGuid, UINT Type);
        VOID InitializeLANPropertiesUIDlg(HWND hwndDlg);
        VOID ShowNetworkComponentProperties(HWND hwndDlg);
        static INT_PTR CALLBACK LANPropertiesUIDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

        INetConnection * pCon;
        INetCfgLock *NCfgLock;
        INetCfg * pNCfg;
        NETCON_PROPERTIES * pProperties;
        LONG ref;
};

CNetConnectionPropertyUi::CNetConnectionPropertyUi()
{
    ref = 0;
    pCon = NULL;
    pNCfg = NULL;
    NCfgLock = NULL;
    pProperties = NULL;
}

HPROPSHEETPAGE
InitializePropertySheetPage(LPWSTR resname, DLGPROC dlgproc, LPARAM lParam, LPWSTR szTitle)
{
    PROPSHEETPAGEW ppage;

    memset(&ppage, 0x0, sizeof(PROPSHEETPAGEW));
    ppage.dwSize = sizeof(PROPSHEETPAGEW);
    ppage.dwFlags = PSP_DEFAULT;
    ppage.pszTemplate = resname;
    ppage.pfnDlgProc = dlgproc;
    ppage.lParam = lParam;
    ppage.hInstance = netshell_hInstance;
    if (szTitle)
    {
        ppage.dwFlags |= PSP_USETITLE;
        ppage.pszTitle = szTitle;
    }
    return CreatePropertySheetPageW(&ppage);
}

VOID
AddItemToListView(HWND hDlgCtrl, PNET_ITEM pItem, LPWSTR szName, BOOL bChecked)
{
    LVITEMW lvItem;

    ZeroMemory(&lvItem, sizeof(lvItem));
    lvItem.mask  = LVIF_TEXT | LVIF_PARAM;
    lvItem.pszText = szName;
    lvItem.lParam = (LPARAM)pItem;
    lvItem.iItem = ListView_GetItemCount(hDlgCtrl);
    lvItem.iItem = SendMessageW(hDlgCtrl, LVM_INSERTITEMW, 0, (LPARAM)&lvItem);
    ListView_SetCheckState(hDlgCtrl, lvItem.iItem, bChecked);
}

BOOL
CNetConnectionPropertyUi::GetINetCfgComponent(INetCfg *pNCfg, INetCfgComponent ** pOut)
{
    LPWSTR pName;
    HRESULT hr;
    INetCfgComponent * pNCg;
    ULONG Fetched;
    IEnumNetCfgComponent * pEnumCfg;

    hr = pNCfg->EnumComponents(&GUID_DEVCLASS_NET, &pEnumCfg);
    if (FAILED(hr))
    {
        return FALSE;
    }

    while (pEnumCfg->Next(1, &pNCg, &Fetched) == S_OK)
    {
       hr = pNCg->GetDisplayName(&pName);
       if (SUCCEEDED(hr))
       {
           if (!_wcsicmp(pName, pProperties->pszwDeviceName))
           {
               *pOut = pNCg;
               pEnumCfg->Release();
               return TRUE;
           }
           CoTaskMemFree(pName);
       }
       pNCg->Release();
    }
    pEnumCfg->Release();
    return FALSE;
}

VOID
CNetConnectionPropertyUi::EnumComponents(HWND hDlgCtrl, INetCfg *pNCfg, const GUID *CompGuid, UINT Type)
{
    HRESULT hr;
    IEnumNetCfgComponent * pENetCfg;
    INetCfgComponent  *pNCfgComp, *pAdapterCfgComp;
    INetCfgComponentBindings * pCompBind;
    ULONG Num;
    DWORD dwCharacteristics;
    LPOLESTR pDisplayName, pHelpText;
    PNET_ITEM pItem;
    BOOL bChecked;

    hr = pNCfg->EnumComponents(CompGuid, &pENetCfg);
    if (FAILED(hr))
    {
        pNCfg->Release();
        return;
    }
    while(pENetCfg->Next(1, &pNCfgComp, &Num) == S_OK)
    {
          hr = pNCfgComp->GetCharacteristics(&dwCharacteristics);
          if (SUCCEEDED(hr) && (dwCharacteristics & NCF_HIDDEN))
          {
              pNCfgComp->Release();
              continue;
          }
          pDisplayName = NULL;
          pHelpText = NULL;
          hr = pNCfgComp->GetDisplayName(&pDisplayName);
          hr = pNCfgComp->GetHelpText(&pHelpText);
          bChecked = TRUE; //ReactOS hack
          hr = pNCfgComp->QueryInterface(IID_INetCfgComponentBindings, (LPVOID*)&pCompBind);
          if (SUCCEEDED(hr))
          {
              if (GetINetCfgComponent(pNCfg, &pAdapterCfgComp))
              {
                  hr = pCompBind->IsBoundTo(pAdapterCfgComp);
                  if (hr == S_OK)
                      bChecked = TRUE;
                  else
                      bChecked = FALSE;
                  pAdapterCfgComp->Release();
                  pCompBind->Release();
              }
          }

          pItem = (NET_ITEM*)CoTaskMemAlloc(sizeof(NET_ITEM));
          if (!pItem)
              continue;

          pItem->dwCharacteristics = dwCharacteristics;
          pItem->szHelp = pHelpText;
          pItem->Type = (NET_TYPE)Type;
          pItem->pNCfgComp = pNCfgComp;
          pItem->NumPropDialogOpen = 0;

          AddItemToListView(hDlgCtrl, pItem, pDisplayName, bChecked);
          CoTaskMemFree(pDisplayName);
    }
    pENetCfg->Release();
}

VOID
CNetConnectionPropertyUi::InitializeLANPropertiesUIDlg(HWND hwndDlg)
{
    HRESULT hr;
    INetCfg *pNCfg;
    INetCfgLock *pNCfgLock;
    HWND hDlgCtrl = GetDlgItem(hwndDlg, IDC_COMPONENTSLIST);
    LVCOLUMNW lc;
    RECT rc;
    DWORD dwStyle;
    LPWSTR pDisplayName;
    LVITEMW li;

    SendDlgItemMessageW(hwndDlg, IDC_NETCARDNAME, WM_SETTEXT, 0, (LPARAM)pProperties->pszwDeviceName);
    if (pProperties->dwCharacter & NCCF_SHOW_ICON)
    {
        /* check show item on taskbar*/
        SendDlgItemMessageW(hwndDlg, IDC_SHOWTASKBAR, BM_SETCHECK, BST_CHECKED, 0);
    }
    if (pProperties->dwCharacter & NCCF_NOTIFY_DISCONNECTED)
    {
        /* check notify item */
        SendDlgItemMessageW(hwndDlg, IDC_NOTIFYNOCONNECTION, BM_SETCHECK, BST_CHECKED, 0);
    }

    memset(&lc, 0, sizeof(LV_COLUMN));
    lc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lc.fmt = LVCFMT_FIXED_WIDTH;
    if (GetClientRect(hDlgCtrl, &rc))
    {
        lc.mask |= LVCF_WIDTH;
        lc.cx = rc.right - rc.left;
    }
    lc.pszText = (LPWSTR)L"";
    (void)SendMessageW(hDlgCtrl, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);
    dwStyle = (DWORD) SendMessage(hDlgCtrl, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
    dwStyle = dwStyle | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
    SendMessage(hDlgCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);

    hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (LPVOID*)&pNCfg);
    if (FAILED(hr))
        return;

    hr = pNCfg->QueryInterface(IID_INetCfgLock, (LPVOID*)&pNCfgLock);
    hr = pNCfgLock->AcquireWriteLock(100, L"", &pDisplayName);
    if (hr == S_FALSE)
    {
        CoTaskMemFree(pDisplayName);
        return;
    }

    NCfgLock = pNCfgLock;
    hr = pNCfg->Initialize(NULL);
    if (FAILED(hr))
    {
        pNCfg->Release();
        return;
    }

    EnumComponents(hDlgCtrl, pNCfg, &GUID_DEVCLASS_NETCLIENT, NET_TYPE_CLIENT);
    EnumComponents(hDlgCtrl, pNCfg, &GUID_DEVCLASS_NETSERVICE, NET_TYPE_SERVICE);
    EnumComponents(hDlgCtrl, pNCfg, &GUID_DEVCLASS_NETTRANS, NET_TYPE_PROTOCOL);
    this->pNCfg = pNCfg;

    ZeroMemory(&li, sizeof(li));
    li.mask = LVIF_STATE;
    li.stateMask = (UINT)-1;
    li.state = LVIS_FOCUSED|LVIS_SELECTED;
    (void)SendMessageW(hDlgCtrl, LVM_SETITEMW, 0, (LPARAM)&li);
}

VOID
CNetConnectionPropertyUi::ShowNetworkComponentProperties(HWND hwndDlg)
{
    LVITEMW lvItem;
    HWND hDlgCtrl;
    UINT Index, Count;
    HRESULT hr;
    INetCfgComponent *pNCfgComp;
    PNET_ITEM pItem;

    hDlgCtrl = GetDlgItem(hwndDlg, IDC_COMPONENTSLIST);
    Count = ListView_GetItemCount(hDlgCtrl);
    if (!Count)
        return;

    ZeroMemory(&lvItem, sizeof(LVITEMW));
    lvItem.mask = LVIF_PARAM | LVIF_STATE;
    lvItem.stateMask = (UINT)-1;
    for (Index = 0; Index < Count; Index++)
    {
        lvItem.iItem = Index;
        if (SendMessageW(hDlgCtrl, LVM_GETITEMW, 0, (LPARAM)&lvItem))
        {
           if (lvItem.state & LVIS_SELECTED)
               break;
        }
    }

    if (!(lvItem.state & LVIS_SELECTED))
    {
        return;
    }

    pItem = (PNET_ITEM)lvItem.lParam;
    pNCfgComp = (INetCfgComponent*)pItem->pNCfgComp;
    hr = pNCfgComp->RaisePropertyUi(GetParent(hwndDlg), NCRP_QUERY_PROPERTY_UI, (INetConnectionConnectUi*)this);
    if (SUCCEEDED(hr))
    {
        hr = pNCfgComp->RaisePropertyUi(GetParent(hwndDlg), NCRP_SHOW_PROPERTY_UI, (INetConnectionConnectUi*)this);
    }
}

INT_PTR
CALLBACK
CNetConnectionPropertyUi::LANPropertiesUIDlg(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PROPSHEETPAGE *page;
    LPNMLISTVIEW lppl;
    LVITEMW li;
    PNET_ITEM pItem;
    CNetConnectionPropertyUi * This;
    LPPSHNOTIFY lppsn;
    DWORD dwShowIcon;
    HRESULT hr;
    WCHAR szKey[200];
    LPOLESTR pStr;
    HKEY hKey;

    switch(uMsg)
    {
        case WM_INITDIALOG:
            page = (PROPSHEETPAGE*)lParam;
            This = (CNetConnectionPropertyUi*)page->lParam;
            This->InitializeLANPropertiesUIDlg(hwndDlg);
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)This);
            return TRUE;
        case WM_NOTIFY:
            lppl = (LPNMLISTVIEW) lParam;
            lppsn = (LPPSHNOTIFY) lParam;
            if (lppsn->hdr.code == PSN_APPLY)
            {
                This = (CNetConnectionPropertyUi*)GetWindowLongPtr(hwndDlg, DWLP_USER);
                if (This->pNCfg)
                {
                    hr = This->pNCfg->Apply();
                    if (FAILED(hr))
                        return PSNRET_INVALID;
                }

                if (SendDlgItemMessageW(hwndDlg, IDC_SHOWTASKBAR, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    dwShowIcon = 1;
                else
                    dwShowIcon = 0;


                if (StringFromCLSID((CLSID)This->pProperties->guidId, &pStr) == ERROR_SUCCESS)
                {
                    swprintf(szKey, L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection", pStr);
                    CoTaskMemFree(pStr);
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, szKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
                    {
                        RegSetValueExW(hKey, L"ShowIcon", 0, REG_DWORD, (LPBYTE)&dwShowIcon, sizeof(DWORD));
                        RegCloseKey(hKey);
                    }
                }

                return PSNRET_NOERROR;
            }
#if 0
            else if (lppsn->hdr.code == PSN_CANCEL)
            {
                This = (CNetConnectionPropertyUi*)GetWindowLongPtr(hwndDlg, DWLP_USER);
                if (This->pNCfg)
                {
                    hr = This->pNCfg->Cancel();
                    if (SUCCEEDED(hr))
                        return PSNRET_NOERROR;
                    else
                        return PSNRET_INVALID;
                }
                return PSNRET_NOERROR;
            }
#endif
            if (lppl->hdr.code == LVN_ITEMCHANGING)
            {
                    ZeroMemory(&li, sizeof(li));
                    li.mask = LVIF_PARAM;
                    li.iItem = lppl->iItem;
                    if (!SendMessageW(lppl->hdr.hwndFrom, LVM_GETITEMW, 0, (LPARAM)&li))
                        return TRUE;

                    pItem = (PNET_ITEM)li.lParam;
                    if (!pItem)
                        return TRUE;

                    if (!(lppl->uOldState & LVIS_FOCUSED) && (lppl->uNewState & LVIS_FOCUSED))
                    {
                        /* new focused item */
                        if (pItem->dwCharacteristics & NCF_NOT_USER_REMOVABLE)
                            EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTALL), FALSE);
                        else
                            EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTALL), TRUE);

                        if (pItem->dwCharacteristics & NCF_HAS_UI)
                            EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTIES), TRUE);
                        else
                            EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTIES), FALSE);

                        SendDlgItemMessageW(hwndDlg, IDC_DESCRIPTION, WM_SETTEXT, 0, (LPARAM)pItem->szHelp);
                    }
             }
             break;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_PROPERTIES)
            {
                This = (CNetConnectionPropertyUi*) GetWindowLongPtr(hwndDlg, DWLP_USER);
                This->ShowNetworkComponentProperties(hwndDlg);
                return FALSE;
            }
            break;
    }
    return FALSE;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::QueryInterface(
    REFIID iid,
    LPVOID *ppvObj)
{
    LPOLESTR pStr;
    *ppvObj = NULL;

    if (IsEqualIID (iid, IID_IUnknown) ||
        IsEqualIID (iid, IID_INetConnectionPropertyUi) ||
        IsEqualIID (iid, IID_INetConnectionPropertyUi2))
    {
        *ppvObj = (INetConnectionPropertyUi2*)this;
        AddRef();
        return S_OK;
    }
    else if (IsEqualIID(iid, IID_INetLanConnectionUiInfo))
    {
        *ppvObj = (INetLanConnectionUiInfo*)this;
        AddRef();
        return S_OK;
    }
    else if (IsEqualIID(iid, IID_INetConnectionConnectUi))
    {
        *ppvObj = (INetConnectionConnectUi*)this;
        AddRef();
        return S_OK;
    }

    StringFromCLSID((CLSID)iid, &pStr);
    MessageBoxW(NULL, pStr, L"INetConnectionPropertyUi_fnQueryInterface", MB_OK);
    CoTaskMemFree(pStr);
    return E_NOINTERFACE;
}

ULONG
WINAPI
CNetConnectionPropertyUi::AddRef()
{
    ULONG refCount = InterlockedIncrement(&ref);

    return refCount;
}

ULONG
WINAPI
CNetConnectionPropertyUi::Release()
{
    ULONG refCount = InterlockedDecrement(&ref);

    if (!refCount)
    {
        if (pNCfg)
        {
            pNCfg->Uninitialize();
            pNCfg->Release();
        }
        if (NCfgLock)
        {
            NCfgLock->Release();
        }
        if (pProperties)
        {
            NcFreeNetconProperties(pProperties);
        }
        delete this;
    }

    return refCount;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::AddPages(
    HWND hwndParent,
    LPFNADDPROPSHEETPAGE pfnAddPage,
    LPARAM lParam)
{
    HPROPSHEETPAGE hProp;
    BOOL ret;
    HRESULT hr = E_FAIL;
    INITCOMMONCONTROLSEX initEx;

    initEx.dwSize = sizeof(initEx);
    initEx.dwICC = ICC_LISTVIEW_CLASSES;
    if(!InitCommonControlsEx(&initEx))
        return E_FAIL;

    hr = pCon->GetProperties(&pProperties);
    if (FAILED(hr))
        return hr;

	hProp = InitializePropertySheetPage(MAKEINTRESOURCEW(IDD_NETPROPERTIES), LANPropertiesUIDlg, (LPARAM)this, pProperties->pszwName);
    if (hProp)
    {
        ret = (*pfnAddPage)(hProp, lParam);
        if (ret)
        {
            hr = NOERROR;
        }
        else
        {
            DestroyPropertySheetPage(hProp);
        }
    }
    return hr;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::GetIcon(
    DWORD dwSize,
    HICON *phIcon)
{
    return E_NOTIMPL;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::GetDeviceGuid(GUID *pGuid)
{
    CopyMemory(pGuid, &pProperties->guidId, sizeof(GUID));
    return S_OK;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::SetConnection(INetConnection* pCon)
{
    if (this->pCon)
        this->pCon->Release();

    if (!pCon)
        return E_POINTER;

    this->pCon = pCon;
    pCon->AddRef();
    return S_OK;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::Connect(
    HWND hwndParent,
    DWORD dwFlags)
{
    if (!pCon)
        return E_POINTER; //FIXME


    if (dwFlags & NCUC_NO_UI)
        return pCon->Connect();

    return E_FAIL;
}

HRESULT
WINAPI
CNetConnectionPropertyUi::Disconnect(
    HWND hwndParent,
    DWORD dwFlags)
{
	WCHAR szBuffer[100];
    swprintf(szBuffer, L"INetConnectionConnectUi_fnDisconnect flags %x\n", dwFlags);
	MessageBoxW(NULL, szBuffer, NULL, MB_OK);

    return S_OK;
}

HRESULT WINAPI LanConnectUI_Constructor(IUnknown * pUnkOuter, REFIID riid, LPVOID * ppv)
{
    TRACE("LanConnectUI_Constructor\n");

    if (!ppv)
        return E_POINTER;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CNetConnectionPropertyUi *pConPropUi = new CNetConnectionPropertyUi;
    if (!pConPropUi)
        return E_OUTOFMEMORY;

    pConPropUi->AddRef();
    HRESULT hr = pConPropUi->QueryInterface(riid, ppv);
    pConPropUi->Release();

    return hr;
}
