/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Kernel Streaming
 * FILE:            drivers/wdm/audio/legacy/stream/driver.c
 * PURPOSE:         WDM Codec Class Driver
 * PROGRAMMER:      Johannes Anderwald
 */

#include "stream.h"

NTSTATUS
NTAPI
StreamClassAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject)
{
    PSTREAM_CLASS_DRIVER_EXTENSION DriverObjectExtension;
    PDEVICE_OBJECT DeviceObject, LowerDeviceObject;
    PSTREAM_DEVICE_EXTENSION DeviceExtension;
    PKSOBJECT_CREATE_ITEM ItemList;
    NTSTATUS Status;

    /* Fetch driver object extension */
    DriverObjectExtension = IoGetDriverObjectExtension(DriverObject, (PVOID)StreamClassAddDevice);
    if (!DriverObjectExtension)
    {
        /* Failed to get driver extension */
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }
    /* Allocate Create Item */
    ItemList = ExAllocatePool(NonPagedPool, sizeof(KSOBJECT_CREATE_ITEM));
    if (!ItemList)
    {
        /* Failed to allocated Create Item */
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* Create the FDO */
    Status = IoCreateDevice(DriverObject, DriverObjectExtension->Data.DeviceExtensionSize + sizeof(STREAM_DEVICE_EXTENSION), NULL, FILE_DEVICE_KS, FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN, 0, &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
        /* Failed to create the FDO */
        ExFreePool(ItemList);
        return Status;
    }

    /* Attach to device stack */
    LowerDeviceObject = IoAttachDeviceToDeviceStack(DeviceObject, PhysicalDeviceObject);
    if (!LowerDeviceObject)
    {
        /* Failed to attach */
        IoDeleteDevice(DeviceObject);
        return STATUS_UNSUCCESSFUL;
    }

    /* Zero Create item */
    RtlZeroMemory(ItemList, sizeof(KSOBJECT_CREATE_ITEM));
    /* Setup object class */
    RtlInitUnicodeString(&ItemList->ObjectClass, L"GLOBAL");
    /* Setup CreateDispatch routine */
    ItemList->Create = StreamClassCreateFilter;

    /* Get device extension */
    DeviceExtension = (PSTREAM_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    /* Zero device extension */
    RtlZeroMemory(DeviceExtension, sizeof(STREAM_DEVICE_EXTENSION));
    /* Initialize Ks streaming */
    Status = KsAllocateDeviceHeader(&DeviceExtension->Header, 1, ItemList);
    if (!NT_SUCCESS(Status))
    {
        /* Cleanup resources */
        IoDetachDevice(LowerDeviceObject);
        IoDeleteDevice(DeviceObject);
        ExFreePool(ItemList);
        return Status;
    }

    /* Store lower device object */
    DeviceExtension->LowerDeviceObject = LowerDeviceObject;

    /* Store physical device object */
    DeviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    /* Store driver object extension */
    DeviceExtension->DriverExtension = DriverObjectExtension;
    /* Initialize memory list */
    InitializeListHead(&DeviceExtension->MemoryResourceList);
    /* Setup device extension */
    DeviceExtension->DeviceExtension = (PVOID) (DeviceExtension + 1);
    /* Init interrupt dpc */
    KeInitializeDpc(&DeviceExtension->InterruptDpc, StreamClassInterruptDpc, (PVOID)DeviceExtension);

    /* Set device transfer method */
    DeviceObject->Flags |= DO_DIRECT_IO | DO_POWER_PAGABLE;
    /* Clear init flag */
    DeviceObject->Flags &= ~ DO_DEVICE_INITIALIZING;

    return Status;
}

/*
 *@implemented
 */
NTSTATUS
STREAMAPI
StreamClassRegisterAdapter(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN PHW_INITIALIZATION_DATA HwInitializationData)
{
    NTSTATUS Status;
    PSTREAM_CLASS_DRIVER_EXTENSION DriverObjectExtension;
    PDRIVER_OBJECT DriverObject = (PDRIVER_OBJECT)Argument1;

    /* Allocate driver extension */
    Status = IoAllocateDriverObjectExtension(DriverObject, (PVOID)StreamClassAddDevice, sizeof(STREAM_CLASS_DRIVER_EXTENSION), (PVOID*)&DriverObjectExtension);
    if (!NT_SUCCESS(Status))
    {
        /* Failed to allocate */
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* Zero driver object extension */
    RtlZeroMemory(DriverObjectExtension, sizeof(STREAM_CLASS_DRIVER_EXTENSION));

    /* copy HwInitializationData */
    RtlCopyMemory(&DriverObjectExtension->Data, HwInitializationData, sizeof(HW_INITIALIZATION_DATA));

    /* Setup device init methods */
    DriverObject->DriverExtension->AddDevice = StreamClassAddDevice;
    DriverObject->DriverUnload = KsNullDriverUnload;

    /* Setup irp handlers */
    DriverObject->MajorFunction[IRP_MJ_PNP] = StreamClassPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = StreamClassPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = StreamClassSystemControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = StreamClassCleanup;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = StreamClassFlushBuffers;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = StreamClassDeviceControl;

    /* Let Ks handle these */
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);

    return STATUS_SUCCESS;
}

/*
 *@implemented
 */

VOID
NTAPI
StreamClassReenumerateStreams(
    IN PVOID  HwDeviceExtension,
    IN ULONG  StreamDescriptorSize)
{
    HW_STREAM_REQUEST_BLOCK_EXT RequestBlock;
    PSTREAM_DEVICE_EXTENSION DeviceExtension;
    PHW_STREAM_DESCRIPTOR StreamDescriptor;

    if (!HwDeviceExtension || !StreamDescriptorSize)
        return;

    StreamDescriptor = ExAllocatePool(NonPagedPool, StreamDescriptorSize);
    if (!StreamDescriptor)
        return;

    /* Zero stream descriptor */
    RtlZeroMemory(StreamDescriptor, StreamDescriptorSize);

    /* Get our DeviceExtension */
    DeviceExtension = (PSTREAM_DEVICE_EXTENSION) ((ULONG_PTR)HwDeviceExtension - sizeof(STREAM_DEVICE_EXTENSION));
    ASSERT(DeviceExtension->DeviceExtension == HwDeviceExtension);


    /* Zero RequestBlock */
    RtlZeroMemory(&RequestBlock, sizeof(HW_STREAM_REQUEST_BLOCK_EXT));

    /* Setup get stream info struct */
    RequestBlock.Block.SizeOfThisPacket = sizeof(HW_STREAM_REQUEST_BLOCK);
    RequestBlock.Block.Command = SRB_GET_STREAM_INFO;
    RequestBlock.Block.CommandData.StreamBuffer = StreamDescriptor;
    KeInitializeEvent(&RequestBlock.Event, SynchronizationEvent, FALSE);

    /* FIXME SYNCHRONIZATION */

    /* Send the request */
    DeviceExtension->DriverExtension->Data.HwReceivePacket ((PHW_STREAM_REQUEST_BLOCK)&RequestBlock);

    /* Is the device already completed? */
    if (RequestBlock.Block.Status == STATUS_PENDING)
    {
        /* Request is pending, wait for result */
        KeWaitForSingleObject(&RequestBlock.Event, Executive, KernelMode, FALSE, NULL);
    }

    if (!NT_SUCCESS(RequestBlock.Block.Status))
    {
        /* Release Stream descriptor */
        ExFreePool(StreamDescriptor);
    }
    else
    {
        if (DeviceExtension->StreamDescriptor)
        {
            /* Release old stream descriptor */
            ExFreePool(DeviceExtension->StreamDescriptor);
        }

        /* Store stream descriptor */
        DeviceExtension->StreamDescriptor = StreamDescriptor;
        DeviceExtension->StreamDescriptorSize = StreamDescriptorSize;
    }

}

/*
 *@implemented
 */

VOID
NTAPI
StreamClassDebugAssert(
    IN PCHAR File,
    IN ULONG Line,
    IN PCHAR AssertText,
    IN ULONG AssertValue)
{
#if DBG
    DbgBreakPoint();
#endif
}

/*
 *@implemented
 */
VOID
__cdecl
StreamClassDebugPrint(
    IN STREAM_DEBUG_LEVEL DebugPrintLevel,
    IN PCCHAR DebugMessage,
    ...)
{
#if DBG
    va_list ap;

    if (DebugPrintLevel <=STREAMDEBUG_LEVEL)
    {
        va_start(ap, DebugMessage);

        DbgPrint(DebugMessage, ap);

        va_end(ap);
    }
#endif

}

/*
 *@unimplemented
 */
VOID
__cdecl
StreamClassDeviceNotification(
    IN STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE  NotificationType,
    IN PVOID  HwDeviceExtension,
    IN PHW_STREAM_REQUEST_BLOCK  pSrb,
    IN PKSEVENT_ENTRY  EventEntry,
    IN GUID  *EventSet,
    IN ULONG  EventId)
{
    PHW_STREAM_REQUEST_BLOCK_EXT RequestBlock;
    if (NotificationType == DeviceRequestComplete)
    {
        RequestBlock = (PHW_STREAM_REQUEST_BLOCK_EXT)pSrb;

        KeSetEvent(&RequestBlock->Event, 0, FALSE);
        return;
    }

    UNIMPLEMENTED
}

/*
 *@implemented
 */
PVOID
STREAMAPI
StreamClassGetDmaBuffer(
    IN PVOID  HwDeviceExtension)
{
    PSTREAM_DEVICE_EXTENSION DeviceExtension;

    /* Get our DeviceExtension */
    DeviceExtension = (PSTREAM_DEVICE_EXTENSION) ((ULONG_PTR)HwDeviceExtension - sizeof(STREAM_DEVICE_EXTENSION));
    ASSERT(DeviceExtension->DeviceExtension == HwDeviceExtension);

    return DeviceExtension->DmaCommonBuffer;
}

NTSTATUS
NTAPI
StreamClassRWCompletion(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP  Irp,
    IN PVOID  Context)
{
    PIO_STATUS_BLOCK IoStatusBlock = (PIO_STATUS_BLOCK)Context;

    IoStatusBlock->Information = Irp->IoStatus.Information;
    IoStatusBlock->Status = Irp->IoStatus.Status;

    return STATUS_SUCCESS;
}

/*
 *@implemented
 */
BOOLEAN
STREAMAPI
StreamClassReadWriteConfig(
    IN PVOID  HwDeviceExtension,
    IN BOOLEAN  Read,
    IN PVOID  Buffer,
    IN ULONG  OffSet,
    IN ULONG  Length)
{
    PIRP Irp;
    ULONG MajorFunction;
    KEVENT Event;
    PSTREAM_DEVICE_EXTENSION DeviceExtension;
    LARGE_INTEGER Offset;
    IO_STATUS_BLOCK StatusBlock;
    NTSTATUS Status;

    /* Get our DeviceExtension */
    DeviceExtension = (PSTREAM_DEVICE_EXTENSION) ((ULONG_PTR)HwDeviceExtension - sizeof(STREAM_DEVICE_EXTENSION));
    ASSERT(DeviceExtension->DeviceExtension == HwDeviceExtension);

    if (Read)
    {
        /* Zero input buffer */
        RtlZeroMemory(Buffer, Length);
    }

    /* Set request type */
    MajorFunction = (Read ? IRP_MJ_READ : IRP_MJ_WRITE);

    /* Initialize event */
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    /* Set offset */
    Offset.QuadPart = OffSet;

    /* Pre-init status block */
    StatusBlock.Status = STATUS_NOT_SUPPORTED;

    /* Create Irp */
    Irp = IoBuildSynchronousFsdRequest(MajorFunction,
                                       DeviceExtension->LowerDeviceObject, /* Verify */
                                       Buffer,
                                       Length,
                                       &Offset,
                                       &Event,
                                       &StatusBlock);

    if (!Irp)
    {
        /* Failed to allocate memory */
        return FALSE;
    }

    /* Setup a completion routine */
    IoSetCompletionRoutine(Irp, StreamClassRWCompletion, (PVOID)&Event, TRUE, TRUE, TRUE);

    /* Call driver */
    Status = IoCallDriver(DeviceExtension->LowerDeviceObject, Irp);

    if (Status == STATUS_PENDING)
    {
        /* Request is pending, wait for result */
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        /* Fetch result */
        Status = StatusBlock.Status;
    }

    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    /* FIXME Handle Length != InputLength */
    return TRUE;
}
