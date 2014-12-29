/*
 *  ReactOS kernel
 *  Copyright (C) 2002, 2014 ReactOS Team
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS kernel
 * FILE:             drivers/filesystem/ntfs/mft.c
 * PURPOSE:          NTFS filesystem driver
 * PROGRAMMERS:      Eric Kohl
 *                   Valentin Verkhovsky
 *                   Pierre Schweitzer (pierre@reactos.org)
 *                   Hervé Poussineau (hpoussin@reactos.org)
 */

/* INCLUDES *****************************************************************/

#include "ntfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

PNTFS_ATTR_CONTEXT
PrepareAttributeContext(PNTFS_ATTR_RECORD AttrRecord)
{
    PNTFS_ATTR_CONTEXT Context;

    Context = ExAllocatePoolWithTag(NonPagedPool,
                                    FIELD_OFFSET(NTFS_ATTR_CONTEXT, Record) + AttrRecord->Length,
                                    TAG_NTFS);
    RtlCopyMemory(&Context->Record, AttrRecord, AttrRecord->Length);
    if (AttrRecord->IsNonResident)
    {
        LONGLONG DataRunOffset;
        ULONGLONG DataRunLength;

        Context->CacheRun = (PUCHAR)&Context->Record + Context->Record.NonResident.MappingPairsOffset;
        Context->CacheRunOffset = 0;
        Context->CacheRun = DecodeRun(Context->CacheRun, &DataRunOffset, &DataRunLength);
        Context->CacheRunLength = DataRunLength;
        if (DataRunOffset != -1)
        {
            /* Normal run. */
            Context->CacheRunStartLCN =
            Context->CacheRunLastLCN = DataRunOffset;
        }
        else
        {
            /* Sparse run. */
            Context->CacheRunStartLCN = -1;
            Context->CacheRunLastLCN = 0;
        }
        Context->CacheRunCurrentOffset = 0;
    }

    return Context;
}


VOID
ReleaseAttributeContext(PNTFS_ATTR_CONTEXT Context)
{
    ExFreePoolWithTag(Context, TAG_NTFS);
}


PNTFS_ATTR_CONTEXT
FindAttributeHelper(PDEVICE_EXTENSION Vcb,
                    PNTFS_ATTR_RECORD AttrRecord,
                    PNTFS_ATTR_RECORD AttrRecordEnd,
                    ULONG Type,
                    PCWSTR Name,
                    ULONG NameLength)
{
    DPRINT("FindAttributeHelper(%p, %p, %p, 0x%x, %S, %u)\n", Vcb, AttrRecord, AttrRecordEnd, Type, Name, NameLength);

    while (AttrRecord < AttrRecordEnd)
    {
        DPRINT("AttrRecord->Type = 0x%x\n", AttrRecord->Type);

        if (AttrRecord->Type == AttributeEnd)
            break;

        if (AttrRecord->Type == AttributeAttributeList)
        {
            PNTFS_ATTR_CONTEXT Context;
            PNTFS_ATTR_CONTEXT ListContext;
            PVOID ListBuffer;
            ULONGLONG ListSize;
            PNTFS_ATTR_RECORD ListAttrRecord;
            PNTFS_ATTR_RECORD ListAttrRecordEnd;

            ListContext = PrepareAttributeContext(AttrRecord);

            ListSize = AttributeDataLength(&ListContext->Record);
            if(ListSize <= 0xFFFFFFFF)
                ListBuffer = ExAllocatePoolWithTag(NonPagedPool, (ULONG)ListSize, TAG_NTFS);
            else
                ListBuffer = NULL;

            if(!ListBuffer)
            {
                DPRINT("Failed to allocate memory: %x\n", (ULONG)ListSize);
                continue;
            }

            ListAttrRecord = (PNTFS_ATTR_RECORD)ListBuffer;
            ListAttrRecordEnd = (PNTFS_ATTR_RECORD)((PCHAR)ListBuffer + ListSize);

            if (ReadAttribute(Vcb, ListContext, 0, ListBuffer, (ULONG)ListSize) == ListSize)
            {
                Context = FindAttributeHelper(Vcb, ListAttrRecord, ListAttrRecordEnd,
                                              Type, Name, NameLength);

                ReleaseAttributeContext(ListContext);
                ExFreePoolWithTag(ListBuffer, TAG_NTFS);

                if (Context != NULL)
                {
                    if (AttrRecord->IsNonResident) DPRINT("Found context = %p\n", Context);
                    return Context;
                }
            }
        }

        if (AttrRecord->Type == Type)
        {
            if (AttrRecord->NameLength == NameLength)
            {
                PWCHAR AttrName;

                AttrName = (PWCHAR)((PCHAR)AttrRecord + AttrRecord->NameOffset);
                DPRINT("%.*S, %.*S\n", AttrRecord->NameLength, AttrName, NameLength, Name);
                if (RtlCompareMemory(AttrName, Name, NameLength << 1) == (NameLength << 1))
                {
                    /* Found it, fill up the context and return. */
                    DPRINT("Found context\n");
                    return PrepareAttributeContext(AttrRecord);
                }
            }
        }

        if (AttrRecord->Length == 0)
        {
            DPRINT("Null length attribute record\n");
            return NULL;
        }
        AttrRecord = (PNTFS_ATTR_RECORD)((PCHAR)AttrRecord + AttrRecord->Length);
    }

    DPRINT("Ended\n");
    return NULL;
}


