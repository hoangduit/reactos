/*
 *  ReactOS kernel
 *  Copyright (C) 2002, 2003, 2004, 2005 ReactOS Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS text-mode setup
 * FILE:            subsys/system/usetup/partlist.c
 * PURPOSE:         Partition list functions
 * PROGRAMMER:      Eric Kohl
 *                  Casper S. Hornstrup (chorns@users.sourceforge.net)
 */

#include "usetup.h"

#include <ntddscsi.h>

#define NDEBUG
#include <debug.h>

#define DUMP_PARTITION_TABLE

/* FUNCTIONS ****************************************************************/

#ifdef DUMP_PARTITION_TABLE
static
VOID
DumpPartitionTable(
    PDISKENTRY DiskEntry)
{
    PPARTITION_INFORMATION PartitionInfo;
    ULONG i;

    for (i = 0; i < DiskEntry->LayoutBuffer->PartitionCount; i++)
    {
        PartitionInfo = &DiskEntry->LayoutBuffer->PartitionEntry[i];
        DPRINT("\n%lu: %12I64u  %12I64u  %10lu  %2lu  %2x  %c  %c\n",
                i,
                PartitionInfo->StartingOffset.QuadPart,
                PartitionInfo->PartitionLength.QuadPart,
                PartitionInfo->HiddenSectors,
                PartitionInfo->PartitionNumber,
                PartitionInfo->PartitionType,
                PartitionInfo->BootIndicator ? '*': ' ',
                PartitionInfo->RewritePartition ? 'Y': 'N');
    }
}
#endif


ULONGLONG
Align(
    IN ULONGLONG Value,
    IN ULONG Alignment)
{
    ULONGLONG Temp;

    Temp = Value / Alignment;

    return Temp * Alignment;
}


ULONGLONG
RoundingDivide(
   IN ULONGLONG Dividend,
   IN ULONGLONG Divisor)
{
    return (Dividend + Divisor / 2) / Divisor;
}


static
VOID
GetDriverName(
    PDISKENTRY DiskEntry)
{
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    WCHAR KeyName[32];
    NTSTATUS Status;

    RtlInitUnicodeString(&DiskEntry->DriverName,
                         NULL);

    swprintf(KeyName,
             L"\\Scsi\\Scsi Port %lu",
             DiskEntry->Port);

    RtlZeroMemory(&QueryTable,
                  sizeof(QueryTable));

    QueryTable[0].Name = L"Driver";
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[0].EntryContext = &DiskEntry->DriverName;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_DEVICEMAP,
                                    KeyName,
                                    QueryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("RtlQueryRegistryValues() failed (Status %lx)\n", Status);
    }
}


static
VOID
AssignDriveLetters(
    PPARTLIST List)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PLIST_ENTRY Entry1;
    PLIST_ENTRY Entry2;
    CHAR Letter;

    Letter = 'C';

    /* Assign drive letters to primary partitions */
    Entry1 = List->DiskListHead.Flink;
    while (Entry1 != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry1, DISKENTRY, ListEntry);

        Entry2 = DiskEntry->PrimaryPartListHead.Flink;
        while (Entry2 != &DiskEntry->PrimaryPartListHead)
        {
            PartEntry = CONTAINING_RECORD(Entry2, PARTENTRY, ListEntry);

            PartEntry->DriveLetter = 0;

            if (PartEntry->IsPartitioned &&
                !IsContainerPartition(PartEntry->PartitionType))
            {
                if (IsRecognizedPartition(PartEntry->PartitionType) ||
                    (PartEntry->PartitionType == PARTITION_ENTRY_UNUSED &&
                     PartEntry->SectorCount.QuadPart != 0LL))
                {
                    if (Letter <= 'Z')
                    {
                        PartEntry->DriveLetter = Letter;
                        Letter++;
                    }
                }
            }

            Entry2 = Entry2->Flink;
        }

        Entry1 = Entry1->Flink;
    }

    /* Assign drive letters to logical drives */
#if 0
    Entry1 = List->DiskListHead.Flink;
    while (Entry1 != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry1, DISKENTRY, ListEntry);

        Entry2 = DiskEntry->PartListHead.Flink;
        if (Entry2 != &DiskEntry->PartListHead)
        {
            Entry2 = Entry2->Flink;
            while (Entry2 != &DiskEntry->PartListHead)
            {
                PartEntry = CONTAINING_RECORD(Entry2,
                                              PARTENTRY,
                                              ListEntry);

                PartEntry->DriveLetter = 0;

                if (PartEntry->Unpartitioned == FALSE &&
                    !IsContainerPartition(PartEntry->PartInfo[0].PartitionType))
                {
                    if (IsRecognizedPartition(PartEntry->PartInfo[0].PartitionType) ||
                        (PartEntry->PartInfo[0].PartitionType == PARTITION_ENTRY_UNUSED &&
                         PartEntry->PartInfo[0].PartitionLength.QuadPart != 0LL))
                    {
                        if (Letter <= 'Z')
                        {
                            PartEntry->DriveLetter = Letter;
                            Letter++;
                        }
                    }
                }

                Entry2 = Entry2->Flink;
            }
        }

        Entry1 = Entry1->Flink;
    }
#endif
}


static
VOID
UpdatePartitionNumbers(
    PDISKENTRY DiskEntry)
{
    PPARTENTRY PartEntry;
    PLIST_ENTRY Entry;
//    ULONG PartitionNumber = 1;
    ULONG PartitionIndex = 0;

    Entry = DiskEntry->PrimaryPartListHead.Flink;
    while (Entry != &DiskEntry->PrimaryPartListHead)
    {
        PartEntry = CONTAINING_RECORD(Entry,
                                      PARTENTRY,
                                      ListEntry);

        if (PartEntry->IsPartitioned == FALSE)
        {
//            PartEntry->PartitionNumber = 0;
            PartEntry->PartitionIndex = (ULONG)-1;
        }
        else
        {
            if (IsContainerPartition(PartEntry->PartitionType))
            {
//                PartEntry->PartitionNumber = 0;
            }
            else if (PartEntry->PartitionType == PARTITION_ENTRY_UNUSED &&
                     PartEntry->SectorCount.QuadPart == 0ULL)
            {
//                PartEntry->PartitionNumber = 0;
            }
            else
            {
//                PartEntry->PartitionNumber = PartitionNumber++;
            }

            PartEntry->PartitionIndex = PartitionIndex++;
        }

        Entry = Entry->Flink;
    }
}


NTSTATUS
NTAPI
DiskIdentifierQueryRoutine(
    PWSTR ValueName,
    ULONG ValueType,
    PVOID ValueData,
    ULONG ValueLength,
    PVOID Context,
    PVOID EntryContext)
{
    PBIOSDISKENTRY BiosDiskEntry = (PBIOSDISKENTRY)Context;
    UNICODE_STRING NameU;

    if (ValueType == REG_SZ &&
        ValueLength == 20 * sizeof(WCHAR))
    {
        NameU.Buffer = (PWCHAR)ValueData;
        NameU.Length = NameU.MaximumLength = 8 * sizeof(WCHAR);
        RtlUnicodeStringToInteger(&NameU, 16, &BiosDiskEntry->Checksum);

        NameU.Buffer = (PWCHAR)ValueData + 9;
        RtlUnicodeStringToInteger(&NameU, 16, &BiosDiskEntry->Signature);

        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
NTAPI
DiskConfigurationDataQueryRoutine(
    PWSTR ValueName,
    ULONG ValueType,
    PVOID ValueData,
    ULONG ValueLength,
    PVOID Context,
    PVOID EntryContext)
{
    PBIOSDISKENTRY BiosDiskEntry = (PBIOSDISKENTRY)Context;
    PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor;
    PCM_DISK_GEOMETRY_DEVICE_DATA DiskGeometry;
    ULONG i;

    if (ValueType != REG_FULL_RESOURCE_DESCRIPTOR ||
        ValueLength < sizeof(CM_FULL_RESOURCE_DESCRIPTOR))
        return STATUS_UNSUCCESSFUL;

    FullResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)ValueData;

    /* Hm. Version and Revision are not set on Microsoft Windows XP... */
#if 0
    if (FullResourceDescriptor->PartialResourceList.Version != 1 ||
        FullResourceDescriptor->PartialResourceList.Revision != 1)
        return STATUS_UNSUCCESSFUL;
#endif

    for (i = 0; i < FullResourceDescriptor->PartialResourceList.Count; i++)
    {
        if (FullResourceDescriptor->PartialResourceList.PartialDescriptors[i].Type != CmResourceTypeDeviceSpecific ||
            FullResourceDescriptor->PartialResourceList.PartialDescriptors[i].u.DeviceSpecificData.DataSize != sizeof(CM_DISK_GEOMETRY_DEVICE_DATA))
            continue;

        DiskGeometry = (PCM_DISK_GEOMETRY_DEVICE_DATA)&FullResourceDescriptor->PartialResourceList.PartialDescriptors[i + 1];
        BiosDiskEntry->DiskGeometry = *DiskGeometry;

        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
NTAPI
SystemConfigurationDataQueryRoutine(
    PWSTR ValueName,
    ULONG ValueType,
    PVOID ValueData,
    ULONG ValueLength,
    PVOID Context,
    PVOID EntryContext)
{
    PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor;
    PCM_INT13_DRIVE_PARAMETER* Int13Drives = (PCM_INT13_DRIVE_PARAMETER*)Context;
    ULONG i;

    if (ValueType != REG_FULL_RESOURCE_DESCRIPTOR ||
        ValueLength < sizeof (CM_FULL_RESOURCE_DESCRIPTOR))
        return STATUS_UNSUCCESSFUL;

    FullResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)ValueData;

    /* Hm. Version and Revision are not set on Microsoft Windows XP... */
#if 0
    if (FullResourceDescriptor->PartialResourceList.Version != 1 ||
        FullResourceDescriptor->PartialResourceList.Revision != 1)
        return STATUS_UNSUCCESSFUL;
#endif

    for (i = 0; i < FullResourceDescriptor->PartialResourceList.Count; i++)
    {
        if (FullResourceDescriptor->PartialResourceList.PartialDescriptors[i].Type != CmResourceTypeDeviceSpecific ||
            FullResourceDescriptor->PartialResourceList.PartialDescriptors[i].u.DeviceSpecificData.DataSize % sizeof(CM_INT13_DRIVE_PARAMETER) != 0)
            continue;

        *Int13Drives = (CM_INT13_DRIVE_PARAMETER*) RtlAllocateHeap(ProcessHeap, 0, FullResourceDescriptor->PartialResourceList.PartialDescriptors[i].u.DeviceSpecificData.DataSize);
        if (*Int13Drives == NULL)
            return STATUS_NO_MEMORY;

        memcpy(*Int13Drives,
               &FullResourceDescriptor->PartialResourceList.PartialDescriptors[i + 1],
               FullResourceDescriptor->PartialResourceList.PartialDescriptors[i].u.DeviceSpecificData.DataSize);
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}


#define ROOT_NAME   L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter"

static VOID
EnumerateBiosDiskEntries(
    PPARTLIST PartList)
{
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    WCHAR Name[120];
    ULONG AdapterCount;
    ULONG DiskCount;
    NTSTATUS Status;
    PCM_INT13_DRIVE_PARAMETER Int13Drives;
    PBIOSDISKENTRY BiosDiskEntry;

    memset(QueryTable, 0, sizeof(QueryTable));

    QueryTable[1].Name = L"Configuration Data";
    QueryTable[1].QueryRoutine = SystemConfigurationDataQueryRoutine;
    Int13Drives = NULL;
    Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System",
                                    &QueryTable[1],
                                    (PVOID)&Int13Drives,
                                    NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Unable to query the 'Configuration Data' key in '\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System', status=%lx\n", Status);
        return;
    }

    AdapterCount = 0;
    while (1)
    {
        swprintf(Name, L"%s\\%lu", ROOT_NAME, AdapterCount);
        Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        Name,
                                        &QueryTable[2],
                                        NULL,
                                        NULL);
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        swprintf(Name, L"%s\\%lu\\DiskController", ROOT_NAME, AdapterCount);
        Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        Name,
                                        &QueryTable[2],
                                        NULL,
                                        NULL);
        if (NT_SUCCESS(Status))
        {
            while (1)
            {
                swprintf(Name, L"%s\\%lu\\DiskController\\0", ROOT_NAME, AdapterCount);
                Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                                Name,
                                                &QueryTable[2],
                                                NULL,
                                                NULL);
                if (!NT_SUCCESS(Status))
                {
                    RtlFreeHeap(ProcessHeap, 0, Int13Drives);
                    return;
                }

                swprintf(Name, L"%s\\%lu\\DiskController\\0\\DiskPeripheral", ROOT_NAME, AdapterCount);
                Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                                Name,
                                                &QueryTable[2],
                                                NULL,
                                                NULL);
                if (NT_SUCCESS(Status))
                {
                    QueryTable[0].Name = L"Identifier";
                    QueryTable[0].QueryRoutine = DiskIdentifierQueryRoutine;
                    QueryTable[1].Name = L"Configuration Data";
                    QueryTable[1].QueryRoutine = DiskConfigurationDataQueryRoutine;

                    DiskCount = 0;
                    while (1)
                    {
                        BiosDiskEntry = (BIOSDISKENTRY*) RtlAllocateHeap(ProcessHeap, HEAP_ZERO_MEMORY, sizeof(BIOSDISKENTRY));
                        if (BiosDiskEntry == NULL)
                        {
                            break;
                        }

                        swprintf(Name, L"%s\\%lu\\DiskController\\0\\DiskPeripheral\\%lu", ROOT_NAME, AdapterCount, DiskCount);
                        Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                                        Name,
                                                        QueryTable,
                                                        (PVOID)BiosDiskEntry,
                                                        NULL);
                        if (!NT_SUCCESS(Status))
                        {
                            RtlFreeHeap(ProcessHeap, 0, BiosDiskEntry);
                            break;
                        }

                        BiosDiskEntry->DiskNumber = DiskCount;
                        BiosDiskEntry->Recognized = FALSE;

                        if (DiskCount < Int13Drives[0].NumberDrives)
                        {
                            BiosDiskEntry->Int13DiskData = Int13Drives[DiskCount];
                        }
                        else
                        {
                            DPRINT1("Didn't find int13 drive datas for disk %u\n", DiskCount);
                        }

                        InsertTailList(&PartList->BiosDiskListHead, &BiosDiskEntry->ListEntry);

                        DPRINT("DiskNumber:        %lu\n", BiosDiskEntry->DiskNumber);
                        DPRINT("Signature:         %08lx\n", BiosDiskEntry->Signature);
                        DPRINT("Checksum:          %08lx\n", BiosDiskEntry->Checksum);
                        DPRINT("BytesPerSector:    %lu\n", BiosDiskEntry->DiskGeometry.BytesPerSector);
                        DPRINT("NumberOfCylinders: %lu\n", BiosDiskEntry->DiskGeometry.NumberOfCylinders);
                        DPRINT("NumberOfHeads:     %lu\n", BiosDiskEntry->DiskGeometry.NumberOfHeads);
                        DPRINT("DriveSelect:       %02x\n", BiosDiskEntry->Int13DiskData.DriveSelect);
                        DPRINT("MaxCylinders:      %lu\n", BiosDiskEntry->Int13DiskData.MaxCylinders);
                        DPRINT("SectorsPerTrack:   %d\n", BiosDiskEntry->Int13DiskData.SectorsPerTrack);
                        DPRINT("MaxHeads:          %d\n", BiosDiskEntry->Int13DiskData.MaxHeads);
                        DPRINT("NumberDrives:      %d\n", BiosDiskEntry->Int13DiskData.NumberDrives);

                        DiskCount++;
                    }
                }

                RtlFreeHeap(ProcessHeap, 0, Int13Drives);
                return;
            }
        }

        AdapterCount++;
    }

    RtlFreeHeap(ProcessHeap, 0, Int13Drives);
}


