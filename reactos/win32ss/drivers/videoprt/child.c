/*
 * VideoPort driver
 *
 * Copyright (C) 2012 ReactOS Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "videoprt.h"

/* PRIVATE FUNCTIONS **********************************************************/

BOOLEAN
NTAPI
IntVideoPortGetMonitorId(
    IN PVIDEO_PORT_CHILD_EXTENSION ChildExtension,
    IN OUT PWCHAR Buffer)
{
    USHORT Manufacturer, Model;
    UNICODE_STRING UnicodeModelStr;

    /* This must be valid to call this function */
    ASSERT(ChildExtension->EdidValid);

    /* 3 letters 5-bit ANSI manufacturer code (big endian) */
    Manufacturer = *(PUSHORT)(&ChildExtension->ChildDescriptor[8]);

    /* Letters encoded as A=1 to Z=26 */
    Buffer[0] = (WCHAR)((Manufacturer & 0x7C00) + 'A' - 1);
    Buffer[1] = (WCHAR)((Manufacturer & 0x03E0) + 'A' - 1);
    Buffer[2] = (WCHAR)((Manufacturer & 0x001F) + 'A' - 1);

    /* Model number (16-bit little endian) */
    Model = *(PUSHORT)(&ChildExtension->ChildDescriptor[10]);

    /* Use Rtl helper for conversion */
    UnicodeModelStr.Buffer = &Buffer[3];
    UnicodeModelStr.Length = 0;
    UnicodeModelStr.MaximumLength = 4 * sizeof(WCHAR);
    RtlIntegerToUnicodeString(Model, 16, &UnicodeModelStr);

    /* Terminate it */
    Buffer[7] = UNICODE_NULL;

    /* And we're done */
    return TRUE;
}

