/*
 * PROJECT:     ReactOS Named Pipe FileSystem
 * LICENSE:     BSD - See COPYING.ARM in the top level directory
 * FILE:        drivers/filesystems/npfs/volinfo.c
 * PURPOSE:     Named Pipe FileSystem Volume Information
 * PROGRAMMERS: ReactOS Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include "npfs.h"

// File ID number for NPFS bugchecking support
#define NPFS_BUGCHECK_FILE_ID   (NPFS_BUGCHECK_VOLINFO)

/* FUNCTIONS ******************************************************************/

NTSTATUS
NTAPI
NpQueryFsVolumeInfo(IN PVOID Buffer,
                    IN OUT PULONG Length)
{
    PFILE_FS_VOLUME_INFORMATION InfoBuffer = Buffer;
    NTSTATUS Status;
    USHORT NameLength;

    *Length -= sizeof(*InfoBuffer);

    InfoBuffer->VolumeCreationTime.LowPart = 0;
    InfoBuffer->VolumeCreationTime.HighPart = 0;
    InfoBuffer->VolumeSerialNumber = 0;
    InfoBuffer->SupportsObjects = 0;

    NameLength = 18;
    InfoBuffer->VolumeLabelLength = 18;

    if (NameLength < 18)
    {
        NameLength = (USHORT)*Length;
        Status = STATUS_BUFFER_OVERFLOW;
    }
    else
    {
        Status = STATUS_SUCCESS;
    }

    RtlCopyMemory(InfoBuffer->VolumeLabel, L"Named Pipe", NameLength);
    *Length -= NameLength;

    return Status;
}

NTSTATUS
NTAPI
NpQueryFsSizeInfo(IN PVOID Buffer,
                  IN OUT PULONG Length)
{
    PFILE_FS_SIZE_INFORMATION InfoBuffer = Buffer;

    *Length -= sizeof(*InfoBuffer);

    InfoBuffer->TotalAllocationUnits.QuadPart = 0;
    InfoBuffer->AvailableAllocationUnits.QuadPart = 0;
    InfoBuffer->SectorsPerAllocationUnit = 1;
    InfoBuffer->BytesPerSector = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
NpQueryFsDeviceInfo(IN PVOID Buffer,
                    IN OUT PULONG Length)
{
    PFILE_FS_DEVICE_INFORMATION InfoBuffer = Buffer;
    NTSTATUS Status;

    if (*Length >= sizeof(*InfoBuffer))
    {
        InfoBuffer->DeviceType = 0;
        InfoBuffer->Characteristics = 0;
        InfoBuffer->DeviceType = FILE_DEVICE_NAMED_PIPE;
        *Length -= sizeof(*InfoBuffer);
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_BUFFER_OVERFLOW;
    }
    return Status;
}

NTSTATUS
NTAPI
NpQueryFsAttributeInfo(IN PVOID Buffer,
                       IN OUT PULONG Length)
{
    PFILE_FS_ATTRIBUTE_INFORMATION InfoBuffer = Buffer;
    NTSTATUS Status;
    USHORT NameLength;

    NameLength = (USHORT)(*Length - 12);
    if (NameLength < 8)
    {
        *Length = 0;
        Status = STATUS_BUFFER_OVERFLOW;
    }
    else
    {
        *Length -= 20;
        NameLength = 8;
        Status = STATUS_SUCCESS;
    }

    InfoBuffer->MaximumComponentNameLength = 0xFFFFFFFF;
    InfoBuffer->FileSystemNameLength = 8;
    InfoBuffer->FileSystemAttributes = FILE_CASE_PRESERVED_NAMES;
    RtlCopyMemory(InfoBuffer->FileSystemName, L"NPFS", NameLength);

    return Status;
}

NTSTATUS
NTAPI
NpQueryFsFullSizeInfo(IN PVOID Buffer,
                      IN OUT PULONG Length)
{
    PFILE_FS_FULL_SIZE_INFORMATION InfoBuffer = Buffer;

    *Length -= sizeof(*InfoBuffer);

    RtlZeroMemory(InfoBuffer, sizeof(*InfoBuffer));

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
NpCommonQueryVolumeInformation(IN PDEVICE_OBJECT DeviceObject,
                               IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    FS_INFORMATION_CLASS InfoClass;
    ULONG Length;
    PVOID Buffer;
    NTSTATUS Status;
    PAGED_CODE();

    IoStack = IoGetCurrentIrpStackLocation(Irp);
    Buffer = Irp->AssociatedIrp.SystemBuffer;
    Length = IoStack->Parameters.QueryVolume.Length;
    InfoClass = IoStack->Parameters.QueryVolume.FsInformationClass;

    switch (InfoClass)
    {
        case FileFsVolumeInformation:
            Status = NpQueryFsVolumeInfo(Buffer, &Length);
            break;
        case FileFsSizeInformation:
            Status = NpQueryFsSizeInfo(Buffer, &Length);
            break;
        case FileFsDeviceInformation:
            Status = NpQueryFsDeviceInfo(Buffer, &Length);
            break;
        case FileFsAttributeInformation:
            Status = NpQueryFsAttributeInfo(Buffer, &Length);
            break;
        case FileFsFullSizeInformation:
            Status = NpQueryFsFullSizeInfo(Buffer, &Length);
            break;
        default:
            Status = STATUS_NOT_SUPPORTED;
            break;
    }

    Irp->IoStatus.Information = IoStack->Parameters.QueryVolume.Length - Length;
    return Status;
}

NTSTATUS
NTAPI
NpFsdQueryVolumeInformation(IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp)
{
    NTSTATUS Status;
    PAGED_CODE();

    FsRtlEnterFileSystem();
    ExAcquireResourceSharedLite(&NpVcb->Lock, TRUE);

    Status = NpCommonQueryVolumeInformation(DeviceObject, Irp);

    ExReleaseResourceLite(&NpVcb->Lock);
    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING)
    {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NAMED_PIPE_INCREMENT);
    }

    return Status;
}

/* EOF */