NTSTATUS
FindAttribute(PDEVICE_EXTENSION Vcb,
              PFILE_RECORD_HEADER MftRecord,
              ULONG Type,
              PCWSTR Name,
              ULONG NameLength,
              PNTFS_ATTR_CONTEXT * AttrCtx)
{
    PNTFS_ATTR_RECORD AttrRecord;
    PNTFS_ATTR_RECORD AttrRecordEnd;

    DPRINT("FindAttribute(%p, %p, 0x%x, %S, %u, %p)\n", Vcb, MftRecord, Type, Name, NameLength, AttrCtx);

    AttrRecord = (PNTFS_ATTR_RECORD)((PCHAR)MftRecord + MftRecord->AttributeOffset);
    AttrRecordEnd = (PNTFS_ATTR_RECORD)((PCHAR)MftRecord + Vcb->NtfsInfo.BytesPerFileRecord);

    *AttrCtx = FindAttributeHelper(Vcb, AttrRecord, AttrRecordEnd, Type, Name, NameLength);
    if (*AttrCtx == NULL)
    {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}


ULONG
AttributeAllocatedLength(PNTFS_ATTR_RECORD AttrRecord)
{
    if (AttrRecord->IsNonResident)
        return AttrRecord->NonResident.AllocatedSize;
    else
        return AttrRecord->Resident.ValueLength;
}


ULONGLONG
AttributeDataLength(PNTFS_ATTR_RECORD AttrRecord)
{
    if (AttrRecord->IsNonResident)
        return AttrRecord->NonResident.DataSize;
    else
        return AttrRecord->Resident.ValueLength;
}


ULONG
ReadAttribute(PDEVICE_EXTENSION Vcb,
              PNTFS_ATTR_CONTEXT Context,
              ULONGLONG Offset,
              PCHAR Buffer,
              ULONG Length)
{
    ULONGLONG LastLCN;
    PUCHAR DataRun;
    LONGLONG DataRunOffset;
    ULONGLONG DataRunLength;
    LONGLONG DataRunStartLCN;
    ULONGLONG CurrentOffset;
    ULONG ReadLength;
    ULONG AlreadyRead;
    NTSTATUS Status;

    if (!Context->Record.IsNonResident)
    {
        if (Offset > Context->Record.Resident.ValueLength)
            return 0;
        if (Offset + Length > Context->Record.Resident.ValueLength)
            Length = (ULONG)(Context->Record.Resident.ValueLength - Offset);
        RtlCopyMemory(Buffer, (PCHAR)&Context->Record + Context->Record.Resident.ValueOffset + Offset, Length);
        return Length;
    }

    /*
     * Non-resident attribute
     */

    /*
     * I. Find the corresponding start data run.
     */

    AlreadyRead = 0;

    // FIXME: Cache seems to be non-working. Disable it for now
    //if(Context->CacheRunOffset <= Offset && Offset < Context->CacheRunOffset + Context->CacheRunLength * Volume->ClusterSize)
    if (0)
    {
        DataRun = Context->CacheRun;
        LastLCN = Context->CacheRunLastLCN;
        DataRunStartLCN = Context->CacheRunStartLCN;
        DataRunLength = Context->CacheRunLength;
        CurrentOffset = Context->CacheRunCurrentOffset;
    }
    else
    {
        LastLCN = 0;
        DataRun = (PUCHAR)&Context->Record + Context->Record.NonResident.MappingPairsOffset;
        CurrentOffset = 0;

        while (1)
        {
            DataRun = DecodeRun(DataRun, &DataRunOffset, &DataRunLength);
            if (DataRunOffset != -1)
            {
                /* Normal data run. */
                DataRunStartLCN = LastLCN + DataRunOffset;
                LastLCN = DataRunStartLCN;
            }
            else
            {
                /* Sparse data run. */
                DataRunStartLCN = -1;
            }

            if (Offset >= CurrentOffset &&
                Offset < CurrentOffset + (DataRunLength * Vcb->NtfsInfo.BytesPerCluster))
            {
                break;
            }

            if (*DataRun == 0)
            {
                return AlreadyRead;
            }

            CurrentOffset += DataRunLength * Vcb->NtfsInfo.BytesPerCluster;
        }
    }

    /*
     * II. Go through the run list and read the data
     */

    ReadLength = (ULONG)min(DataRunLength * Vcb->NtfsInfo.BytesPerCluster - (Offset - CurrentOffset), Length);
    if (DataRunStartLCN == -1)
        RtlZeroMemory(Buffer, ReadLength);
    Status = NtfsReadDisk(Vcb->StorageDevice,
                          DataRunStartLCN * Vcb->NtfsInfo.BytesPerCluster + Offset - CurrentOffset,
                          ReadLength,
                          Vcb->NtfsInfo.BytesPerSector,
                          (PVOID)Buffer,
                          FALSE);
    if (NT_SUCCESS(Status))
    {
        Length -= ReadLength;
        Buffer += ReadLength;
        AlreadyRead += ReadLength;

        if (ReadLength == DataRunLength * Vcb->NtfsInfo.BytesPerCluster - (Offset - CurrentOffset))
        {
            CurrentOffset += DataRunLength * Vcb->NtfsInfo.BytesPerCluster;
            DataRun = DecodeRun(DataRun, &DataRunOffset, &DataRunLength);
            if (DataRunLength != (ULONGLONG)-1)
            {
                DataRunStartLCN = LastLCN + DataRunOffset;
                LastLCN = DataRunStartLCN;
            }
            else
                DataRunStartLCN = -1;

            if (*DataRun == 0)
                return AlreadyRead;
        }

        while (Length > 0)
        {
            ReadLength = (ULONG)min(DataRunLength * Vcb->NtfsInfo.BytesPerCluster, Length);
            if (DataRunStartLCN == -1)
                RtlZeroMemory(Buffer, ReadLength);
            else
            {
                Status = NtfsReadDisk(Vcb->StorageDevice,
                                      DataRunStartLCN * Vcb->NtfsInfo.BytesPerCluster,
                                      ReadLength,
                                      Vcb->NtfsInfo.BytesPerSector,
                                      (PVOID)Buffer,
                                      FALSE);
                if (!NT_SUCCESS(Status))
                    break;
            }

            Length -= ReadLength;
            Buffer += ReadLength;
            AlreadyRead += ReadLength;

            /* We finished this request, but there still data in this data run. */
            if (Length == 0 && ReadLength != DataRunLength * Vcb->NtfsInfo.BytesPerCluster)
                break;

            /*
             * Go to next run in the list.
             */

            if (*DataRun == 0)
                break;
            CurrentOffset += DataRunLength * Vcb->NtfsInfo.BytesPerCluster;
            DataRun = DecodeRun(DataRun, &DataRunOffset, &DataRunLength);
            if (DataRunOffset != -1)
            {
                /* Normal data run. */
                DataRunStartLCN = LastLCN + DataRunOffset;
                LastLCN = DataRunStartLCN;
            }
            else
            {
                /* Sparse data run. */
                DataRunStartLCN = -1;
            }
        } /* while */

    } /* if Disk */

    Context->CacheRun = DataRun;
    Context->CacheRunOffset = Offset + AlreadyRead;
    Context->CacheRunStartLCN = DataRunStartLCN;
    Context->CacheRunLength = DataRunLength;
    Context->CacheRunLastLCN = LastLCN;
    Context->CacheRunCurrentOffset = CurrentOffset;

    return AlreadyRead;
}


NTSTATUS
ReadFileRecord(PDEVICE_EXTENSION Vcb,
               ULONGLONG index,
               PFILE_RECORD_HEADER file)
{
    ULONGLONG BytesRead;

    DPRINT("ReadFileRecord(%p, %I64x, %p)\n", Vcb, index, file);

    BytesRead = ReadAttribute(Vcb, Vcb->MFTContext, index * Vcb->NtfsInfo.BytesPerFileRecord, (PCHAR)file, Vcb->NtfsInfo.BytesPerFileRecord);
    if (BytesRead != Vcb->NtfsInfo.BytesPerFileRecord)
    {
        DPRINT1("ReadFileRecord failed: %I64u read, %u expected\n", BytesRead, Vcb->NtfsInfo.BytesPerFileRecord);
        return STATUS_PARTIAL_COPY;
    }

    /* Apply update sequence array fixups. */
    return FixupUpdateSequenceArray(Vcb, &file->Ntfs);
}


NTSTATUS 
FixupUpdateSequenceArray(PDEVICE_EXTENSION Vcb,
                         PNTFS_RECORD_HEADER Record)
{
    USHORT *USA;
    USHORT USANumber;
    USHORT USACount;
    USHORT *Block;

    USA = (USHORT*)((PCHAR)Record + Record->UsaOffset);
    USANumber = *(USA++);
    USACount = Record->UsaCount - 1; /* Exclude the USA Number. */
    Block = (USHORT*)((PCHAR)Record + Vcb->NtfsInfo.BytesPerSector - 2);

    while (USACount)
    {
        if (*Block != USANumber)
        {
            DPRINT1("Mismatch with USA: %u read, %u expected\n" , *Block, USANumber);
            return STATUS_UNSUCCESSFUL;
        }
        *Block = *(USA++);
        Block = (USHORT*)((PCHAR)Block + Vcb->NtfsInfo.BytesPerSector);
        USACount--;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ReadLCN(PDEVICE_EXTENSION Vcb,
        ULONGLONG lcn,
        ULONG count,
        PVOID buffer)
{
    LARGE_INTEGER DiskSector;

    DiskSector.QuadPart = lcn;

    return NtfsReadSectors(Vcb->StorageDevice,
                           DiskSector.u.LowPart * Vcb->NtfsInfo.SectorsPerCluster,
                           count * Vcb->NtfsInfo.SectorsPerCluster,
                           Vcb->NtfsInfo.BytesPerSector,
                           buffer,
                           FALSE);
}


BOOLEAN
CompareFileName(PUNICODE_STRING FileName,
                PINDEX_ENTRY_ATTRIBUTE IndexEntry,
                BOOLEAN DirSearch)
{
    BOOLEAN Ret, Alloc = FALSE;
    UNICODE_STRING EntryName;

    EntryName.Buffer = IndexEntry->FileName.Name;
    EntryName.Length = 
    EntryName.MaximumLength = IndexEntry->FileName.NameLength * sizeof(WCHAR);

    if (DirSearch)
    {
        UNICODE_STRING IntFileName;
        if (IndexEntry->FileName.NameType != NTFS_FILE_NAME_POSIX)
        {
            NT_VERIFY(NT_SUCCESS(RtlUpcaseUnicodeString(&IntFileName, FileName, TRUE)));
            Alloc = TRUE;
        }
        else
        {
            IntFileName = *FileName;
        }

        Ret = FsRtlIsNameInExpression(&IntFileName, &EntryName, (IndexEntry->FileName.NameType != NTFS_FILE_NAME_POSIX), NULL);

        if (Alloc)
        {
            RtlFreeUnicodeString(&IntFileName);
        }

        return Ret;
    }
    else
    {
        return (RtlCompareUnicodeString(FileName, &EntryName, (IndexEntry->FileName.NameType != NTFS_FILE_NAME_POSIX)) == 0);
    }
}


NTSTATUS
NtfsFindMftRecord(PDEVICE_EXTENSION Vcb,
                  ULONGLONG MFTIndex,
                  PUNICODE_STRING FileName,
                  PULONG FirstEntry,
                  BOOLEAN DirSearch,
                  ULONGLONG *OutMFTIndex)
{
    PFILE_RECORD_HEADER MftRecord;
    PNTFS_ATTR_CONTEXT IndexRootCtx;
    PNTFS_ATTR_CONTEXT IndexBitmapCtx;
    PNTFS_ATTR_CONTEXT IndexAllocationCtx;
    PINDEX_ROOT_ATTRIBUTE IndexRoot;
    PINDEX_BUFFER IndexBuffer;
    ULONGLONG BitmapDataSize;
    ULONGLONG IndexAllocationSize;
    PCHAR BitmapData;
    PCHAR IndexRecord;
    PINDEX_ENTRY_ATTRIBUTE IndexEntry, IndexEntryEnd;
    ULONG RecordOffset;
    ULONG IndexBlockSize;
    NTSTATUS Status;
    ULONG CurrentEntry = 0;

    DPRINT("NtfsFindMftRecord(%p, %I64d, %wZ, %p, %u, %p)\n", Vcb, MFTIndex, FileName, FirstEntry, DirSearch, OutMFTIndex);

    MftRecord = ExAllocatePoolWithTag(NonPagedPool,
                                      Vcb->NtfsInfo.BytesPerFileRecord,
                                      TAG_NTFS);
    if (MftRecord == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(ReadFileRecord(Vcb, MFTIndex, MftRecord)))
    {
        ASSERT(MftRecord->Ntfs.Type == NRH_FILE_TYPE);

        Status = FindAttribute(Vcb, MftRecord, AttributeIndexRoot, L"$I30", 4, &IndexRootCtx);
        if (!NT_SUCCESS(Status))
        {
            ExFreePoolWithTag(MftRecord, TAG_NTFS);
            return Status;
        }

        IndexRecord = ExAllocatePoolWithTag(NonPagedPool, Vcb->NtfsInfo.BytesPerIndexRecord, TAG_NTFS);
        if (IndexRecord == NULL)
        {
            ExFreePoolWithTag(MftRecord, TAG_NTFS);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ReadAttribute(Vcb, IndexRootCtx, 0, IndexRecord, Vcb->NtfsInfo.BytesPerIndexRecord);
        IndexRoot = (PINDEX_ROOT_ATTRIBUTE)IndexRecord;
        IndexEntry = (PINDEX_ENTRY_ATTRIBUTE)((PCHAR)&IndexRoot->Header + IndexRoot->Header.FirstEntryOffset);
        /* Index root is always resident. */
        IndexEntryEnd = (PINDEX_ENTRY_ATTRIBUTE)(IndexRecord + IndexRoot->Header.TotalSizeOfEntries);
        ReleaseAttributeContext(IndexRootCtx);

        DPRINT("IndexRecordSize: %x IndexBlockSize: %x\n", Vcb->NtfsInfo.BytesPerIndexRecord, IndexRoot->SizeOfEntry);

        while (IndexEntry < IndexEntryEnd &&
               !(IndexEntry->Flags & NTFS_INDEX_ENTRY_END))
        {
            UNICODE_STRING EntryName;
            EntryName.Buffer = IndexEntry->FileName.Name;
            EntryName.Length = 
            EntryName.MaximumLength = IndexEntry->FileName.NameLength * sizeof(WCHAR);

            if (IndexEntry->Flags & NTFS_INDEX_ENTRY_NODE)
                DPRINT1("Warning: sub-node browsing unimplemented! (%wZ)\n", &EntryName);

            if ((IndexEntry->Data.Directory.IndexedFile & NTFS_MFT_MASK) > 0x10 &&
                CurrentEntry >= *FirstEntry &&
                CompareFileName(FileName, IndexEntry, DirSearch))
            {
                *OutMFTIndex = (IndexEntry->Data.Directory.IndexedFile & NTFS_MFT_MASK);
                *FirstEntry = CurrentEntry;
                ExFreePoolWithTag(IndexRecord, TAG_NTFS);
                ExFreePoolWithTag(MftRecord, TAG_NTFS);
                return STATUS_SUCCESS;
            }

            ++CurrentEntry;
            IndexEntry = (PINDEX_ENTRY_ATTRIBUTE)((PCHAR)IndexEntry + IndexEntry->Length);
        }

        if (IndexRoot->Header.Flags & INDEX_ROOT_LARGE)
        {
            DPRINT("Large Index!\n");

            IndexBlockSize = IndexRoot->SizeOfEntry;

            Status = FindAttribute(Vcb, MftRecord, AttributeBitmap, L"$I30", 4, &IndexBitmapCtx);
            if (!NT_SUCCESS(Status))
            {
                DPRINT1("Corrupted filesystem!\n");
                ExFreePoolWithTag(MftRecord, TAG_NTFS);
                return Status;
            }
            BitmapDataSize = AttributeDataLength(&IndexBitmapCtx->Record);
            DPRINT("BitmapDataSize: %x\n", (ULONG)BitmapDataSize);
            if(BitmapDataSize <= 0xFFFFFFFF)
                BitmapData = ExAllocatePoolWithTag(NonPagedPool, (ULONG)BitmapDataSize, TAG_NTFS);
            else
                BitmapData = NULL;

            if (BitmapData == NULL)
            {
                ExFreePoolWithTag(IndexRecord, TAG_NTFS);
                ExFreePoolWithTag(MftRecord, TAG_NTFS);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            ReadAttribute(Vcb, IndexBitmapCtx, 0, BitmapData, (ULONG)BitmapDataSize);
            ReleaseAttributeContext(IndexBitmapCtx);

            Status = FindAttribute(Vcb, MftRecord, AttributeIndexAllocation, L"$I30", 4, &IndexAllocationCtx);
            if (!NT_SUCCESS(Status))
            {
                DPRINT("Corrupted filesystem!\n");
                ExFreePoolWithTag(BitmapData, TAG_NTFS);
                ExFreePoolWithTag(IndexRecord, TAG_NTFS);
                ExFreePoolWithTag(MftRecord, TAG_NTFS);
                return Status;
            }
            IndexAllocationSize = AttributeDataLength(&IndexAllocationCtx->Record);

            RecordOffset = 0;

            for (;;)
            {
                DPRINT("RecordOffset: %x IndexAllocationSize: %x\n", RecordOffset, IndexAllocationSize);
                for (; RecordOffset < IndexAllocationSize;)
                {
                    UCHAR Bit = 1 << ((RecordOffset / IndexBlockSize) & 7);
                    ULONG Byte = (RecordOffset / IndexBlockSize) >> 3;
                    if ((BitmapData[Byte] & Bit))
                        break;
                    RecordOffset += IndexBlockSize;
                }

                if (RecordOffset >= IndexAllocationSize)
                {
                    break;
                }

                ReadAttribute(Vcb, IndexAllocationCtx, RecordOffset, IndexRecord, IndexBlockSize);

                if (!NT_SUCCESS(FixupUpdateSequenceArray(Vcb, &((PFILE_RECORD_HEADER)IndexRecord)->Ntfs)))
                {
                    break;
                }

                IndexBuffer = (PINDEX_BUFFER)IndexRecord;
                ASSERT(IndexBuffer->Ntfs.Type == NRH_INDX_TYPE);
                ASSERT(IndexBuffer->Header.AllocatedSize + 0x18 == IndexBlockSize);
                IndexEntry = (PINDEX_ENTRY_ATTRIBUTE)((ULONG_PTR)&IndexBuffer->Header + IndexBuffer->Header.FirstEntryOffset);
                IndexEntryEnd = (PINDEX_ENTRY_ATTRIBUTE)((ULONG_PTR)&IndexBuffer->Header + IndexBuffer->Header.TotalSizeOfEntries);
                ASSERT(IndexEntryEnd <= (PINDEX_ENTRY_ATTRIBUTE)((ULONG_PTR)IndexBuffer + IndexBlockSize));

                while (IndexEntry < IndexEntryEnd &&
                       !(IndexEntry->Flags & NTFS_INDEX_ENTRY_END))
                {
                    UNICODE_STRING EntryName;
                    EntryName.Buffer = IndexEntry->FileName.Name;
                    EntryName.Length = 
                    EntryName.MaximumLength = IndexEntry->FileName.NameLength * sizeof(WCHAR);

                    if (IndexEntry->Flags & NTFS_INDEX_ENTRY_NODE)
                        DPRINT1("Warning: sub-node browsing unimplemented! (%wZ)\n", &EntryName);

                    if ((IndexEntry->Data.Directory.IndexedFile & NTFS_MFT_MASK) > 0x10 &&
                        CurrentEntry >= *FirstEntry &&
                        CompareFileName(FileName, IndexEntry, DirSearch))
                    {
                        DPRINT("File found\n");
                        *OutMFTIndex = (IndexEntry->Data.Directory.IndexedFile & NTFS_MFT_MASK);
                        *FirstEntry = CurrentEntry;
                        ExFreePoolWithTag(BitmapData, TAG_NTFS);
                        ExFreePoolWithTag(IndexRecord, TAG_NTFS);
                        ExFreePoolWithTag(MftRecord, TAG_NTFS);
                        ReleaseAttributeContext(IndexAllocationCtx);
                        return STATUS_SUCCESS;
                    }

                    ++CurrentEntry;
                    ASSERT(IndexEntry->Length >= sizeof(INDEX_ENTRY_ATTRIBUTE));
                    IndexEntry = (PINDEX_ENTRY_ATTRIBUTE)((PCHAR)IndexEntry + IndexEntry->Length);
                }

                RecordOffset += IndexBlockSize;
            }

            ReleaseAttributeContext(IndexAllocationCtx);
            ExFreePoolWithTag(BitmapData, TAG_NTFS);
        }

        ExFreePoolWithTag(IndexRecord, TAG_NTFS);
    }
    else
    {
        DPRINT("Can't read MFT record\n");
    }
    ExFreePoolWithTag(MftRecord, TAG_NTFS);

    return STATUS_OBJECT_PATH_NOT_FOUND;
}

NTSTATUS
NtfsLookupFileAt(PDEVICE_EXTENSION Vcb,
                 PUNICODE_STRING PathName,
                 PFILE_RECORD_HEADER *FileRecord,
                 PNTFS_ATTR_CONTEXT *DataContext,
                 PULONGLONG MFTIndex,
                 ULONGLONG CurrentMFTIndex)
{
    UNICODE_STRING Current, Remaining;
    NTSTATUS Status;
    ULONG FirstEntry = 0;

    DPRINT("NtfsLookupFileAt(%p, %wZ, %p, %p, %I64x)\n", Vcb, PathName, FileRecord, DataContext, CurrentMFTIndex);

    FsRtlDissectName(*PathName, &Current, &Remaining);

    while (Current.Length != 0)
    {
        DPRINT("Current: %wZ\n", &Current);

        Status = NtfsFindMftRecord(Vcb, CurrentMFTIndex, &Current, &FirstEntry, FALSE, &CurrentMFTIndex);
        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        if (Remaining.Length == 0)
            break;

        FsRtlDissectName(Current, &Current, &Remaining);
    }

    *FileRecord = ExAllocatePoolWithTag(NonPagedPool, Vcb->NtfsInfo.BytesPerFileRecord, TAG_NTFS);
    if (*FileRecord == NULL)
    {
        DPRINT("NtfsLookupFileAt: Can't allocate MFT record\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = ReadFileRecord(Vcb, CurrentMFTIndex, *FileRecord);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("NtfsLookupFileAt: Can't read MFT record\n");
        ExFreePoolWithTag(*FileRecord, TAG_NTFS);
        return Status;
    }

    if (!((*FileRecord)->Flags & FRH_DIRECTORY))
    {
        Status = FindAttribute(Vcb, *FileRecord, AttributeData, L"", 0, DataContext);
        if (!NT_SUCCESS(Status))
        {
            DPRINT("NtfsLookupFileAt: Can't find data attribute\n");
            ExFreePoolWithTag(*FileRecord, TAG_NTFS);
            return Status;
        }
    }
    else
    {
        *DataContext = NULL;
    }

    *MFTIndex = CurrentMFTIndex;

    return STATUS_SUCCESS;
}

NTSTATUS
NtfsLookupFile(PDEVICE_EXTENSION Vcb,
               PUNICODE_STRING PathName,
               PFILE_RECORD_HEADER *FileRecord,
               PNTFS_ATTR_CONTEXT *DataContext,
               PULONGLONG MFTIndex)
{
    return NtfsLookupFileAt(Vcb, PathName, FileRecord, DataContext, MFTIndex, NTFS_FILE_ROOT);
}

NTSTATUS
NtfsFindFileAt(PDEVICE_EXTENSION Vcb,
               PUNICODE_STRING SearchPattern,
               PULONG FirstEntry,
               PFILE_RECORD_HEADER *FileRecord,
               PNTFS_ATTR_CONTEXT *DataContext,
               PULONGLONG MFTIndex,
               ULONGLONG CurrentMFTIndex)
{
    NTSTATUS Status;

    DPRINT("NtfsFindFileAt(%p, %wZ, %p, %p, %p, %p, %I64x)\n", Vcb, SearchPattern, FirstEntry, FileRecord, DataContext, MFTIndex, CurrentMFTIndex);

    Status = NtfsFindMftRecord(Vcb, CurrentMFTIndex, SearchPattern, FirstEntry, TRUE, &CurrentMFTIndex);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("NtfsFindFileAt: NtfsFindMftRecord() failed with status 0x%08lx\n", Status);
        return Status;
    }

    *FileRecord = ExAllocatePoolWithTag(NonPagedPool, Vcb->NtfsInfo.BytesPerFileRecord, TAG_NTFS);
    if (*FileRecord == NULL)
    {
        DPRINT("NtfsFindFileAt: Can't allocate MFT record\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = ReadFileRecord(Vcb, CurrentMFTIndex, *FileRecord);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("NtfsFindFileAt: Can't read MFT record\n");
        ExFreePoolWithTag(*FileRecord, TAG_NTFS);
        return Status;
    }

    if (!((*FileRecord)->Flags & FRH_DIRECTORY))
    {
        Status = FindAttribute(Vcb, *FileRecord, AttributeData, L"", 0, DataContext);
        if (!NT_SUCCESS(Status))
        {
            DPRINT("NtfsFindFileAt: Can't find data attribute\n");
            ExFreePoolWithTag(*FileRecord, TAG_NTFS);
            return Status;
        }
    }
    else
    {
        *DataContext = NULL;
    }

    *MFTIndex = CurrentMFTIndex;

    return STATUS_SUCCESS;
}

/* EOF */