static
VOID
AddPartitionToDisk(
    ULONG DiskNumber,
    PDISKENTRY DiskEntry,
    ULONG PartitionIndex,
    BOOLEAN LogicalPartition)
{
    PPARTITION_INFORMATION PartitionInfo;
    PPARTENTRY PartEntry;

    PartitionInfo = &DiskEntry->LayoutBuffer->PartitionEntry[PartitionIndex];
    if (PartitionInfo->PartitionType == 0 ||
        (LogicalPartition == TRUE && IsContainerPartition(PartitionInfo->PartitionType)))
        return;

    PartEntry = RtlAllocateHeap(ProcessHeap,
                                HEAP_ZERO_MEMORY,
                                sizeof(PARTENTRY));
    if (PartEntry == NULL)
    {
        return;
    }

    PartEntry->DiskEntry = DiskEntry;

    PartEntry->StartSector.QuadPart = (ULONGLONG)PartitionInfo->StartingOffset.QuadPart / DiskEntry->BytesPerSector;
    PartEntry->SectorCount.QuadPart = (ULONGLONG)PartitionInfo->PartitionLength.QuadPart / DiskEntry->BytesPerSector;

    PartEntry->BootIndicator = PartitionInfo->BootIndicator;
    PartEntry->PartitionType = PartitionInfo->PartitionType;
    PartEntry->HiddenSectors = PartitionInfo->HiddenSectors;

    PartEntry->LogicalPartition = LogicalPartition;
    PartEntry->IsPartitioned = TRUE;
    PartEntry->PartitionNumber = PartitionInfo->PartitionNumber;
    PartEntry->PartitionIndex = PartitionIndex;

    if (IsContainerPartition(PartEntry->PartitionType))
    {
        PartEntry->FormatState = Unformatted;

        if (LogicalPartition == FALSE && DiskEntry->ExtendedPartition == NULL)
            DiskEntry->ExtendedPartition = PartEntry;
    }
    else if ((PartEntry->PartitionType == PARTITION_FAT_12) ||
             (PartEntry->PartitionType == PARTITION_FAT_16) ||
             (PartEntry->PartitionType == PARTITION_HUGE) ||
             (PartEntry->PartitionType == PARTITION_XINT13) ||
             (PartEntry->PartitionType == PARTITION_FAT32) ||
             (PartEntry->PartitionType == PARTITION_FAT32_XINT13))
    {
#if 0
        if (CheckFatFormat())
        {
            PartEntry->FormatState = Preformatted;
        }
        else
        {
            PartEntry->FormatState = Unformatted;
        }
#endif
        PartEntry->FormatState = Preformatted;
    }
    else if (PartEntry->PartitionType == PARTITION_EXT2)
    {
#if 0
        if (CheckExt2Format())
        {
            PartEntry->FormatState = Preformatted;
        }
        else
        {
            PartEntry->FormatState = Unformatted;
        }
#endif
        PartEntry->FormatState = Preformatted;
    }
    else if (PartEntry->PartitionType == PARTITION_IFS)
    {
#if 0
        if (CheckNtfsFormat())
        {
            PartEntry->FormatState = Preformatted;
        }
        else if (CheckHpfsFormat())
        {
            PartEntry->FormatState = Preformatted;
        }
        else
        {
            PartEntry->FormatState = Unformatted;
        }
#endif
        PartEntry->FormatState = Preformatted;
    }
    else
    {
        PartEntry->FormatState = UnknownFormat;
    }

    if (LogicalPartition)
        InsertTailList(&DiskEntry->LogicalPartListHead,
                       &PartEntry->ListEntry);
    else
        InsertTailList(&DiskEntry->PrimaryPartListHead,
                       &PartEntry->ListEntry);
}


