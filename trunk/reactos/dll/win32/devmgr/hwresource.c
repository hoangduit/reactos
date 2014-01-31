/*
 * PROJECT:         ReactOS devmgr.dll
 * FILE:            dll/win32/devmgr/hwresource.c
 * PURPOSE:         ReactOS Device Manager
 * PROGRAMMER:      Johannes Anderwald <johannes.anderwald@reactos.org>
 * UPDATE HISTORY:
 *      2005/11/24  Created
 */

#include "precomp.h"

#define NDEBUG
#include <debug.h>

typedef struct
{
    HWND hWnd;
    HWND hWndDevList;


}HARDWARE_RESOURCE_DATA, *PHARDWARE_RESOURCE_DATA;

/* Physical Addresses are always treated as 64-bit wide */
typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

#include <pshpack4.h>
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
  UCHAR Type;
  UCHAR ShareDisposition;
  USHORT Flags;
  union {
    struct {
      PHYSICAL_ADDRESS Start;
      ULONG Length;
    } Generic;
    struct {
      PHYSICAL_ADDRESS Start;
      ULONG Length;
    } Port;
    struct {
#if defined(NT_PROCESSOR_GROUPS)
      USHORT Level;
      USHORT Group;
#else
      ULONG Level;
#endif
      ULONG Vector;
      KAFFINITY Affinity;
    } Interrupt;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    struct {
      _ANONYMOUS_UNION union {
        struct {
#if defined(NT_PROCESSOR_GROUPS)
          USHORT Group;
#else
          USHORT Reserved;
#endif
          USHORT MessageCount;
          ULONG Vector;
          KAFFINITY Affinity;
        } Raw;
        struct {
#if defined(NT_PROCESSOR_GROUPS)
          USHORT Level;
          USHORT Group;
#else
          ULONG Level;
#endif
          ULONG Vector;
          KAFFINITY Affinity;
        } Translated;
      } DUMMYUNIONNAME;
    } MessageInterrupt;
#endif
    struct {
      PHYSICAL_ADDRESS Start;
      ULONG Length;
    } Memory;
    struct {
      ULONG Channel;
      ULONG Port;
      ULONG Reserved1;
    } Dma;
    struct {
      ULONG Data[3];
    } DevicePrivate;
    struct {
      ULONG Start;
      ULONG Length;
      ULONG Reserved;
    } BusNumber;
    struct {
      ULONG DataSize;
      ULONG Reserved1;
      ULONG Reserved2;
    } DeviceSpecificData;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    struct {
      PHYSICAL_ADDRESS Start;
      ULONG Length40;
    } Memory40;
    struct {
      PHYSICAL_ADDRESS Start;
      ULONG Length48;
    } Memory48;
    struct {
      PHYSICAL_ADDRESS Start;
      ULONG Length64;
    } Memory64;
#endif
  } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include <poppack.h>