NTSTATUS NTAPI
IntVideoPortChildQueryId(
    IN PVIDEO_PORT_CHILD_EXTENSION ChildExtension,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp)
{
    PWCHAR Buffer = NULL, StaticBuffer;
    UNICODE_STRING UnicodeStr;
    ULONG Length;
    
    switch (IrpSp->Parameters.QueryId.IdType)
    {
        case BusQueryDeviceID:
            switch (ChildExtension->ChildType)
            {
                case Monitor:
                    if (ChildExtension->EdidValid)
                    {
                        StaticBuffer = L"DISPLAY\\";
                        Length = 8 * sizeof(WCHAR);
                        Buffer = ExAllocatePool(PagedPool, (wcslen(StaticBuffer) + 8) * sizeof(WCHAR));
                        if (!Buffer) return STATUS_NO_MEMORY;

                        /* Write the static portion */
                        RtlCopyMemory(Buffer, StaticBuffer, wcslen(StaticBuffer) * sizeof(WCHAR));

                        /* Add the dynamic portion */
                        IntVideoPortGetMonitorId(ChildExtension,
                                                 &Buffer[wcslen(StaticBuffer)]);
                    }
                    else
                    {
                        StaticBuffer = L"DISPLAY\\Default_Monitor";
                        Length = wcslen(StaticBuffer) * sizeof(WCHAR);
                        Buffer = ExAllocatePool(PagedPool, (wcslen(StaticBuffer) + 1) * sizeof(WCHAR));
                        if (!Buffer) return STATUS_NO_MEMORY;

                        /* Copy the default id */
                        RtlCopyMemory(Buffer, StaticBuffer, (wcslen(StaticBuffer) + 1) * sizeof(WCHAR));
                    }
                    break;
                default:
                    ASSERT(FALSE);
                    break;
            }
            break;
        case BusQueryInstanceID:
            Buffer = ExAllocatePool(PagedPool, 5 * sizeof(WCHAR));
            if (!Buffer) return STATUS_NO_MEMORY;

            UnicodeStr.Buffer = Buffer;
            UnicodeStr.Length = 0;
            UnicodeStr.MaximumLength = 4 * sizeof(WCHAR);
            RtlIntegerToUnicodeString(ChildExtension->ChildId, 16, &UnicodeStr);
            break;
        case BusQueryHardwareIDs:
            switch (ChildExtension->ChildType)
            {
                case Monitor:
                    if (ChildExtension->EdidValid)
                    {
                        StaticBuffer = L"MONITOR\\";
                        Length = 8 * sizeof(WCHAR);
                        Buffer = ExAllocatePool(PagedPool, (wcslen(StaticBuffer) + 9) * sizeof(WCHAR));
                        if (!Buffer) return STATUS_NO_MEMORY;

                        /* Write the static portion */
                        RtlCopyMemory(Buffer, StaticBuffer, wcslen(StaticBuffer) * sizeof(WCHAR));

                        /* Add the dynamic portion */
                        IntVideoPortGetMonitorId(ChildExtension,
                                                 &Buffer[wcslen(StaticBuffer)]);

                        /* Add the second null termination char */
                        Buffer[wcslen(StaticBuffer) + 8] = UNICODE_NULL;
                    }
                    else
                    {
                        StaticBuffer = L"MONITOR\\Default_Monitor";
                        Length = wcslen(StaticBuffer) * sizeof(WCHAR);
                        Buffer = ExAllocatePool(PagedPool, (wcslen(StaticBuffer) + 2) * sizeof(WCHAR));
                        if (!Buffer) return STATUS_NO_MEMORY;

                        /* Copy the default id */
                        RtlCopyMemory(Buffer, StaticBuffer, (wcslen(StaticBuffer) + 1) * sizeof(WCHAR));

                        /* Add the second null terminator */
                        Buffer[wcslen(StaticBuffer) + 1] = UNICODE_NULL;
                    }
                    break;
                default:
                    ASSERT(FALSE);
                    break;
            }
            break;
        case BusQueryCompatibleIDs:
            switch (ChildExtension->ChildType)
            {
                case Monitor:
                    if (ChildExtension->EdidValid)
                    {
                        StaticBuffer = L"*PNP09FF";
                        Buffer = ExAllocatePool(PagedPool, (wcslen(StaticBuffer) + 2) * sizeof(WCHAR));
                        if (!Buffer) return STATUS_NO_MEMORY;

                        RtlCopyMemory(Buffer, StaticBuffer, (wcslen(StaticBuffer) + 1) * sizeof(WCHAR));

                        Buffer[wcslen(StaticBuffer)+1] = UNICODE_NULL;
                    }
                    else
                    {
                        /* No PNP ID for non-PnP monitors */
                        return Irp->IoStatus.Status;
                    }
                    break;
                default:
                    ASSERT(FALSE);
                    break;
            }
            break;
        default:
            return Irp->IoStatus.Status;
    }

    INFO_(VIDEOPRT, "Reporting ID: %S\n", Buffer);
    Irp->IoStatus.Information = (ULONG_PTR)Buffer;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
IntVideoPortChildQueryText(
    IN PVIDEO_PORT_CHILD_EXTENSION ChildExtension,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp)
{
    PWCHAR Buffer, StaticBuffer;

    if (IrpSp->Parameters.QueryDeviceText.DeviceTextType != DeviceTextDescription)
        return Irp->IoStatus.Status;

    switch (ChildExtension->ChildType)
    {
        case Monitor:
            /* FIXME: We can return a better description I think */
            StaticBuffer = L"Monitor";
            break;

        case VideoChip:
            /* FIXME: No idea what we return here */
            StaticBuffer = L"Video chip";
            break;

        default: /* Other */
            StaticBuffer = L"Other device";
            break;
    }

    Buffer = ExAllocatePool(PagedPool, (wcslen(StaticBuffer) + 1) * sizeof(WCHAR));
    if (!Buffer) return STATUS_NO_MEMORY;

    RtlCopyMemory(Buffer, StaticBuffer, (wcslen(StaticBuffer) + 1) * sizeof(WCHAR));

    INFO_(VIDEOPRT, "Reporting description: %S\n", Buffer);
    Irp->IoStatus.Information = (ULONG_PTR)Buffer;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
IntVideoPortChildQueryRelations(
    IN PVIDEO_PORT_CHILD_EXTENSION ChildExtension,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp)
{
    PDEVICE_RELATIONS DeviceRelations;

    if (IrpSp->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation)
    {
        WARN_(VIDEOPRT, "Unsupported device relations type\n");
        return Irp->IoStatus.Status;
    }

    DeviceRelations = ExAllocatePool(NonPagedPool, sizeof(DEVICE_RELATIONS));
    if (!DeviceRelations) return STATUS_NO_MEMORY;

    DeviceRelations->Count = 1;
    DeviceRelations->Objects[0] = ChildExtension->PhysicalDeviceObject;

    ObReferenceObject(DeviceRelations->Objects[0]);

    Irp->IoStatus.Information = (ULONG_PTR)DeviceRelations;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
IntVideoPortChildQueryCapabilities(
    IN PVIDEO_PORT_CHILD_EXTENSION ChildExtension,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp)
{
    PDEVICE_CAPABILITIES DeviceCaps = IrpSp->Parameters.DeviceCapabilities.Capabilities;
    ULONG i;

    /* Set some values */
    DeviceCaps->LockSupported = FALSE;
    DeviceCaps->EjectSupported = FALSE;
    DeviceCaps->DockDevice = FALSE;
    DeviceCaps->UniqueID = FALSE;
    DeviceCaps->RawDeviceOK = FALSE;
    DeviceCaps->WakeFromD0 = FALSE;
    DeviceCaps->WakeFromD1 = FALSE;
    DeviceCaps->WakeFromD2 = FALSE;
    DeviceCaps->WakeFromD3 = FALSE;
    DeviceCaps->HardwareDisabled = FALSE;
    DeviceCaps->NoDisplayInUI = FALSE;

    /* Address and UI number are set by default */

    DeviceCaps->DeviceState[PowerSystemWorking] = PowerDeviceD0;
    for (i = 1; i < POWER_SYSTEM_MAXIMUM; i++)
    {
        DeviceCaps->DeviceState[i] = PowerDeviceD3;
    }

    DeviceCaps->SystemWake = PowerSystemUnspecified;
    DeviceCaps->DeviceWake = PowerDeviceUnspecified;

    /* FIXME: Device power states */
    DeviceCaps->DeviceD1 = FALSE;
    DeviceCaps->DeviceD2 = FALSE;
    DeviceCaps->D1Latency = 0;
    DeviceCaps->D2Latency = 0;
    DeviceCaps->D3Latency = 0; 

    switch (ChildExtension->ChildType)
    {
        case VideoChip:
            /* FIXME: Copy capabilities from parent */
            ASSERT(FALSE);
            break;

        case NonPrimaryChip: /* Reserved */
            ASSERT(FALSE);
            break;

        case Monitor:
            DeviceCaps->SilentInstall = TRUE;
            DeviceCaps->Removable = TRUE;
            DeviceCaps->SurpriseRemovalOK = TRUE;
            break;

        default: /* Other */
            DeviceCaps->SilentInstall = FALSE;
            DeviceCaps->Removable = FALSE;
            DeviceCaps->SurpriseRemovalOK = FALSE;
            break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
IntVideoPortDispatchPdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status = Irp->IoStatus.Status;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->MinorFunction)
    {
        case IRP_MN_START_DEVICE:
        case IRP_MN_STOP_DEVICE:
            /* Nothing to do */
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            /* None (keep old status) */
            break;

        case IRP_MN_QUERY_ID:
            /* Call our helper */
            Status = IntVideoPortChildQueryId(DeviceObject->DeviceExtension,
                                              Irp,
                                              IrpSp);
            break;

        case IRP_MN_QUERY_CAPABILITIES:
            /* Call our helper */
            Status = IntVideoPortChildQueryCapabilities(DeviceObject->DeviceExtension,
                                                        Irp,
                                                        IrpSp);
            break;

        case IRP_MN_SURPRISE_REMOVAL:
        case IRP_MN_QUERY_REMOVE_DEVICE:
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            IoDeleteDevice(DeviceObject);
            return STATUS_SUCCESS;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            /* Call our helper */
            Status = IntVideoPortChildQueryRelations(DeviceObject->DeviceExtension,
                                                     Irp,
                                                     IrpSp);
            break;

        case IRP_MN_QUERY_DEVICE_TEXT:
            /* Call our helper */
            Status = IntVideoPortChildQueryText(DeviceObject->DeviceExtension,
                                                Irp,
                                                IrpSp);
            break;

        default:
            break;
    }

    Irp->IoStatus.Status = Status;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}