static
VOID
ScanForUnpartitionedDiskSpace(
    PDISKENTRY DiskEntry)
{
    ULONGLONG LastStartSector;
    ULONGLONG LastSectorCount;
    ULONGLONG LastUnusedSectorCount;
    PPARTENTRY PartEntry;
    PPARTENTRY NewPartEntry;
    PLIST_ENTRY Entry;

    DPRINT("ScanForUnpartitionedDiskSpace()\n");

    if (IsListEmpty(&DiskEntry->PrimaryPartListHead))
    {
        DPRINT1("No primary partition!\n");

        /* Create a partition table that represents the empty disk */
        NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                       HEAP_ZERO_MEMORY,
                                       sizeof(PARTENTRY));
        if (NewPartEntry == NULL)
            return;

        NewPartEntry->DiskEntry = DiskEntry;

        NewPartEntry->IsPartitioned = FALSE;
        NewPartEntry->StartSector.QuadPart = (ULONGLONG)DiskEntry->SectorsPerTrack;
        NewPartEntry->SectorCount.QuadPart = Align(DiskEntry->SectorCount.QuadPart, DiskEntry->SectorAlignment) -
                                             DiskEntry->SectorsPerTrack;
DPRINT1("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
DPRINT1("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
DPRINT1("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

        NewPartEntry->FormatState = Unformatted;

        InsertTailList(&DiskEntry->PrimaryPartListHead,
                       &NewPartEntry->ListEntry);

        return;
    }

    /* Start partition at head 1, cylinder 0 */
    LastStartSector = DiskEntry->SectorsPerTrack;
    LastSectorCount = 0ULL;
    LastUnusedSectorCount = 0ULL;

    Entry = DiskEntry->PrimaryPartListHead.Flink;
    while (Entry != &DiskEntry->PrimaryPartListHead)
    {
        PartEntry = CONTAINING_RECORD(Entry, PARTENTRY, ListEntry);

        if (PartEntry->PartitionType != PARTITION_ENTRY_UNUSED ||
            PartEntry->SectorCount.QuadPart != 0ULL)
        {
            LastUnusedSectorCount =
                PartEntry->StartSector.QuadPart - (LastStartSector + LastSectorCount);

            if (PartEntry->StartSector.QuadPart > (LastStartSector + LastSectorCount) &&
                LastUnusedSectorCount >= (ULONGLONG)DiskEntry->SectorAlignment)
            {
                DPRINT("Unpartitioned disk space %I64u sectors\n", LastUnusedSectorCount);

                NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                               HEAP_ZERO_MEMORY,
                                               sizeof(PARTENTRY));
                if (NewPartEntry == NULL)
                    return;

                NewPartEntry->DiskEntry = DiskEntry;

                NewPartEntry->IsPartitioned = FALSE;
                NewPartEntry->StartSector.QuadPart = LastStartSector + LastSectorCount;
                NewPartEntry->SectorCount.QuadPart = Align(NewPartEntry->StartSector.QuadPart + LastUnusedSectorCount, DiskEntry->SectorAlignment) -
                                                     NewPartEntry->StartSector.QuadPart;
DPRINT1("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
DPRINT1("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
DPRINT1("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

                NewPartEntry->FormatState = Unformatted;

                /* Insert the table into the list */
                InsertTailList(&PartEntry->ListEntry,
                               &NewPartEntry->ListEntry);
            }

            LastStartSector = PartEntry->StartSector.QuadPart;
            LastSectorCount = PartEntry->SectorCount.QuadPart;
        }

        Entry = Entry->Flink;
    }

    /* Check for trailing unpartitioned disk space */
    if ((LastStartSector + LastSectorCount) < DiskEntry->SectorCount.QuadPart)
    {
        LastUnusedSectorCount = Align(DiskEntry->SectorCount.QuadPart - (LastStartSector + LastSectorCount), DiskEntry->SectorAlignment);

        if (LastUnusedSectorCount >= (ULONGLONG)DiskEntry->SectorAlignment)
        {
            DPRINT("Unpartitioned disk space: %I64u sectors\n", LastUnusedSectorCount);

            NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                           HEAP_ZERO_MEMORY,
                                           sizeof(PARTENTRY));
            if (NewPartEntry == NULL)
                return;

            NewPartEntry->DiskEntry = DiskEntry;

            NewPartEntry->IsPartitioned = FALSE;
            NewPartEntry->StartSector.QuadPart = LastStartSector + LastSectorCount;
            NewPartEntry->SectorCount.QuadPart = Align(NewPartEntry->StartSector.QuadPart + LastUnusedSectorCount, DiskEntry->SectorAlignment) -
                                                 NewPartEntry->StartSector.QuadPart;
            DPRINT("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
            DPRINT("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
            DPRINT("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

            NewPartEntry->FormatState = Unformatted;

            /* Append the table to the list */
            InsertTailList(&DiskEntry->PrimaryPartListHead,
                           &NewPartEntry->ListEntry);
        }
    }

    if (DiskEntry->ExtendedPartition != NULL)
    {
        if (IsListEmpty(&DiskEntry->LogicalPartListHead))
        {
            DPRINT1("No logical partition!\n");

            /* Create a partition table entry that represents the empty extended partition */
            NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                           HEAP_ZERO_MEMORY,
                                           sizeof(PARTENTRY));
            if (NewPartEntry == NULL)
                return;

            NewPartEntry->DiskEntry = DiskEntry;
            NewPartEntry->LogicalPartition = TRUE;

            NewPartEntry->IsPartitioned = FALSE;
            NewPartEntry->StartSector.QuadPart = DiskEntry->ExtendedPartition->StartSector.QuadPart + (ULONGLONG)DiskEntry->SectorsPerTrack;
            NewPartEntry->SectorCount.QuadPart = DiskEntry->ExtendedPartition->SectorCount.QuadPart - (ULONGLONG)DiskEntry->SectorsPerTrack;

            DPRINT1("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
            DPRINT1("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
            DPRINT1("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

            NewPartEntry->FormatState = Unformatted;

            InsertTailList(&DiskEntry->LogicalPartListHead,
                           &NewPartEntry->ListEntry);

            return;
        }

        /* Start partition at head 1, cylinder 0 */
        LastStartSector = DiskEntry->ExtendedPartition->StartSector.QuadPart + (ULONGLONG)DiskEntry->SectorsPerTrack;
        LastSectorCount = 0ULL;
        LastUnusedSectorCount = 0ULL;

        Entry = DiskEntry->LogicalPartListHead.Flink;
        while (Entry != &DiskEntry->LogicalPartListHead)
        {
            PartEntry = CONTAINING_RECORD(Entry, PARTENTRY, ListEntry);

            if (PartEntry->PartitionType != PARTITION_ENTRY_UNUSED ||
                PartEntry->SectorCount.QuadPart != 0ULL)
            {
                LastUnusedSectorCount =
                    PartEntry->StartSector.QuadPart - (ULONGLONG)DiskEntry->SectorsPerTrack - (LastStartSector + LastSectorCount);

                if ((PartEntry->StartSector.QuadPart - (ULONGLONG)DiskEntry->SectorsPerTrack) > (LastStartSector + LastSectorCount) &&
                    LastUnusedSectorCount >= (ULONGLONG)DiskEntry->SectorAlignment)
                {
                    DPRINT("Unpartitioned disk space %I64u sectors\n", LastUnusedSectorCount);

                    NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                                   HEAP_ZERO_MEMORY,
                                                   sizeof(PARTENTRY));
                    if (NewPartEntry == NULL)
                        return;

                    NewPartEntry->DiskEntry = DiskEntry;
                    NewPartEntry->LogicalPartition = TRUE;

                    NewPartEntry->IsPartitioned = FALSE;
                    NewPartEntry->StartSector.QuadPart = LastStartSector + LastSectorCount;
                    NewPartEntry->SectorCount.QuadPart = Align(NewPartEntry->StartSector.QuadPart + LastUnusedSectorCount, DiskEntry->SectorAlignment) -
                                                         NewPartEntry->StartSector.QuadPart;
                    DPRINT("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
                    DPRINT("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
                    DPRINT("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

                    NewPartEntry->FormatState = Unformatted;

                    /* Insert the table into the list */
                    InsertTailList(&PartEntry->ListEntry,
                                   &NewPartEntry->ListEntry);
                }

                LastStartSector = PartEntry->StartSector.QuadPart;
                LastSectorCount = PartEntry->SectorCount.QuadPart;
            }

            Entry = Entry->Flink;
        }

        /* Check for trailing unpartitioned disk space */
        if ((LastStartSector + LastSectorCount) < DiskEntry->ExtendedPartition->StartSector.QuadPart + DiskEntry->ExtendedPartition->SectorCount.QuadPart)
        {
            LastUnusedSectorCount = Align(DiskEntry->ExtendedPartition->StartSector.QuadPart + DiskEntry->ExtendedPartition->SectorCount.QuadPart - (LastStartSector + LastSectorCount), DiskEntry->SectorAlignment);

            if (LastUnusedSectorCount >= (ULONGLONG)DiskEntry->SectorAlignment)
            {
                DPRINT("Unpartitioned disk space: %I64u sectors\n", LastUnusedSectorCount);

                NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                               HEAP_ZERO_MEMORY,
                                               sizeof(PARTENTRY));
                if (NewPartEntry == NULL)
                    return;

                NewPartEntry->DiskEntry = DiskEntry;
                NewPartEntry->LogicalPartition = TRUE;

                NewPartEntry->IsPartitioned = FALSE;
                NewPartEntry->StartSector.QuadPart = LastStartSector + LastSectorCount;
                NewPartEntry->SectorCount.QuadPart = Align(NewPartEntry->StartSector.QuadPart + LastUnusedSectorCount, DiskEntry->SectorAlignment) -
                                                     NewPartEntry->StartSector.QuadPart;
                DPRINT("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
                DPRINT("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
                DPRINT("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

                NewPartEntry->FormatState = Unformatted;

                /* Append the table to the list */
                InsertTailList(&DiskEntry->LogicalPartListHead,
                               &NewPartEntry->ListEntry);
            }
        }
    }

    DPRINT("ScanForUnpartitionedDiskSpace() done\n");
}


static
VOID
SetDiskSignature(
    IN PPARTLIST List,
    IN PDISKENTRY DiskEntry)
{
    LARGE_INTEGER SystemTime;
    TIME_FIELDS TimeFields;
    PLIST_ENTRY Entry2;
    PDISKENTRY DiskEntry2;
    PUCHAR Buffer;

    Buffer = (PUCHAR)&DiskEntry->LayoutBuffer->Signature;

    while (1)
    {
        NtQuerySystemTime(&SystemTime);
        RtlTimeToTimeFields(&SystemTime, &TimeFields);

        Buffer[0] = (UCHAR)(TimeFields.Year & 0xFF) + (UCHAR)(TimeFields.Hour & 0xFF);
        Buffer[1] = (UCHAR)(TimeFields.Year >> 8) + (UCHAR)(TimeFields.Minute & 0xFF);
        Buffer[2] = (UCHAR)(TimeFields.Month & 0xFF) + (UCHAR)(TimeFields.Second & 0xFF);
        Buffer[3] = (UCHAR)(TimeFields.Day & 0xFF) + (UCHAR)(TimeFields.Milliseconds & 0xFF);

        if (DiskEntry->LayoutBuffer->Signature == 0)
        {
            continue;
        }

        /* check if the signature already exist */
        /* FIXME:
         *   Check also signatures from disks, which are
         *   not visible (bootable) by the bios.
         */
        Entry2 = List->DiskListHead.Flink;
        while (Entry2 != &List->DiskListHead)
        {
            DiskEntry2 = CONTAINING_RECORD(Entry2, DISKENTRY, ListEntry);

            if (DiskEntry != DiskEntry2 &&
                DiskEntry->LayoutBuffer->Signature == DiskEntry2->LayoutBuffer->Signature)
                break;

            Entry2 = Entry2->Flink;
        }

        if (Entry2 == &List->DiskListHead)
            break;
    }
}


static
VOID
UpdateDiskSignatures(
    PPARTLIST List)
{
    PLIST_ENTRY Entry;
    PDISKENTRY DiskEntry;

    /* Print partition lines*/
    Entry = List->DiskListHead.Flink;
    while (Entry != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry, DISKENTRY, ListEntry);

        if (DiskEntry->LayoutBuffer &&
            DiskEntry->LayoutBuffer->Signature == 0)
        {
            SetDiskSignature(List, DiskEntry);
            DiskEntry->LayoutBuffer->PartitionEntry[0].RewritePartition = TRUE;
        }

        Entry = Entry->Flink;
    }
}


static
VOID
AddDiskToList(
    HANDLE FileHandle,
    ULONG DiskNumber,
    PPARTLIST List)
{
    DISK_GEOMETRY DiskGeometry;
    SCSI_ADDRESS ScsiAddress;
    PDISKENTRY DiskEntry;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;
    PPARTITION_SECTOR Mbr;
    PULONG Buffer;
    LARGE_INTEGER FileOffset;
    WCHAR Identifier[20];
    ULONG Checksum;
    ULONG Signature;
    ULONG i;
    PLIST_ENTRY ListEntry;
    PBIOSDISKENTRY BiosDiskEntry;
    ULONG LayoutBufferSize;

    Status = NtDeviceIoControlFile(FileHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &Iosb,
                                   IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                   NULL,
                                   0,
                                   &DiskGeometry,
                                   sizeof(DISK_GEOMETRY));
    if (!NT_SUCCESS(Status))
    {
        return;
    }

    if (DiskGeometry.MediaType != FixedMedia &&
        DiskGeometry.MediaType != RemovableMedia)
    {
        return;
    }

    Status = NtDeviceIoControlFile(FileHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &Iosb,
                                   IOCTL_SCSI_GET_ADDRESS,
                                   NULL,
                                   0,
                                   &ScsiAddress,
                                   sizeof(SCSI_ADDRESS));
    if (!NT_SUCCESS(Status))
    {
        return;
    }

    Mbr = (PARTITION_SECTOR*)RtlAllocateHeap(ProcessHeap,
                                             0,
                                             DiskGeometry.BytesPerSector);
    if (Mbr == NULL)
    {
        return;
    }

    FileOffset.QuadPart = 0;
    Status = NtReadFile(FileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &Iosb,
                        (PVOID)Mbr,
                        DiskGeometry.BytesPerSector,
                        &FileOffset,
                        NULL);
    if (!NT_SUCCESS(Status))
    {
        RtlFreeHeap(ProcessHeap,
                    0,
                    Mbr);
        DPRINT1("NtReadFile failed, status=%x\n", Status);
        return;
    }
    Signature = Mbr->Signature;

    /* Calculate the MBR checksum */
    Checksum = 0;
    Buffer = (PULONG)Mbr;
    for (i = 0; i < 128; i++)
    {
        Checksum += Buffer[i];
    }
    Checksum = ~Checksum + 1;

    swprintf(Identifier, L"%08x-%08x-A", Checksum, Signature);
    DPRINT("Identifier: %S\n", Identifier);

    DiskEntry = RtlAllocateHeap(ProcessHeap,
                                HEAP_ZERO_MEMORY,
                                sizeof(DISKENTRY));
    if (DiskEntry == NULL)
    {
        return;
    }

//    DiskEntry->Checksum = Checksum;
//    DiskEntry->Signature = Signature;
    DiskEntry->BiosFound = FALSE;

    /* Check if this disk has a valid MBR */
    if (Mbr->BootCode[0] == 0 && Mbr->BootCode[1] == 0)
        DiskEntry->NoMbr = TRUE;
    else
        DiskEntry->NoMbr = FALSE;

    /* Free Mbr sector buffer */
    RtlFreeHeap(ProcessHeap,
                0,
                Mbr);

    ListEntry = List->BiosDiskListHead.Flink;
    while(ListEntry != &List->BiosDiskListHead)
    {
        BiosDiskEntry = CONTAINING_RECORD(ListEntry, BIOSDISKENTRY, ListEntry);
        /* FIXME:
         *   Compare the size from bios and the reported size from driver.
         *   If we have more than one disk with a zero or with the same signatur
         *   we must create new signatures and reboot. After the reboot,
         *   it is possible to identify the disks.
         */
        if (BiosDiskEntry->Signature == Signature &&
            BiosDiskEntry->Checksum == Checksum &&
            !BiosDiskEntry->Recognized)
        {
            if (!DiskEntry->BiosFound)
            {
                DiskEntry->BiosDiskNumber = BiosDiskEntry->DiskNumber;
                DiskEntry->BiosFound = TRUE;
                BiosDiskEntry->Recognized = TRUE;
            }
            else
            {
            }
        }
        ListEntry = ListEntry->Flink;
    }

    if (!DiskEntry->BiosFound)
    {
#if 0
        RtlFreeHeap(ProcessHeap, 0, DiskEntry);
        return;
#else
        DPRINT1("WARNING: Setup could not find a matching BIOS disk entry. Disk %d is not be bootable by the BIOS!\n", DiskNumber);
#endif
    }

    InitializeListHead(&DiskEntry->PrimaryPartListHead);
    InitializeListHead(&DiskEntry->LogicalPartListHead);

    DiskEntry->Cylinders = DiskGeometry.Cylinders.QuadPart;
    DiskEntry->TracksPerCylinder = DiskGeometry.TracksPerCylinder;
    DiskEntry->SectorsPerTrack = DiskGeometry.SectorsPerTrack;
    DiskEntry->BytesPerSector = DiskGeometry.BytesPerSector;

    DPRINT("Cylinders %I64u\n", DiskEntry->Cylinders);
    DPRINT("TracksPerCylinder %I64u\n", DiskEntry->TracksPerCylinder);
    DPRINT("SectorsPerTrack %I64u\n", DiskEntry->SectorsPerTrack);
    DPRINT("BytesPerSector %I64u\n", DiskEntry->BytesPerSector);

    DiskEntry->SectorCount.QuadPart = DiskGeometry.Cylinders.QuadPart *
                                      (ULONGLONG)DiskGeometry.TracksPerCylinder *
                                      (ULONGLONG)DiskGeometry.SectorsPerTrack;

    DiskEntry->SectorAlignment = DiskGeometry.SectorsPerTrack;

    DPRINT("SectorCount %I64u\n", DiskEntry->SectorCount);
    DPRINT("SectorAlignment %lu\n", DiskEntry->SectorAlignment);

    DiskEntry->DiskNumber = DiskNumber;
    DiskEntry->Port = ScsiAddress.PortNumber;
    DiskEntry->Bus = ScsiAddress.PathId;
    DiskEntry->Id = ScsiAddress.TargetId;

    GetDriverName(DiskEntry);

    InsertAscendingList(&List->DiskListHead, DiskEntry, DISKENTRY, ListEntry, DiskNumber);

    /*
     * Allocate a buffer for 26 logical drives (2 entries each == 52)
     * plus the main partiton table (4 entries). Total 56 entries.
     */
    LayoutBufferSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                       ((56 - ANYSIZE_ARRAY) * sizeof(PARTITION_INFORMATION));
    DiskEntry->LayoutBuffer = RtlAllocateHeap(ProcessHeap,
                                              HEAP_ZERO_MEMORY,
                                              LayoutBufferSize);
    if (DiskEntry->LayoutBuffer == NULL)
    {
        return;
    }

    Status = NtDeviceIoControlFile(FileHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &Iosb,
                                   IOCTL_DISK_GET_DRIVE_LAYOUT,
                                   NULL,
                                   0,
                                   DiskEntry->LayoutBuffer,
                                   LayoutBufferSize);
    if (NT_SUCCESS(Status))
    {
#ifdef DUMP_PARTITION_TABLE
        DumpPartitionTable(DiskEntry);
#endif

        if (DiskEntry->LayoutBuffer->PartitionEntry[0].StartingOffset.QuadPart != 0 &&
            DiskEntry->LayoutBuffer->PartitionEntry[0].PartitionLength.QuadPart != 0 &&
            DiskEntry->LayoutBuffer->PartitionEntry[0].PartitionType != 0)
        {
            if ((DiskEntry->LayoutBuffer->PartitionEntry[0].StartingOffset.QuadPart / DiskEntry->BytesPerSector) % DiskEntry->SectorsPerTrack == 0)
            {
                DPRINT("Use %lu Sector alignment!\n", DiskEntry->SectorsPerTrack);
            }
            else if (DiskEntry->LayoutBuffer->PartitionEntry[0].StartingOffset.QuadPart % 1048756 == 0)
            {
                DPRINT1("Use megabyte (%lu Sectors) alignment!\n", 1048756 / DiskEntry->BytesPerSector);
            }
            else
            {
                DPRINT1("No matching aligment found! Partiton 1 starts at %I64u\n", DiskEntry->LayoutBuffer->PartitionEntry[0].StartingOffset.QuadPart);
            }
        }
        else
        {
            DPRINT1("No valid partiton table found! Use megabyte (%lu Sectors) alignment!\n", 1048756 / DiskEntry->BytesPerSector);
        }


        if (DiskEntry->LayoutBuffer->PartitionCount == 0)
        {
            DiskEntry->NewDisk = TRUE;
            DiskEntry->LayoutBuffer->PartitionCount = 4;

            for (i = 0; i < 4; i++)
                DiskEntry->LayoutBuffer->PartitionEntry[i].RewritePartition = TRUE;
        }
        else
        {
            for (i = 0; i < 4; i++)
            {
                AddPartitionToDisk(DiskNumber,
                                   DiskEntry,
                                   i,
                                   FALSE);
            }

            for (i = 4; i < DiskEntry->LayoutBuffer->PartitionCount; i += 4)
            {
                AddPartitionToDisk(DiskNumber,
                                   DiskEntry,
                                   i,
                                   TRUE);
            }
        }
    }

    ScanForUnpartitionedDiskSpace(DiskEntry);
}


PPARTLIST
CreatePartitionList(
    SHORT Left,
    SHORT Top,
    SHORT Right,
    SHORT Bottom)
{
    PPARTLIST List;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SYSTEM_DEVICE_INFORMATION Sdi;
    IO_STATUS_BLOCK Iosb;
    ULONG ReturnSize;
    NTSTATUS Status;
    ULONG DiskNumber;
    WCHAR Buffer[MAX_PATH];
    UNICODE_STRING Name;
    HANDLE FileHandle;

    List = (PPARTLIST)RtlAllocateHeap(ProcessHeap,
                                      0,
                                      sizeof (PARTLIST));
    if (List == NULL)
        return NULL;

    List->Left = Left;
    List->Top = Top;
    List->Right = Right;
    List->Bottom = Bottom;

    List->Line = 0;
    List->Offset = 0;

    List->TopDisk = (ULONG)-1;
    List->TopPartition = (ULONG)-1;

    List->CurrentDisk = NULL;
    List->CurrentPartition = NULL;

    InitializeListHead(&List->DiskListHead);
    InitializeListHead(&List->BiosDiskListHead);

    EnumerateBiosDiskEntries(List);

    Status = NtQuerySystemInformation(SystemDeviceInformation,
                                      &Sdi,
                                      sizeof(SYSTEM_DEVICE_INFORMATION),
                                      &ReturnSize);
    if (!NT_SUCCESS(Status))
    {
        RtlFreeHeap(ProcessHeap, 0, List);
        return NULL;
    }

    for (DiskNumber = 0; DiskNumber < Sdi.NumberOfDisks; DiskNumber++)
    {
        swprintf(Buffer,
                 L"\\Device\\Harddisk%d\\Partition0",
                 DiskNumber);
        RtlInitUnicodeString(&Name,
                             Buffer);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &Name,
                                   0,
                                   NULL,
                                   NULL);

        Status = NtOpenFile(&FileHandle,
                            FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                            &ObjectAttributes,
                            &Iosb,
                            FILE_SHARE_READ,
                            FILE_SYNCHRONOUS_IO_NONALERT);
        if (NT_SUCCESS(Status))
        {
            AddDiskToList(FileHandle,
                          DiskNumber,
                          List);

            NtClose(FileHandle);
        }
    }

    UpdateDiskSignatures(List);

    AssignDriveLetters(List);

    List->TopDisk = 0;
    List->TopPartition = 0;

    /* Search for first usable disk and partition */
    if (IsListEmpty(&List->DiskListHead))
    {
        List->CurrentDisk = NULL;
        List->CurrentPartition = NULL;
    }
    else
    {
        List->CurrentDisk = CONTAINING_RECORD(List->DiskListHead.Flink,
                                              DISKENTRY,
                                              ListEntry);

        if (IsListEmpty(&List->CurrentDisk->PrimaryPartListHead))
        {
            List->CurrentPartition = 0;
        }
        else
        {
            List->CurrentPartition = CONTAINING_RECORD(List->CurrentDisk->PrimaryPartListHead.Flink,
                                                       PARTENTRY,
                                                       ListEntry);
        }
    }

    return List;
}


VOID
DestroyPartitionList(
    PPARTLIST List)
{
    PDISKENTRY DiskEntry;
    PBIOSDISKENTRY BiosDiskEntry;
    PPARTENTRY PartEntry;
    PLIST_ENTRY Entry;

    /* Release disk and partition info */
    while (!IsListEmpty(&List->DiskListHead))
    {
        Entry = RemoveHeadList(&List->DiskListHead);
        DiskEntry = CONTAINING_RECORD(Entry, DISKENTRY, ListEntry);

        /* Release driver name */
        RtlFreeUnicodeString(&DiskEntry->DriverName);

        /* Release primary partition list */
        while (!IsListEmpty(&DiskEntry->PrimaryPartListHead))
        {
            Entry = RemoveHeadList(&DiskEntry->PrimaryPartListHead);
            PartEntry = CONTAINING_RECORD(Entry, PARTENTRY, ListEntry);

            RtlFreeHeap(ProcessHeap, 0, PartEntry);
        }

        /* Release logical partition list */
        while (!IsListEmpty(&DiskEntry->LogicalPartListHead))
        {
            Entry = RemoveHeadList(&DiskEntry->LogicalPartListHead);
            PartEntry = CONTAINING_RECORD(Entry, PARTENTRY, ListEntry);

            RtlFreeHeap(ProcessHeap, 0, PartEntry);
        }

        /* Release layout buffer */
        if (DiskEntry->LayoutBuffer != NULL)
            RtlFreeHeap(ProcessHeap, 0, DiskEntry->LayoutBuffer);


        /* Release disk entry */
        RtlFreeHeap(ProcessHeap, 0, DiskEntry);
    }

    /* release the bios disk info */
    while(!IsListEmpty(&List->BiosDiskListHead))
    {
        Entry = RemoveHeadList(&List->BiosDiskListHead);
        BiosDiskEntry = CONTAINING_RECORD(Entry, BIOSDISKENTRY, ListEntry);

        RtlFreeHeap(ProcessHeap, 0, BiosDiskEntry);
    }

    /* Release list head */
    RtlFreeHeap(ProcessHeap, 0, List);
}


static
VOID
PrintEmptyLine(
    PPARTLIST List)
{
    COORD coPos;
    DWORD Written;
    USHORT Width;
    USHORT Height;

    Width = List->Right - List->Left - 1;
    Height = List->Bottom - List->Top - 2;

    coPos.X = List->Left + 1;
    coPos.Y = List->Top + 1 + List->Line;

    if (List->Line >= 0 && List->Line <= Height)
    {
        FillConsoleOutputAttribute(StdOutput,
                                   FOREGROUND_WHITE | BACKGROUND_BLUE,
                                   Width,
                                   coPos,
                                   &Written);

        FillConsoleOutputCharacterA(StdOutput,
                                    ' ',
                                    Width,
                                    coPos,
                                    &Written);
    }

    List->Line++;
}


static
VOID
PrintPartitionData(
    PPARTLIST List,
    PDISKENTRY DiskEntry,
    PPARTENTRY PartEntry)
{
    CHAR LineBuffer[128];
    COORD coPos;
    DWORD Written;
    USHORT Width;
    USHORT Height;
    LARGE_INTEGER PartSize;
    PCHAR Unit;
    UCHAR Attribute;
    PCHAR PartType;

    Width = List->Right - List->Left - 1;
    Height = List->Bottom - List->Top - 2;

    coPos.X = List->Left + 1;
    coPos.Y = List->Top + 1 + List->Line;

    if (PartEntry->IsPartitioned == FALSE)
    {
        PartSize.QuadPart = PartEntry->SectorCount.QuadPart * DiskEntry->BytesPerSector;
#if 0
        if (PartSize.QuadPart >= 10737418240) /* 10 GB */
        {
            PartSize.QuadPart = RoundingDivide(PartSize.QuadPart, 1073741824);
            Unit = MUIGetString(STRING_GB);
        }
        else
#endif
        if (PartSize.QuadPart >= 10485760) /* 10 MB */
        {
            PartSize.QuadPart = RoundingDivide(PartSize.QuadPart, 1048576);
            Unit = MUIGetString(STRING_MB);
        }
        else
        {
            PartSize.QuadPart = RoundingDivide(PartSize.QuadPart, 1024);
            Unit = MUIGetString(STRING_KB);
        }

        sprintf(LineBuffer,
                MUIGetString(STRING_UNPSPACE),
                PartEntry->LogicalPartition ? "  " : "",
                PartEntry->LogicalPartition ? "" : "  ",
                PartSize.u.LowPart,
                Unit);
    }
    else
    {
        /* Determine partition type */
        PartType = NULL;
        if (PartEntry->New == TRUE)
        {
            PartType = MUIGetString(STRING_UNFORMATTED);
        }
        else if (PartEntry->IsPartitioned == TRUE)
        {
            if ((PartEntry->PartitionType == PARTITION_FAT_12) ||
                (PartEntry->PartitionType == PARTITION_FAT_16) ||
                (PartEntry->PartitionType == PARTITION_HUGE) ||
                (PartEntry->PartitionType == PARTITION_XINT13))
            {
                PartType = "FAT";
            }
            else if ((PartEntry->PartitionType == PARTITION_FAT32) ||
                     (PartEntry->PartitionType == PARTITION_FAT32_XINT13))
            {
                PartType = "FAT32";
            }
            else if (PartEntry->PartitionType == PARTITION_EXT2)
            {
                PartType = "EXT2";
            }
            else if (PartEntry->PartitionType == PARTITION_IFS)
            {
                PartType = "NTFS"; /* FIXME: Not quite correct! */
            }
            else if ((PartEntry->PartitionType == PARTITION_EXTENDED) ||
                     (PartEntry->PartitionType == PARTITION_XINT13_EXTENDED))
            {
                PartType = MUIGetString(STRING_EXTENDED_PARTITION);
            }
        }

        PartSize.QuadPart = PartEntry->SectorCount.QuadPart * DiskEntry->BytesPerSector;
#if 0
        if (PartSize.QuadPart >= 10737418240) /* 10 GB */
        {
            PartSize.QuadPart = RoundingDivide(PartSize.QuadPart, 1073741824);
            Unit = MUIGetString(STRING_GB);
        }
        else
#endif
        if (PartSize.QuadPart >= 10485760) /* 10 MB */
        {
            PartSize.QuadPart = RoundingDivide(PartSize.QuadPart, 1048576);
            Unit = MUIGetString(STRING_MB);
        }
        else
        {
            PartSize.QuadPart = RoundingDivide(PartSize.QuadPart, 1024);
            Unit = MUIGetString(STRING_KB);
        }

        if (PartType == NULL)
        {
            sprintf(LineBuffer,
                    MUIGetString(STRING_HDDINFOUNK5),
                    (PartEntry->DriveLetter == 0) ? '-' : PartEntry->DriveLetter,
                    (PartEntry->DriveLetter == 0) ? '-' : ':',
                    PartEntry->BootIndicator ? '*' : ' ',
                    PartEntry->LogicalPartition ? "  " : "",
                    PartEntry->PartitionType,
                    PartEntry->LogicalPartition ? "" : "  ",
                    PartSize.u.LowPart,
                    Unit);
        }
        else
        {
            sprintf(LineBuffer,
                    "%c%c %c %s%-24s%s     %6lu %s",
                    (PartEntry->DriveLetter == 0) ? '-' : PartEntry->DriveLetter,
                    (PartEntry->DriveLetter == 0) ? '-' : ':',
                    PartEntry->BootIndicator ? '*' : ' ',
                    PartEntry->LogicalPartition ? "  " : "",
                    PartType,
                    PartEntry->LogicalPartition ? "" : "  ",
                    PartSize.u.LowPart,
                    Unit);
        }
    }

    Attribute = (List->CurrentDisk == DiskEntry &&
                 List->CurrentPartition == PartEntry) ?
                 FOREGROUND_BLUE | BACKGROUND_WHITE :
                 FOREGROUND_WHITE | BACKGROUND_BLUE;

    if (List->Line >= 0 && List->Line <= Height)
    {
        FillConsoleOutputCharacterA(StdOutput,
                                    ' ',
                                    Width,
                                    coPos,
                                    &Written);
    }
    coPos.X += 4;
    Width -= 8;
    if (List->Line >= 0 && List->Line <= Height)
    {
        FillConsoleOutputAttribute(StdOutput,
                                   Attribute,
                                   Width,
                                   coPos,
                                   &Written);
    }
    coPos.X++;
    Width -= 2;
    if (List->Line >= 0 && List->Line <= Height)
    {
        WriteConsoleOutputCharacterA(StdOutput,
                                     LineBuffer,
                                     min(strlen(LineBuffer), Width),
                                     coPos,
                                     &Written);
    }

    List->Line++;
}


static
VOID
PrintDiskData(
    PPARTLIST List,
    PDISKENTRY DiskEntry)
{
    PPARTENTRY PrimaryPartEntry, LogicalPartEntry;
    PLIST_ENTRY PrimaryEntry, LogicalEntry;
    CHAR LineBuffer[128];
    COORD coPos;
    DWORD Written;
    USHORT Width;
    USHORT Height;
    ULARGE_INTEGER DiskSize;
    PCHAR Unit;

    Width = List->Right - List->Left - 1;
    Height = List->Bottom - List->Top - 2;

    coPos.X = List->Left + 1;
    coPos.Y = List->Top + 1 + List->Line;

    DiskSize.QuadPart = DiskEntry->SectorCount.QuadPart * DiskEntry->BytesPerSector;
    if (DiskSize.QuadPart >= 10737418240) /* 10 GB */
    {
        DiskSize.QuadPart = RoundingDivide(DiskSize.QuadPart, 1073741824);
        Unit = MUIGetString(STRING_GB);
    }
    else
    {
        DiskSize.QuadPart = RoundingDivide(DiskSize.QuadPart, 1048576);
        if (DiskSize.QuadPart == 0)
            DiskSize.QuadPart = 1;
        Unit = MUIGetString(STRING_MB);
    }

    if (DiskEntry->DriverName.Length > 0)
    {
        sprintf(LineBuffer,
                MUIGetString(STRING_HDINFOPARTSELECT),
                DiskSize.u.LowPart,
                Unit,
                DiskEntry->DiskNumber,
                DiskEntry->Port,
                DiskEntry->Bus,
                DiskEntry->Id,
                DiskEntry->DriverName.Buffer);
    }
    else
    {
        sprintf(LineBuffer,
                MUIGetString(STRING_HDDINFOUNK6),
                DiskSize.u.LowPart,
                Unit,
                DiskEntry->DiskNumber,
                DiskEntry->Port,
                DiskEntry->Bus,
                DiskEntry->Id);
    }

    if (List->Line >= 0 && List->Line <= Height)
    {
        FillConsoleOutputAttribute(StdOutput,
                                   FOREGROUND_WHITE | BACKGROUND_BLUE,
                                   Width,
                                   coPos,
                                   &Written);

        FillConsoleOutputCharacterA(StdOutput,
                                    ' ',
                                    Width,
                                    coPos,
                                    &Written);
    }

    coPos.X++;
    if (List->Line >= 0 && List->Line <= Height)
    {
        WriteConsoleOutputCharacterA(StdOutput,
                                     LineBuffer,
                                     min((USHORT)strlen(LineBuffer), Width - 2),
                                     coPos,
                                     &Written);
    }

    List->Line++;

    /* Print separator line */
    PrintEmptyLine(List);

    /* Print partition lines*/
    PrimaryEntry = DiskEntry->PrimaryPartListHead.Flink;
    while (PrimaryEntry != &DiskEntry->PrimaryPartListHead)
    {
        PrimaryPartEntry = CONTAINING_RECORD(PrimaryEntry, PARTENTRY, ListEntry);

        PrintPartitionData(List,
                           DiskEntry,
                           PrimaryPartEntry);

        if (IsContainerPartition(PrimaryPartEntry->PartitionType))
        {
            LogicalEntry = DiskEntry->LogicalPartListHead.Flink;
            while (LogicalEntry != &DiskEntry->LogicalPartListHead)
            {
                LogicalPartEntry = CONTAINING_RECORD(LogicalEntry, PARTENTRY, ListEntry);

                PrintPartitionData(List,
                                   DiskEntry,
                                   LogicalPartEntry);

                LogicalEntry = LogicalEntry->Flink;
            }
        }

        PrimaryEntry = PrimaryEntry->Flink;
    }

    /* Print separator line */
    PrintEmptyLine(List);
}


VOID
DrawPartitionList(
    PPARTLIST List)
{
    PLIST_ENTRY Entry, Entry2;
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry = NULL;
    COORD coPos;
    DWORD Written;
    SHORT i;
    SHORT CurrentDiskLine;
    SHORT CurrentPartLine;
    SHORT LastLine;
    BOOL CurrentPartLineFound = FALSE;
    BOOL CurrentDiskLineFound = FALSE;

    /* Calculate the line of the current disk and partition */
    CurrentDiskLine = 0;
    CurrentPartLine = 0;
    LastLine = 0;

    Entry = List->DiskListHead.Flink;
    while (Entry != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry, DISKENTRY, ListEntry);

        LastLine += 2;
        if (CurrentPartLineFound == FALSE)
        {
            CurrentPartLine += 2;
        }

        Entry2 = DiskEntry->PrimaryPartListHead.Flink;
        while (Entry2 != &DiskEntry->PrimaryPartListHead)
        {
            PartEntry = CONTAINING_RECORD(Entry2, PARTENTRY, ListEntry);
            if (PartEntry == List->CurrentPartition)
            {
                CurrentPartLineFound = TRUE;
            }

            Entry2 = Entry2->Flink;
            if (CurrentPartLineFound == FALSE)
            {
                CurrentPartLine++;
            }

            LastLine++;
        }

        if (DiskEntry == List->CurrentDisk)
        {
            CurrentDiskLineFound = TRUE;
        }

        Entry = Entry->Flink;
        if (Entry != &List->DiskListHead)
        {
            if (CurrentDiskLineFound == FALSE)
            {
                CurrentPartLine ++;
                CurrentDiskLine = CurrentPartLine;
            }

            LastLine++;
        }
        else
        {
            LastLine--;
        }
    }

    /* If it possible, make the disk name visible */
    if (CurrentPartLine < List->Offset)
    {
        List->Offset = CurrentPartLine;
    }
    else if (CurrentPartLine - List->Offset > List->Bottom - List->Top - 2)
    {
        List->Offset = CurrentPartLine - (List->Bottom - List->Top - 2);
    }

    if (CurrentDiskLine < List->Offset && CurrentPartLine - CurrentDiskLine < List->Bottom - List->Top - 2)
    {
        List->Offset = CurrentDiskLine;
    }

    /* draw upper left corner */
    coPos.X = List->Left;
    coPos.Y = List->Top;
    FillConsoleOutputCharacterA(StdOutput,
                                0xDA, // '+',
                                1,
                                coPos,
                                &Written);

    /* draw upper edge */
    coPos.X = List->Left + 1;
    coPos.Y = List->Top;
    if (List->Offset == 0)
    {
        FillConsoleOutputCharacterA(StdOutput,
                                    0xC4, // '-',
                                    List->Right - List->Left - 1,
                                    coPos,
                                    &Written);
    }
    else
    {
        FillConsoleOutputCharacterA(StdOutput,
                                    0xC4, // '-',
                                    List->Right - List->Left - 5,
                                    coPos,
                                    &Written);
        coPos.X = List->Right - 5;
        WriteConsoleOutputCharacterA(StdOutput,
                                     "(\x18)", // "(up)"
                                     3,
                                     coPos,
                                     &Written);
        coPos.X = List->Right - 2;
        FillConsoleOutputCharacterA(StdOutput,
                                    0xC4, // '-',
                                    2,
                                    coPos,
                                    &Written);
    }

    /* draw upper right corner */
    coPos.X = List->Right;
    coPos.Y = List->Top;
    FillConsoleOutputCharacterA(StdOutput,
                                0xBF, // '+',
                                1,
                                coPos,
                                &Written);

    /* draw left and right edge */
    for (i = List->Top + 1; i < List->Bottom; i++)
    {
        coPos.X = List->Left;
        coPos.Y = i;
        FillConsoleOutputCharacterA(StdOutput,
                                    0xB3, // '|',
                                    1,
                                    coPos,
                                    &Written);

        coPos.X = List->Right;
        FillConsoleOutputCharacterA(StdOutput,
                                    0xB3, //'|',
                                    1,
                                    coPos,
                                    &Written);
    }

    /* draw lower left corner */
    coPos.X = List->Left;
    coPos.Y = List->Bottom;
    FillConsoleOutputCharacterA(StdOutput,
                                0xC0, // '+',
                                1,
                                coPos,
                                &Written);

    /* draw lower edge */
    coPos.X = List->Left + 1;
    coPos.Y = List->Bottom;
    if (LastLine - List->Offset <= List->Bottom - List->Top - 2)
    {
        FillConsoleOutputCharacterA(StdOutput,
                                    0xC4, // '-',
                                    List->Right - List->Left - 1,
                                    coPos,
                                    &Written);
    }
    else
    {
        FillConsoleOutputCharacterA(StdOutput,
                                    0xC4, // '-',
                                    List->Right - List->Left - 5,
                                    coPos,
                                    &Written);
        coPos.X = List->Right - 5;
        WriteConsoleOutputCharacterA(StdOutput,
                                     "(\x19)", // "(down)"
                                     3,
                                     coPos,
                                     &Written);
       coPos.X = List->Right - 2;
       FillConsoleOutputCharacterA(StdOutput,
                                   0xC4, // '-',
                                   2,
                                   coPos,
                                   &Written);
    }

    /* draw lower right corner */
    coPos.X = List->Right;
    coPos.Y = List->Bottom;
    FillConsoleOutputCharacterA(StdOutput,
                                0xD9, // '+',
                                1,
                                coPos,
                                &Written);

    /* print list entries */
    List->Line = - List->Offset;

    Entry = List->DiskListHead.Flink;
    while (Entry != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry, DISKENTRY, ListEntry);

        /* Print disk entry */
        PrintDiskData(List,
                      DiskEntry);

        Entry = Entry->Flink;
    }
}


DWORD
SelectPartition(
    PPARTLIST List,
    ULONG DiskNumber,
    ULONG PartitionNumber)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PLIST_ENTRY Entry1;
    PLIST_ENTRY Entry2;

    /* Check for empty disks */
    if (IsListEmpty(&List->DiskListHead))
        return FALSE;

    /* Check for first usable entry on next disk */
    Entry1 = List->CurrentDisk->ListEntry.Flink;
    while (Entry1 != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry1, DISKENTRY, ListEntry);

        if (DiskEntry->DiskNumber == DiskNumber)
        {
            Entry2 = DiskEntry->PrimaryPartListHead.Flink;
            while (Entry2 != &DiskEntry->PrimaryPartListHead)
            {
                PartEntry = CONTAINING_RECORD(Entry2, PARTENTRY, ListEntry);

                if (PartEntry->PartitionNumber == PartitionNumber)
                {
                     List->CurrentDisk = DiskEntry;
                     List->CurrentPartition = PartEntry;
                     DrawPartitionList(List);
                     return TRUE;
                }

                Entry2 = Entry2->Flink;
            }

            return FALSE;
        }

        Entry1 = Entry1->Flink;
    }

    return FALSE;
}


BOOL
ScrollDownPartitionList(
    PPARTLIST List)
{
    PLIST_ENTRY DiskListEntry;
    PLIST_ENTRY PartListEntry;
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;

    /* Fail, if no disks are available */
    if (IsListEmpty(&List->DiskListHead))
        return FALSE;

    /* Check for next usable entry on current disk */
    if (List->CurrentPartition != NULL)
    {
        if (List->CurrentPartition->LogicalPartition)
        {
            /* Logical partition */

            PartListEntry = List->CurrentPartition->ListEntry.Flink;
            if (PartListEntry != &List->CurrentDisk->LogicalPartListHead)
            {
                /* Next logical partition */
                PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

                List->CurrentPartition = PartEntry;
                return TRUE;
            }
            else
            {
                PartListEntry = List->CurrentDisk->ExtendedPartition->ListEntry.Flink;
                if (PartListEntry != &List->CurrentDisk->PrimaryPartListHead)
                {
                    PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

                    List->CurrentPartition = PartEntry;
                    return TRUE;
                }
            }
        }
        else
        {
            /* Primary or extended partition */

            if (List->CurrentPartition->IsPartitioned == TRUE &&
                IsContainerPartition(List->CurrentPartition->PartitionType))
            {
                /* First logical partition */
                PartListEntry = List->CurrentDisk->LogicalPartListHead.Flink;
                if (PartListEntry != &List->CurrentDisk->LogicalPartListHead)
                {
                    PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

                    List->CurrentPartition = PartEntry;
                    return TRUE;
                }
            }
            else
            {
                /* Next primary partition */
                PartListEntry = List->CurrentPartition->ListEntry.Flink;
                if (PartListEntry != &List->CurrentDisk->PrimaryPartListHead)
                {
                    PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

                    List->CurrentPartition = PartEntry;
                    return TRUE;
                }
            }
        }
    }

    /* Search for the first partition entry on the next disk */
    DiskListEntry = List->CurrentDisk->ListEntry.Flink;
    while (DiskListEntry != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(DiskListEntry, DISKENTRY, ListEntry);

        PartListEntry = DiskEntry->PrimaryPartListHead.Flink;
        if (PartListEntry != &DiskEntry->PrimaryPartListHead)
        {
            PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

            List->CurrentDisk = DiskEntry;
            List->CurrentPartition = PartEntry;
            return TRUE;
        }

        DiskListEntry = DiskListEntry->Flink;
    }

    return FALSE;
}


BOOL
ScrollUpPartitionList(
    PPARTLIST List)
{
    PLIST_ENTRY DiskListEntry;
    PLIST_ENTRY PartListEntry;
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;

    /* Fail, if no disks are available */
    if (IsListEmpty(&List->DiskListHead))
        return FALSE;

    /* Check for previous usable entry on current disk */
    if (List->CurrentPartition != NULL)
    {
        if (List->CurrentPartition->LogicalPartition)
        {
            /* Logical partition */
            PartListEntry = List->CurrentPartition->ListEntry.Blink;
            if (PartListEntry != &List->CurrentDisk->LogicalPartListHead)
            {
                /* Previous logical partition */
                PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);
            }
            else
            {
                /* Extended partition*/
                PartEntry = List->CurrentDisk->ExtendedPartition;
            }

            List->CurrentPartition = PartEntry;
            return TRUE;
        }
        else
        {
            /* Primary or extended partition */

            PartListEntry = List->CurrentPartition->ListEntry.Blink;
            if (PartListEntry != &List->CurrentDisk->PrimaryPartListHead)
            {
                PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

                if (PartEntry->IsPartitioned == TRUE &&
                    IsContainerPartition(PartEntry->PartitionType))
                {
                    PartListEntry = List->CurrentDisk->LogicalPartListHead.Blink;
                    PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);
                }

                List->CurrentPartition = PartEntry;
                return TRUE;
            }

        }
    }

    /* Search for the last partition entry on the previous disk */
    DiskListEntry = List->CurrentDisk->ListEntry.Blink;
    while (DiskListEntry != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(DiskListEntry, DISKENTRY, ListEntry);

        PartListEntry = DiskEntry->PrimaryPartListHead.Blink;
        if (PartListEntry != &DiskEntry->PrimaryPartListHead)
        {
            PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

            if (PartEntry->IsPartitioned == TRUE &&
                IsContainerPartition(PartEntry->PartitionType))
            {
                PartListEntry = DiskEntry->LogicalPartListHead.Blink;
                if (PartListEntry != &DiskEntry->LogicalPartListHead)
                {
                    PartEntry = CONTAINING_RECORD(PartListEntry, PARTENTRY, ListEntry);

                    List->CurrentDisk = DiskEntry;
                    List->CurrentPartition = PartEntry;
                    return TRUE;
                }
            }
            else
            {
                List->CurrentDisk = DiskEntry;
                List->CurrentPartition = PartEntry;
                return TRUE;
            }
        }

        DiskListEntry = DiskListEntry->Blink;
    }

    return FALSE;
}


static
BOOLEAN
IsEmptyLayoutEntry(
    PPARTITION_INFORMATION PartitionInfo)
{
    if (PartitionInfo->StartingOffset.QuadPart == 0 &&
        PartitionInfo->PartitionLength.QuadPart == 0)
//        PartitionInfo->PartitionType == 0)
        return TRUE;

    return FALSE;
}


static
BOOLEAN
IsSamePrimaryLayoutEntry(
    IN PPARTITION_INFORMATION PartitionInfo,
    IN PDISKENTRY DiskEntry,
    IN PPARTENTRY PartEntry)
{
    if (PartitionInfo->StartingOffset.QuadPart == PartEntry->StartSector.QuadPart * DiskEntry->BytesPerSector &&
        PartitionInfo->PartitionLength.QuadPart == PartEntry->SectorCount.QuadPart * DiskEntry->BytesPerSector)
//        PartitionInfo->PartitionNumber = PartEntry->PartitionNumber &&
//        PartitionInfo->PartitionType == PartEntry->PartitionType
        return TRUE;

    return FALSE;
}


static
VOID
UpdateDiskLayout(
    IN PDISKENTRY DiskEntry)
{
    PPARTITION_INFORMATION PartitionInfo;
    PLIST_ENTRY ListEntry;
    PPARTENTRY PartEntry;
    ULONG Index = 0;
    ULONG PartitionNumber = 1;

DPRINT1("UpdateDiskLayout()\n");

    ListEntry = DiskEntry->PrimaryPartListHead.Flink;
    while (ListEntry != &DiskEntry->PrimaryPartListHead)
    {
        PartEntry = CONTAINING_RECORD(ListEntry, PARTENTRY, ListEntry);

        if (PartEntry->IsPartitioned == TRUE)
        {
            PartitionInfo = &DiskEntry->LayoutBuffer->PartitionEntry[Index];

            if (!IsSamePrimaryLayoutEntry(PartitionInfo, DiskEntry, PartEntry))
            {
DPRINT1("Updating partition entry %lu\n", Index);
                PartitionInfo->StartingOffset.QuadPart = PartEntry->StartSector.QuadPart * DiskEntry->BytesPerSector;
                PartitionInfo->PartitionLength.QuadPart = PartEntry->SectorCount.QuadPart * DiskEntry->BytesPerSector;
                PartitionInfo->HiddenSectors = 0;
                PartitionInfo->PartitionNumber = (!IsContainerPartition(PartEntry->PartitionType)) ? PartitionNumber : 0;
                PartitionInfo->PartitionType = PartEntry->PartitionType;
                PartitionInfo->BootIndicator = PartEntry->BootIndicator;
                PartitionInfo->RecognizedPartition = FALSE;
                PartitionInfo->RewritePartition = TRUE;

                PartEntry->PartitionNumber = PartitionNumber;
                PartEntry->PartitionIndex = Index;

                PartitionNumber++;
            }
            else if (!IsEmptyLayoutEntry(PartitionInfo))
            {
                PartitionNumber++;
            }

            Index++;
        }

        ListEntry = ListEntry->Flink;
    }

    for (;Index < 4; Index++)
    {
        PartitionInfo = &DiskEntry->LayoutBuffer->PartitionEntry[Index];

        if (!IsEmptyLayoutEntry(PartitionInfo))
        {
DPRINT1("Wiping partition entry %lu\n", Index);
            PartitionInfo->StartingOffset.QuadPart = 0;
            PartitionInfo->PartitionLength.QuadPart = 0;
            PartitionInfo->HiddenSectors = 0;
            PartitionInfo->PartitionNumber = 0;
            PartitionInfo->PartitionType = 0;
            PartitionInfo->BootIndicator = FALSE;
            PartitionInfo->RecognizedPartition = FALSE;
            PartitionInfo->RewritePartition = TRUE;
        }
    }

#ifdef DUMP_PARTITION_TABLE
    DumpPartitionTable(DiskEntry);
#endif
}


static
PPARTENTRY
GetPrevUnpartitionedEntry(
    PDISKENTRY DiskEntry,
    PPARTENTRY PartEntry)
{
    PPARTENTRY PrevPartEntry;

    if (PartEntry->ListEntry.Blink != &DiskEntry->PrimaryPartListHead)
    {
        PrevPartEntry = CONTAINING_RECORD(PartEntry->ListEntry.Blink,
                                          PARTENTRY,
                                          ListEntry);
        if (PrevPartEntry->IsPartitioned == FALSE)
            return PrevPartEntry;
    }

    return NULL;
}


static
PPARTENTRY
GetNextUnpartitionedEntry(
    PDISKENTRY DiskEntry,
    PPARTENTRY PartEntry)
{
    PPARTENTRY NextPartEntry;

    if (PartEntry->ListEntry.Flink != &DiskEntry->PrimaryPartListHead)
    {
        NextPartEntry = CONTAINING_RECORD(PartEntry->ListEntry.Flink,
                                          PARTENTRY,
                                          ListEntry);
        if (NextPartEntry->IsPartitioned == FALSE)
            return NextPartEntry;
    }

    return NULL;
}


VOID
CreatePrimaryPartition(
    PPARTLIST List,
    ULONGLONG SectorCount,
    BOOLEAN AutoCreate)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PPARTENTRY NewPartEntry;

    DPRINT1("CreatePrimaryPartition(%I64u)\n", SectorCount);

    if (List == NULL ||
        List->CurrentDisk == NULL ||
        List->CurrentPartition == NULL ||
        List->CurrentPartition->IsPartitioned == TRUE)
    {
        return;
    }

    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

DPRINT1("Current partition sector count: %I64u\n", PartEntry->SectorCount.QuadPart);

    if (AutoCreate == TRUE ||
        Align(PartEntry->StartSector.QuadPart + SectorCount, DiskEntry->SectorAlignment) - PartEntry->StartSector.QuadPart == PartEntry->SectorCount.QuadPart)
    {
DPRINT1("Convert existing partition entry\n");
        /* Convert current entry to 'new (unformatted)' */
        PartEntry->IsPartitioned = TRUE;
        PartEntry->PartitionType = PARTITION_ENTRY_UNUSED;
        PartEntry->FormatState = Unformatted;
        PartEntry->AutoCreate = AutoCreate;
        PartEntry->New = TRUE;
        PartEntry->BootIndicator = FALSE;

DPRINT1("First Sector: %I64u\n", PartEntry->StartSector.QuadPart);
DPRINT1("Last Sector: %I64u\n", PartEntry->StartSector.QuadPart + PartEntry->SectorCount.QuadPart - 1);
DPRINT1("Total Sectors: %I64u\n", PartEntry->SectorCount.QuadPart);
    }
    else
    {
DPRINT1("Add new partition entry\n");

        /* Insert and initialize a new partition entry */
        NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                       HEAP_ZERO_MEMORY,
                                       sizeof(PARTENTRY));
        if (NewPartEntry == NULL)
            return;

        /* Insert the new entry into the list */
        InsertTailList(&PartEntry->ListEntry,
                       &NewPartEntry->ListEntry);

        NewPartEntry->DiskEntry = DiskEntry;

        NewPartEntry->IsPartitioned = TRUE;
        NewPartEntry->StartSector.QuadPart = PartEntry->StartSector.QuadPart;
        NewPartEntry->SectorCount.QuadPart = Align(NewPartEntry->StartSector.QuadPart + SectorCount, DiskEntry->SectorAlignment) -
                                             NewPartEntry->StartSector.QuadPart;
        NewPartEntry->PartitionType = PARTITION_ENTRY_UNUSED;

DPRINT1("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
DPRINT1("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
DPRINT1("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

        NewPartEntry->New = TRUE;
        NewPartEntry->FormatState = Unformatted;
        NewPartEntry->BootIndicator = FALSE;

        PartEntry->StartSector.QuadPart = NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart;
        PartEntry->SectorCount.QuadPart -= (PartEntry->StartSector.QuadPart - NewPartEntry->StartSector.QuadPart);
    }

    UpdateDiskLayout(DiskEntry);

    DiskEntry->Dirty = TRUE;

    UpdatePartitionNumbers(DiskEntry);

    AssignDriveLetters(List);
}


static
VOID
AddLogicalDiskSpace(
    PDISKENTRY DiskEntry)
{
    PPARTENTRY NewPartEntry;

    DPRINT1("AddLogicalDiskSpace()\n");

    /* Create a partition table entry that represents the empty space in the container partition */
    NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                   HEAP_ZERO_MEMORY,
                                   sizeof(PARTENTRY));
    if (NewPartEntry == NULL)
        return;

    NewPartEntry->DiskEntry = DiskEntry;
    NewPartEntry->LogicalPartition = TRUE;

    NewPartEntry->IsPartitioned = FALSE;
    NewPartEntry->StartSector.QuadPart = DiskEntry->ExtendedPartition->StartSector.QuadPart + (ULONGLONG)DiskEntry->SectorsPerTrack;
    NewPartEntry->SectorCount.QuadPart = DiskEntry->ExtendedPartition->SectorCount.QuadPart - (ULONGLONG)DiskEntry->SectorsPerTrack;

    DPRINT1("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
    DPRINT1("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
    DPRINT1("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);

    NewPartEntry->FormatState = Unformatted;

    InsertTailList(&DiskEntry->LogicalPartListHead,
                   &NewPartEntry->ListEntry);
}


VOID
CreateExtendedPartition(
    PPARTLIST List,
    ULONGLONG SectorCount)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PPARTENTRY NewPartEntry;

    DPRINT1("CreateExtendedPartition(%I64u)\n", SectorCount);

    if (List == NULL ||
        List->CurrentDisk == NULL ||
        List->CurrentPartition == NULL ||
        List->CurrentPartition->IsPartitioned == TRUE)
    {
        return;
    }

    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

DPRINT1("Current partition sector count: %I64u\n", PartEntry->SectorCount.QuadPart);

    if (Align(PartEntry->StartSector.QuadPart + SectorCount, DiskEntry->SectorAlignment) - PartEntry->StartSector.QuadPart == PartEntry->SectorCount.QuadPart)
    {
DPRINT1("Convert existing partition entry\n");
        /* Convert current entry to 'new (unformatted)' */
        PartEntry->IsPartitioned = TRUE;
        PartEntry->FormatState = Formatted;
        PartEntry->AutoCreate = FALSE;
        PartEntry->New = FALSE;
        PartEntry->BootIndicator = FALSE;

        if (PartEntry->StartSector.QuadPart < 1450560)
        {
            /* Partition starts below the 8.4GB boundary ==> CHS partition */
            PartEntry->PartitionType = PARTITION_EXTENDED;
        }
        else
        {
            /* Partition starts above the 8.4GB boundary ==> LBA partition */
            PartEntry->PartitionType = PARTITION_XINT13_EXTENDED;
        }

        DiskEntry->ExtendedPartition = PartEntry;

DPRINT1("First Sector: %I64u\n", PartEntry->StartSector.QuadPart);
DPRINT1("Last Sector: %I64u\n", PartEntry->StartSector.QuadPart + PartEntry->SectorCount.QuadPart - 1);
DPRINT1("Total Sectors: %I64u\n", PartEntry->SectorCount.QuadPart);
    }
    else
    {
DPRINT1("Add new partition entry\n");

        /* Insert and initialize a new partition entry */
        NewPartEntry = RtlAllocateHeap(ProcessHeap,
                                       HEAP_ZERO_MEMORY,
                                       sizeof(PARTENTRY));
        if (NewPartEntry == NULL)
            return;

        /* Insert the new entry into the list */
        InsertTailList(&PartEntry->ListEntry,
                       &NewPartEntry->ListEntry);

        NewPartEntry->DiskEntry = DiskEntry;

        NewPartEntry->IsPartitioned = TRUE;
        NewPartEntry->StartSector.QuadPart = PartEntry->StartSector.QuadPart;
        NewPartEntry->SectorCount.QuadPart = Align(NewPartEntry->StartSector.QuadPart + SectorCount, DiskEntry->SectorAlignment) -
                                             NewPartEntry->StartSector.QuadPart;

        NewPartEntry->New = FALSE;
        NewPartEntry->FormatState = Formatted;
        NewPartEntry->BootIndicator = FALSE;

        if (NewPartEntry->StartSector.QuadPart < 1450560)
        {
            /* Partition starts below the 8.4GB boundary ==> CHS partition */
            NewPartEntry->PartitionType = PARTITION_EXTENDED;
        }
        else
        {
            /* Partition starts above the 8.4GB boundary ==> LBA partition */
            NewPartEntry->PartitionType = PARTITION_XINT13_EXTENDED;
        }

        DiskEntry->ExtendedPartition = NewPartEntry;

        PartEntry->StartSector.QuadPart = NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart;
        PartEntry->SectorCount.QuadPart -= (PartEntry->StartSector.QuadPart - NewPartEntry->StartSector.QuadPart);

DPRINT1("First Sector: %I64u\n", NewPartEntry->StartSector.QuadPart);
DPRINT1("Last Sector: %I64u\n", NewPartEntry->StartSector.QuadPart + NewPartEntry->SectorCount.QuadPart - 1);
DPRINT1("Total Sectors: %I64u\n", NewPartEntry->SectorCount.QuadPart);
    }

    AddLogicalDiskSpace(DiskEntry);

    UpdateDiskLayout(DiskEntry);

    DiskEntry->Dirty = TRUE;

    UpdatePartitionNumbers(DiskEntry);

    AssignDriveLetters(List);
}


VOID
CreateLogicalPartition(
    PPARTLIST List,
    ULONGLONG SectorCount)
{
//    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
//    PPARTENTRY NewPartEntry;

    DPRINT1("CreateLogicalPartition(%I64u)\n", SectorCount);

    if (List == NULL ||
        List->CurrentDisk == NULL ||
        List->CurrentPartition == NULL ||
        List->CurrentPartition->IsPartitioned == TRUE)
    {
        return;
    }

//    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

    DPRINT1("Current partition sector count: %I64u\n", PartEntry->SectorCount.QuadPart);
}


VOID
DeleteCurrentPartition(
    PPARTLIST List)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PPARTENTRY PrevPartEntry;
    PPARTENTRY NextPartEntry;
    PPARTENTRY LogicalPartEntry;
    PLIST_ENTRY Entry;

    if (List == NULL ||
        List->CurrentDisk == NULL ||
        List->CurrentPartition == NULL ||
        List->CurrentPartition->IsPartitioned == FALSE)
    {
        return;
    }

    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

    /* Delete all logical partiton entries if an extended partition will be deleted */
    if (DiskEntry->ExtendedPartition == PartEntry)
    {
        while (!IsListEmpty(&DiskEntry->LogicalPartListHead))
        {
            Entry = RemoveHeadList(&DiskEntry->LogicalPartListHead);
            LogicalPartEntry = CONTAINING_RECORD(Entry, PARTENTRY, ListEntry);

            RtlFreeHeap(ProcessHeap, 0, LogicalPartEntry);
        }

        DiskEntry->ExtendedPartition = NULL;
    }

    /* Adjust unpartitioned disk space entries */

    /* Get pointer to previous and next unpartitioned entries */
    PrevPartEntry = GetPrevUnpartitionedEntry(DiskEntry,
                                              PartEntry);

    NextPartEntry = GetNextUnpartitionedEntry(DiskEntry,
                                              PartEntry);

    if (PrevPartEntry != NULL && NextPartEntry != NULL)
    {
        /* Merge previous, current and next unpartitioned entry */

        /* Adjust the previous entries length */
        PrevPartEntry->SectorCount.QuadPart += (PartEntry->SectorCount.QuadPart + NextPartEntry->SectorCount.QuadPart);

        /* Remove the current entry */
        RemoveEntryList(&PartEntry->ListEntry);
        RtlFreeHeap(ProcessHeap, 0, PartEntry);

        /* Remove the next entry */
        RemoveEntryList (&NextPartEntry->ListEntry);
        RtlFreeHeap(ProcessHeap, 0, NextPartEntry);

        /* Update current partition */
        List->CurrentPartition = PrevPartEntry;
    }
    else if (PrevPartEntry != NULL && NextPartEntry == NULL)
    {
        /* Merge current and previous unpartitioned entry */

        /* Adjust the previous entries length */
        PrevPartEntry->SectorCount.QuadPart += PartEntry->SectorCount.QuadPart;

        /* Remove the current entry */
        RemoveEntryList(&PartEntry->ListEntry);
        RtlFreeHeap(ProcessHeap, 0, PartEntry);

        /* Update current partition */
        List->CurrentPartition = PrevPartEntry;
    }
    else if (PrevPartEntry == NULL && NextPartEntry != NULL)
    {
        /* Merge current and next unpartitioned entry */

        /* Adjust the next entries offset and length */
        NextPartEntry->StartSector.QuadPart = PartEntry->StartSector.QuadPart;
        NextPartEntry->SectorCount.QuadPart += PartEntry->SectorCount.QuadPart;

        /* Remove the current entry */
        RemoveEntryList(&PartEntry->ListEntry);
        RtlFreeHeap(ProcessHeap, 0, PartEntry);

        /* Update current partition */
        List->CurrentPartition = NextPartEntry;
    }
    else
    {
        /* Nothing to merge but change current entry */
        PartEntry->IsPartitioned = FALSE;
        PartEntry->PartitionType = PARTITION_ENTRY_UNUSED;
        PartEntry->FormatState = Unformatted;
        PartEntry->DriveLetter = 0;
    }

    UpdateDiskLayout(DiskEntry);

    DiskEntry->Dirty = TRUE;

    UpdatePartitionNumbers(DiskEntry);

    AssignDriveLetters(List);
}


VOID
CheckActiveBootPartition(
    PPARTLIST List)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PLIST_ENTRY ListEntry;

    /* Check for empty disk list */
    if (IsListEmpty (&List->DiskListHead))
    {
        List->ActiveBootDisk = NULL;
        List->ActiveBootPartition = NULL;
        return;
    }

#if 0
    if (List->ActiveBootDisk != NULL &&
        List->ActiveBootPartition != NULL)
    {
        /* We already have an active boot partition */
        return;
    }
#endif

    /* Choose the currently selected disk */
    DiskEntry = List->CurrentDisk;

    /* Check for empty partition list */
    if (IsListEmpty (&DiskEntry->PrimaryPartListHead))
    {
        List->ActiveBootDisk = NULL;
        List->ActiveBootPartition = NULL;
        return;
    }

    PartEntry = CONTAINING_RECORD(DiskEntry->PrimaryPartListHead.Flink,
                                  PARTENTRY,
                                  ListEntry);

    /* Set active boot partition */
    if ((DiskEntry->NewDisk == TRUE) ||
        (PartEntry->BootIndicator == FALSE))
    {
        PartEntry->BootIndicator = TRUE;
        DiskEntry->LayoutBuffer->PartitionEntry[PartEntry->PartitionIndex].BootIndicator = TRUE;
        DiskEntry->LayoutBuffer->PartitionEntry[PartEntry->PartitionIndex].RewritePartition = TRUE;
        DiskEntry->Dirty = TRUE;

        /* FIXME: Might be incorrect if partitions were created by Linux FDISK */
        List->ActiveBootDisk = DiskEntry;
        List->ActiveBootPartition = PartEntry;

        return;
    }

    /* Disk is not new, scan all partitions to find a bootable one */
    List->ActiveBootDisk = NULL;
    List->ActiveBootPartition = NULL;

    ListEntry = DiskEntry->PrimaryPartListHead.Flink;
    while (ListEntry != &DiskEntry->PrimaryPartListHead)
    {
        PartEntry = CONTAINING_RECORD(ListEntry,
                                      PARTENTRY,
                                      ListEntry);

        /* Check if it is partitioned */
        if (PartEntry->IsPartitioned)
        {
            if (PartEntry->PartitionType != PARTITION_ENTRY_UNUSED &&
                PartEntry->BootIndicator)
            {
                /* Yes, we found it */
                List->ActiveBootDisk = DiskEntry;
                List->ActiveBootPartition = PartEntry;

                DPRINT("Found bootable partition disk %d, drive letter %c\n",
                       DiskEntry->DiskNumber, PartEntry->DriveLetter);
                break;
            }
        }

        /* Go to the next one */
        ListEntry = ListEntry->Flink;
    }
}


BOOLEAN
CheckForLinuxFdiskPartitions(
    PPARTLIST List)
{
#if 0
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    PLIST_ENTRY Entry1;
    PLIST_ENTRY Entry2;
    ULONG PartitionCount;
    ULONG i;

    Entry1 = List->DiskListHead.Flink;
    while (Entry1 != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry1,
                                      DISKENTRY,
                                      ListEntry);

        Entry2 = DiskEntry->PartListHead.Flink;
        while (Entry2 != &DiskEntry->PartListHead)
        {
            PartEntry = CONTAINING_RECORD(Entry2,
                                          PARTENTRY,
                                          ListEntry);

            if (PartEntry->Unpartitioned == FALSE)
            {
                PartitionCount = 0;

                for (i = 0; i < 4; i++)
                {
                    if (!IsContainerPartition(PartEntry->PartInfo[i].PartitionType) &&
                        PartEntry->PartInfo[i].PartitionLength.QuadPart != 0ULL)
                    {
                        PartitionCount++;
                    }
                }

                if (PartitionCount > 1)
                {
                    return TRUE;
                }
            }

            Entry2 = Entry2->Flink;
        }

        Entry1 = Entry1->Flink;
    }
#endif

    return FALSE;
}


static
NTSTATUS
WritePartitons(
    IN PPARTLIST List,
    IN PDISKENTRY DiskEntry)
{
    WCHAR DstPath[MAX_PATH];
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK Iosb;
    UNICODE_STRING Name;
    ULONG BufferSize;
    HANDLE FileHandle = NULL;
    NTSTATUS Status;

    DPRINT("WritePartitions() Disk: %lu\n", DiskEntry->DiskNumber);

    swprintf(DstPath,
             L"\\Device\\Harddisk%d\\Partition0",
             DiskEntry->DiskNumber);
    RtlInitUnicodeString(&Name,
                         DstPath);
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               0,
                               NULL,
                               NULL);

    Status = NtOpenFile(&FileHandle,
                        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                        &ObjectAttributes,
                        &Iosb,
                        0,
                        FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("NtOpenFile() failed (Status %lx)\n", Status);
        return Status;
    }

#ifdef DUMP_PARTITION_TABLE
    DumpPartitionTable(DiskEntry);
#endif

    BufferSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                 ((DiskEntry->LayoutBuffer->PartitionCount - 1) * sizeof(PARTITION_INFORMATION));
    Status = NtDeviceIoControlFile(FileHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &Iosb,
                                   IOCTL_DISK_SET_DRIVE_LAYOUT,
                                   DiskEntry->LayoutBuffer,
                                   BufferSize,
                                   NULL,
                                   0);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("IOCTL_DISK_SET_DRIVE_LAYOUT failed (Status 0x%08lx)\n", Status);
    }

    if (FileHandle != NULL)
        NtClose(FileHandle);

    return Status;
}


NTSTATUS
WriteDirtyPartitions(
    PPARTLIST List)
{
    PLIST_ENTRY Entry;
    PDISKENTRY DiskEntry;
    NTSTATUS Status;

    if (List == NULL)
        return STATUS_SUCCESS;

    Entry = List->DiskListHead.Flink;
    while (Entry != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry, DISKENTRY, ListEntry);

        if (DiskEntry->Dirty == TRUE)
        {
            Status = WritePartitons(List, DiskEntry);
            if (!NT_SUCCESS(Status))
                return Status;

            DiskEntry->Dirty = FALSE;
        }

        Entry = Entry->Flink;
    }

    return STATUS_SUCCESS;
}