typedef struct _CM_PARTIAL_RESOURCE_LIST {
  USHORT Version;
  USHORT Revision;
  ULONG Count;
  CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

#define CmResourceTypeNull              0
#define CmResourceTypePort              1
#define CmResourceTypeInterrupt         2
#define CmResourceTypeMemory            3
#define CmResourceTypeDma               4
#define CmResourceTypeDeviceSpecific    5
#define CmResourceTypeBusNumber         6
#define CmResourceTypeNonArbitrated     128
#define CmResourceTypeConfigData        128
#define CmResourceTypeDevicePrivate     129
#define CmResourceTypePcCardConfig      130
#define CmResourceTypeMfCardConfig      131

typedef enum _INTERFACE_TYPE {
  InterfaceTypeUndefined = -1,
  Internal,
  Isa,
  Eisa,
  MicroChannel,
  TurboChannel,
  PCIBus,
  VMEBus,
  NuBus,
  PCMCIABus,
  CBus,
  MPIBus,
  MPSABus,
  ProcessorInternal,
  InternalPowerBus,
  PNPISABus,
  PNPBus,
  Vmcs,
  MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
  INTERFACE_TYPE InterfaceType;
  ULONG BusNumber;
  CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

typedef struct _CM_RESOURCE_LIST {
  ULONG                       Count;
  CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;


#define CX_TYPECOLUMN_WIDTH 120

static VOID
InitializeDevicesList(
    IN HWND hWndDevList)
{
    LVCOLUMN lvc;
    RECT rcClient;
    WCHAR szColName[255];
    int iCol = 0;

    /* set the list view style */
    (void)ListView_SetExtendedListViewStyle(hWndDevList,
                                            LVS_EX_FULLROWSELECT);

    GetClientRect(hWndDevList,
                  &rcClient);

    /* add the list view columns */
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.pszText = szColName;

    if (LoadString(hDllInstance,
                   IDS_RESOURCE_COLUMN,
                   szColName,
                   sizeof(szColName) / sizeof(szColName[0])))
    {
        lvc.cx = CX_TYPECOLUMN_WIDTH;
        (void)ListView_InsertColumn(hWndDevList,
                                    iCol++,
                                    &lvc);
    }
    if (LoadString(hDllInstance,
                   IDS_SETTING_COLUMN,
                   szColName,
                   sizeof(szColName) / sizeof(szColName[0])))
    {
        lvc.cx = rcClient.right - CX_TYPECOLUMN_WIDTH -
                 GetSystemMetrics(SM_CXVSCROLL);

        (void)ListView_InsertColumn(hWndDevList,
                                    iCol++,
                                    &lvc);
    }
}

VOID
InsertListItem(
    IN HWND hWndDevList,
    IN INT ItemCount,
    IN LPWSTR ResourceType,
    IN LPWSTR ResourceDescription)
{
    INT iItem;
    LVITEM li = {0};

    li.mask = LVIF_STATE | LVIF_TEXT;
    li.iItem = ItemCount;
    li.pszText = ResourceType;
    //li.iImage = ClassDevInfo->ImageIndex;
    iItem = ListView_InsertItem(hWndDevList, &li);

    if (iItem != -1)
    {
        li.mask = LVIF_TEXT;
        li.iItem = iItem;
        li.iSubItem = 1;
        li.pszText = ResourceDescription;
        (void)ListView_SetItem(hWndDevList, &li);
    }
}

VOID
AddResourceItems(
    IN PDEVADVPROP_INFO dap,
    IN HWND hWndDevList)
{
    HKEY hKey;
    WCHAR szBuffer[100];
    WCHAR szDetail[100];
    BYTE szData[512];
    DWORD dwSize;
    PCM_RESOURCE_LIST ResourceList;
    LONG Result;
    ULONG ItemCount = 0, Index;

    wsprintf(szBuffer, L"SYSTEM\\CurrentControlSet\\Enum\\%s\\LogConf", dap->szDeviceID);
    Result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ, &hKey);
    if (Result != ERROR_SUCCESS)
    {
        /* failed to open device instance log conf dir */
        return;
    }

    dwSize = sizeof(szData);
    Result = RegQueryValueExW(hKey, L"BootConfig", NULL, NULL, szData, &dwSize);

    RegCloseKey(hKey);
    if (Result != ERROR_SUCCESS)
    {
        /* failed to query resources */
        return;
    }

    ResourceList = (PCM_RESOURCE_LIST)szData;

    for (Index = 0; Index < ResourceList->List[0].PartialResourceList.Count; Index++)
    {
         PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor = &ResourceList->List[0].PartialResourceList.PartialDescriptors[Index];
         if (Descriptor->Type == CmResourceTypeInterrupt)
         {
             if (LoadString(hDllInstance, IDS_RESOURCE_INTERRUPT, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0])))
             {
                 wsprintf(szDetail, L"0x%08x(%d)", Descriptor->u.Interrupt.Level, Descriptor->u.Interrupt.Vector);
                 InsertListItem(hWndDevList, ItemCount, szBuffer, szDetail);
                 ItemCount++;
             }
         }
         else if (Descriptor->Type == CmResourceTypePort)
         {
             if (LoadString(hDllInstance, IDS_RESOURCE_PORT, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0])))
             {
                 wsprintf(szDetail, L"%08lx - %08lx", Descriptor->u.Port.Start.LowPart, Descriptor->u.Port.Start.LowPart + Descriptor->u.Port.Length - 1);
                 InsertListItem(hWndDevList, ItemCount, szBuffer, szDetail);
                 ItemCount++;
             }
         }
         else if (Descriptor->Type == CmResourceTypeMemory)
         {
             if (LoadString(hDllInstance, IDS_RESOURCE_MEMORY_RANGE, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0])))
             {
                 wsprintf(szDetail, L"%08I64x - %08I64x", Descriptor->u.Memory.Start.QuadPart, Descriptor->u.Memory.Start.QuadPart + Descriptor->u.Memory.Length - 1);
                 InsertListItem(hWndDevList, ItemCount, szBuffer, szDetail);
                 ItemCount++;
             }
         }
         else if (Descriptor->Type == CmResourceTypeDma)
         {
             if (LoadString(hDllInstance, IDS_RESOURCE_DMA, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0])))
             {
                 wsprintf(szDetail, L"%08ld", Descriptor->u.Dma.Channel);
                 InsertListItem(hWndDevList, ItemCount, szBuffer, szDetail);
                 ItemCount++;
             }
         }
    }
}


static VOID
UpdateDriverResourceDlg(IN HWND hwndDlg,
                        IN PDEVADVPROP_INFO dap)
{
    /* set the device image */
    SendDlgItemMessage(hwndDlg,
                       IDC_DEVICON,
                       STM_SETICON,
                       (WPARAM)dap->hDevIcon,
                       0);

    /* set the device name edit control text */
    SetDlgItemText(hwndDlg,
                   IDC_DEVNAME,
                   dap->szDevName);
}

INT_PTR
CALLBACK
ResourcesProcDriverDlgProc(IN HWND hwndDlg,
                     IN UINT uMsg,
                     IN WPARAM wParam,
                     IN LPARAM lParam)
{
    PDEVADVPROP_INFO hpd;
    HWND hWndDevList;
    INT_PTR Ret = FALSE;

    hpd = (PDEVADVPROP_INFO)GetWindowLongPtr(hwndDlg,
                                                DWL_USER);

    if (hpd != NULL || uMsg == WM_INITDIALOG)
    {
        switch (uMsg)
        {
            case WM_INITDIALOG:
            {
                /* init list */
                hWndDevList = GetDlgItem(hwndDlg, IDC_DRIVERRESOURCES);
                InitializeDevicesList(hWndDevList);

                hpd = (PDEVADVPROP_INFO)((LPPROPSHEETPAGE)lParam)->lParam;
                if (hpd != NULL)
                {
                    SetWindowLongPtr(hwndDlg,
                                     DWL_USER,
                                     (DWORD_PTR)hpd);

                    SetWindowLongPtr(hwndDlg,
                                     DWL_USER,
                                     (DWORD_PTR)hpd);

                    UpdateDriverResourceDlg(hwndDlg, hpd);
                    AddResourceItems(hpd, hWndDevList);
                }

                Ret = TRUE;
                break;
            }
        }
    }

    return Ret;
}