BOOL
SetMountedDeviceValues(
    PPARTLIST List)
{
    PLIST_ENTRY Entry1, Entry2;
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;
    LARGE_INTEGER StartingOffset;

    if (List == NULL)
    {
        return FALSE;
    }

    Entry1 = List->DiskListHead.Flink;
    while (Entry1 != &List->DiskListHead)
    {
        DiskEntry = CONTAINING_RECORD(Entry1,
                                      DISKENTRY,
                                      ListEntry);

        Entry2 = DiskEntry->PrimaryPartListHead.Flink;
        while (Entry2 != &DiskEntry->PrimaryPartListHead)
        {
            PartEntry = CONTAINING_RECORD(Entry2, PARTENTRY, ListEntry);
            if (PartEntry->IsPartitioned)
            {
                if (PartEntry->DriveLetter)
                {
                    StartingOffset.QuadPart = PartEntry->StartSector.QuadPart * DiskEntry->BytesPerSector;
                    if (!SetMountedDeviceValue(PartEntry->DriveLetter,
                                               DiskEntry->LayoutBuffer->Signature,
                                               StartingOffset))
                    {
                        return FALSE;
                    }
                }
            }

            Entry2 = Entry2->Flink;
        }

        Entry1 = Entry1->Flink;
    }

    return TRUE;
}


static
ULONG
GetPrimaryPartitionCount(
    IN PDISKENTRY DiskEntry)
{
    PLIST_ENTRY Entry;
    PPARTENTRY PartEntry;
    UINT nCount = 0;

    Entry = DiskEntry->PrimaryPartListHead.Flink;
    while (Entry != &DiskEntry->PrimaryPartListHead)
    {
        PartEntry = CONTAINING_RECORD(Entry, PARTENTRY, ListEntry);
        if (PartEntry->IsPartitioned == TRUE)
            nCount++;

        Entry = Entry->Flink;
    }

    return nCount;
}


ULONG
PrimaryPartitionCreationChecks(
    IN PPARTLIST List)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;

    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

    /* Fail if partition is already in use */
    if (PartEntry->IsPartitioned == TRUE)
        return ERROR_NEW_PARTITION;

    /* Fail if there are more than 4 partitions in the list */
    if (GetPrimaryPartitionCount(DiskEntry) > 4)
        return ERROR_PARTITION_TABLE_FULL;

    return ERROR_SUCCESS;
}


ULONG
ExtendedPartitionCreationChecks(
    IN PPARTLIST List)
{
    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;

    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

    /* Fail if partition is already in use */
    if (PartEntry->IsPartitioned == TRUE)
        return ERROR_NEW_PARTITION;

    /* Fail if there are more than 4 partitions in the list */
    if (GetPrimaryPartitionCount(DiskEntry) > 4)
        return ERROR_PARTITION_TABLE_FULL;

    /* Fail if there is another extended partition in the list */
    if (DiskEntry->ExtendedPartition != NULL)
        return ERROR_ONLY_ONE_EXTENDED;

    return ERROR_SUCCESS;
}


ULONG
LogicalPartitionCreationChecks(
    IN PPARTLIST List)
{
//    PDISKENTRY DiskEntry;
    PPARTENTRY PartEntry;

//    DiskEntry = List->CurrentDisk;
    PartEntry = List->CurrentPartition;

    /* Fail if partition is already in use */
    if (PartEntry->IsPartitioned == TRUE)
        return ERROR_NEW_PARTITION;

    return ERROR_SUCCESS;
}

/* EOF */
