/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS system libraries
 * FILE:            lib/kernel32/file/file.c
 * PURPOSE:         Directory functions
 * PROGRAMMER:      Ariadne ( ariadne@xs4all.nl)
 *                  Gerhard W. Gruber (sparhawk_at_gmx.at)
 *                  Dmitry Philippov (shedon@mail.ru)
 *                  Pierre Schweitzer (pierre@reactos.org)
 * UPDATE HISTORY:
 *                  Created 01/11/98
 *                  DP (29/07/2006)
 *                      Fix some bugs in the add_boot_rename_entry function
 */

/* INCLUDES *****************************************************************/

#include <k32.h>
#include <malloc.h>
#include <strsafe.h>
#define NDEBUG
#include <debug.h>
DEBUG_CHANNEL(kernel32file);

/* GLOBALS *****************************************************************/

/* DEFINES *****************************************************************/
typedef struct _COPY_PROGRESS_CONTEXT
{
    ULONG Flags;
    LPPROGRESS_ROUTINE UserRoutine;
    LPVOID UserData;
} COPY_PROGRESS_CONTEXT, *PCOPY_PROGRESS_CONTEXT;

/* FUNCTIONS ****************************************************************/
/*
 * @implemented
 */
NTSTATUS
WINAPI
BasepMoveFileDelayed(IN PUNICODE_STRING ExistingPath,
                     IN PUNICODE_STRING NewPath,
                     IN INT KeyId,
                     IN BOOL CreateIfNotFound)
{
#define STRING_LENGTH 0x400
    NTSTATUS Status;
    HANDLE KeyHandle;
    PVOID Buffer, BufferBegin;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PWSTR PendingOperations, BufferWrite;
    ULONG DataSize, BufferLength, StringLength = STRING_LENGTH;
    UNICODE_STRING SessionManagerString, PendingOperationsString;
    /* +6 because a INT shouldn't take more than 6 chars. Especially given the call path */
    WCHAR PendingOperationsBuffer[sizeof(L"PendingFileRenameOperations") / sizeof(WCHAR) + 6];

    RtlInitUnicodeString(&SessionManagerString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager");

    /* Select appropriate key for adding our file */
    if (KeyId == 1)
    {
        PendingOperations = L"PendingFileRenameOperations";
    }
    else
    {
        StringCbPrintfW(PendingOperationsBuffer, sizeof(PendingOperationsBuffer), L"PendingFileRenameOperations%d", KeyId);
        PendingOperations = PendingOperationsBuffer;
    }
    RtlInitUnicodeString(&PendingOperationsString, PendingOperations);

    InitializeObjectAttributes(&ObjectAttributes,
                               &SessionManagerString,
                               OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

    /* Open parent key */
    Status = NtCreateKey(&KeyHandle,
                         GENERIC_READ | GENERIC_WRITE,
                         &ObjectAttributes, 0, NULL,
                         REG_OPTION_NON_VOLATILE, NULL);
    if (Status == STATUS_ACCESS_DENIED)
    {
        Status = NtCreateKey(&KeyHandle,
                             GENERIC_READ | GENERIC_WRITE,
                             &ObjectAttributes, 0, NULL,
                             REG_OPTION_BACKUP_RESTORE, NULL);
    }

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    /* Reserve enough to read previous string + to append our with required null chars */
    BufferLength = NewPath->Length + ExistingPath->Length + STRING_LENGTH + 3 * sizeof(WCHAR);
    /* Check we didn't overflow */
    if (BufferLength < STRING_LENGTH)
    {
        NtClose(KeyHandle);
        return STATUS_BUFFER_TOO_SMALL;
    }

    while (TRUE)
    {
        /* Allocate output buffer */
        Buffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, BufferLength);
        if (Buffer == NULL)
        {
            NtClose(KeyHandle);
            return STATUS_NO_MEMORY;
        }

        Status = NtQueryValueKey(KeyHandle,
                                 &PendingOperationsString,
                                 KeyValuePartialInformation,
                                 Buffer, StringLength, &DataSize);
        if (Status != STATUS_BUFFER_OVERFLOW)
        {
            break;
        }

        /* If buffer was too small, then, reallocate one which is big enough */
        StringLength = DataSize;
        RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);
        BufferLength = ExistingPath->Length + StringLength + NewPath->Length + 3 * sizeof(WCHAR);
        if (BufferLength < StringLength)
        {
            NtClose(KeyHandle);
            return STATUS_BUFFER_TOO_SMALL;
        }
    }

    /* Check if it existed - if not, create only IF asked to */
    if (!NT_SUCCESS(Status) && (Status != STATUS_OBJECT_NAME_NOT_FOUND || !CreateIfNotFound))
    {
        NtClose(KeyHandle);
        RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);
        return Status;
    }

    if (!NT_SUCCESS(Status))
    {
        /* We didn't find any - ie, we create, so use complete buffer */
        BufferBegin = Buffer;
        BufferWrite = Buffer;
    }
    else
    {
        PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;

        /* Get data, our buffer begin and then where we should append data (+ null char) */
        BufferBegin = PartialInfo->Data;
        BufferWrite = (PWSTR)((ULONG_PTR)PartialInfo->Data + PartialInfo->DataLength + sizeof(WCHAR));
    }

    /* First copy existing */
    RtlCopyMemory(BufferWrite, ExistingPath->Buffer, ExistingPath->Length);
    BufferWrite += ExistingPath->Length / sizeof(WCHAR);
    /* And append null char */
    *BufferWrite = UNICODE_NULL;
    ++BufferWrite;
    /* Append destination */
    RtlCopyMemory(BufferWrite, NewPath->Buffer, NewPath->Length);
    BufferWrite += NewPath->Length / sizeof(WCHAR);
    /* And append two null char (end of string) */
    *BufferWrite = UNICODE_NULL;
    ++BufferWrite;
    *BufferWrite = UNICODE_NULL;

    /* Set new value */
    Status = NtSetValueKey(KeyHandle,
                           &PendingOperationsString,
                           0, REG_MULTI_SZ, BufferBegin,
                           (ULONG_PTR)BufferWrite - (ULONG_PTR)BufferBegin + sizeof(WCHAR));

    NtClose(KeyHandle);
    RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);

    return Status;
}


/*
 * @unimplemented
 */
NTSTATUS
WINAPI
BasepNotifyTrackingService(IN PHANDLE ExistingHandle,
                           IN POBJECT_ATTRIBUTES ObjectAttributes,
                           IN HANDLE NewHandle,
                           IN PUNICODE_STRING NewPath)
{
    return STATUS_NOT_IMPLEMENTED;
}


/*
 * @implemented
 */
DWORD
WINAPI
BasepMoveFileCopyProgress(IN LARGE_INTEGER TotalFileSize,
                          IN LARGE_INTEGER TotalBytesTransferred,
                          IN LARGE_INTEGER StreamSize,
                          IN LARGE_INTEGER StreamBytesTransferred,
                          IN DWORD dwStreamNumber,
                          IN DWORD dwCallbackReason,
                          IN HANDLE hSourceFile,
                          IN HANDLE hDestinationFile,
                          IN LPVOID lpData OPTIONAL)
{
    DWORD Ret = 0;
    PCOPY_PROGRESS_CONTEXT Context = (PCOPY_PROGRESS_CONTEXT)lpData;

    if (Context->Flags & MOVEFILE_WRITE_THROUGH)
    {
        if (!dwCallbackReason)
        {
            if (StreamBytesTransferred.QuadPart == StreamSize.QuadPart)
            {
                FlushFileBuffers(hDestinationFile);
            }
        }
    }

    if (Context->UserRoutine)
    {
        Ret = Context->UserRoutine(TotalFileSize,
                                   TotalBytesTransferred,
                                   StreamSize,
                                   StreamBytesTransferred,
                                   dwStreamNumber,
                                   dwCallbackReason,
                                   hSourceFile,
                                   hDestinationFile,
                                   Context->UserData);
    }

    return Ret;
}


/*
 * @implemented
 */
BOOL
WINAPI
MoveFileWithProgressW(IN LPCWSTR lpExistingFileName,
                      IN LPCWSTR lpNewFileName,
                      IN LPPROGRESS_ROUTINE lpProgressRoutine,
                      IN LPVOID lpData,
                      IN DWORD dwFlags)
{
    NTSTATUS Status;
    PWSTR NewBuffer;
    IO_STATUS_BLOCK IoStatusBlock;
    COPY_PROGRESS_CONTEXT CopyContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_RENAME_INFORMATION RenameInfo;
    UNICODE_STRING NewPathU, ExistingPathU;
    FILE_ATTRIBUTE_TAG_INFORMATION FileAttrTagInfo;
    HANDLE SourceHandle = INVALID_HANDLE_VALUE, NewHandle, ExistingHandle;
    BOOL Ret = FALSE, ReplaceIfExists, DelayUntilReboot, AttemptReopenWithoutReparse;

    DPRINT("MoveFileWithProgressW(%S, %S, %p, %p, %x)\n", lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);

    NewPathU.Buffer = NULL;
    ExistingPathU.Buffer = NULL;

    _SEH2_TRY
    {
        /* Don't allow renaming to a disk */
        if (lpNewFileName && RtlIsDosDeviceName_U(lpNewFileName))
        {
            BaseSetLastNTError(STATUS_OBJECT_NAME_COLLISION);
            _SEH2_LEAVE;
        }

        ReplaceIfExists = !!(dwFlags & MOVEFILE_REPLACE_EXISTING);

        /* Get file path */
        if (!RtlDosPathNameToNtPathName_U(lpExistingFileName, &ExistingPathU, NULL, NULL))
        {
            BaseSetLastNTError(STATUS_OBJECT_PATH_NOT_FOUND);
            _SEH2_LEAVE;
        }

        /* Sanitize input */
        DelayUntilReboot = !!(dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT);
        if (DelayUntilReboot && (dwFlags & MOVEFILE_CREATE_HARDLINK))
        {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            _SEH2_LEAVE;
        }

        /* Unless we manage a proper opening, we'll attempt to reopen without reparse support */
        AttemptReopenWithoutReparse = TRUE;
        InitializeObjectAttributes(&ObjectAttributes,
                                   &ExistingPathU,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
        /* Attempt to open source file */
        Status = NtOpenFile(&SourceHandle,
                            FILE_READ_ATTRIBUTES | DELETE | SYNCHRONIZE, 
                            &ObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            FILE_OPEN_FOR_BACKUP_INTENT | ((dwFlags & MOVEFILE_WRITE_THROUGH) ? FILE_WRITE_THROUGH : 0));
        if (!NT_SUCCESS(Status))
        {
            /* If we failed and the file doesn't exist, don't attempt to reopen without reparse */
            if (DelayUntilReboot &&
                (Status == STATUS_SHARING_VIOLATION || Status == STATUS_OBJECT_NAME_NOT_FOUND || Status == STATUS_OBJECT_PATH_NOT_FOUND))
            {
                /* Here we don't fail completely, as we postpone the operation to reboot
                 * File might exist afterwards, and we don't need a handle here
                 */
                SourceHandle = INVALID_HANDLE_VALUE;
                AttemptReopenWithoutReparse = FALSE;
            }
            /* If we failed for any reason than unsupported reparse, fail completely */
            else if (Status != STATUS_INVALID_PARAMETER)
            {
                BaseSetLastNTError(Status);
                _SEH2_LEAVE;
            }
        }
        else
        {
            /* We managed to open, so query information */
            Status = NtQueryInformationFile(SourceHandle,
                                            &IoStatusBlock,
                                            &FileAttrTagInfo,
                                            sizeof(FILE_ATTRIBUTE_TAG_INFORMATION),
                                            FileAttributeTagInformation);
            if (!NT_SUCCESS(Status))
            {
                /* Do not tolerate any other error than something related to not supported operation */
                if (Status != STATUS_NOT_IMPLEMENTED && Status != STATUS_INVALID_PARAMETER)
                {
                    BaseSetLastNTError(Status);
                    _SEH2_LEAVE;
                }

                /* Not a reparse point, no need to reopen, it's fine */
                AttemptReopenWithoutReparse = FALSE;
            }
            /* Validate the reparse point (do we support it?) */
            else if (FileAttrTagInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT &&
                     FileAttrTagInfo.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
            {
                NtClose(SourceHandle);
                SourceHandle = INVALID_HANDLE_VALUE;
            }
        }

        /* Simply reopen if required */
        if (AttemptReopenWithoutReparse)
        {
            Status = NtOpenFile(&SourceHandle,
                                DELETE | SYNCHRONIZE,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                ((dwFlags & MOVEFILE_WRITE_THROUGH) ? FILE_WRITE_THROUGH : 0));
            if (!NT_SUCCESS(Status))
            {
                BaseSetLastNTError(Status);
                _SEH2_LEAVE;
            }
        }

        /* Nullify string if we're to use it */
        if (DelayUntilReboot && !lpNewFileName)
        {
            RtlInitUnicodeString(&NewPathU, 0);
        }
        /* Check whether path exists */
        else if (!RtlDosPathNameToNtPathName_U(lpNewFileName, &NewPathU, 0, 0))
        {
            BaseSetLastNTError(STATUS_OBJECT_PATH_NOT_FOUND);
            _SEH2_LEAVE;
        }

        /* Handle postponed renaming */
        if (DelayUntilReboot)
        {
            /* If new file exists and we're allowed to replace, then mark the path with ! */
            if (ReplaceIfExists && NewPathU.Length)
            {
                NewBuffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, NewPathU.Length + sizeof(WCHAR));
                if (NewBuffer == NULL)
                {
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    _SEH2_LEAVE;
                }

                NewBuffer[0] = L'!';
                RtlCopyMemory(&NewBuffer[1], NewPathU.Buffer, NewPathU.Length);
                NewPathU.Length += sizeof(WCHAR);
                NewPathU.MaximumLength += sizeof(WCHAR);
                RtlFreeHeap(RtlGetProcessHeap(), 0, NewPathU.Buffer);
                NewPathU.Buffer = NewBuffer;
            }

            /* Check whether 'copy' renaming is allowed if required */
            if (RtlDetermineDosPathNameType_U(lpExistingFileName) == RtlPathTypeUncAbsolute || dwFlags & MOVEFILE_COPY_ALLOWED)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                /* First, probe 2nd key to see whether it exists - if so, it will be appended there */
                Status = BasepMoveFileDelayed(&ExistingPathU, &NewPathU, 2, FALSE);
                if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                {
                    /* If doesn't exist, append to first key first, creating it if it doesn't exist */
                    Status = BasepMoveFileDelayed(&ExistingPathU, &NewPathU, 1, TRUE);

                    if (Status == STATUS_INSUFFICIENT_RESOURCES)
                    {
                        /* If it failed because it's too big, then create 2nd key and put it there */
                        Status = BasepMoveFileDelayed(&ExistingPathU, &NewPathU, 2, TRUE);
                    }
                }
            }

            /* If we failed at some point, return the error */
            if (!NT_SUCCESS(Status))
            {
                BaseSetLastNTError(Status);
                _SEH2_LEAVE;
            }

            Ret = TRUE;
            _SEH2_LEAVE;
        }

        /* At that point, we MUST have a source handle */
        ASSERT(SourceHandle != INVALID_HANDLE_VALUE);

        /* Allocate renaming buffer and fill it */
        RenameInfo = RtlAllocateHeap(RtlGetProcessHeap(), 0, NewPathU.Length + sizeof(FILE_RENAME_INFORMATION));
        if (RenameInfo == NULL)
        {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            _SEH2_LEAVE;
        }

        RtlCopyMemory(&RenameInfo->FileName, NewPathU.Buffer, NewPathU.Length);
        RenameInfo->ReplaceIfExists = ReplaceIfExists;
        RenameInfo->RootDirectory = 0;
        RenameInfo->FileNameLength = NewPathU.Length;

        /* Attempt to rename the file */
        Status = NtSetInformationFile(SourceHandle,
                                      &IoStatusBlock,
                                      RenameInfo,
                                      NewPathU.Length + sizeof(FILE_RENAME_INFORMATION),
                                      ((dwFlags & MOVEFILE_CREATE_HARDLINK) ? FileLinkInformation : FileRenameInformation));
        RtlFreeHeap(RtlGetProcessHeap(), 0, RenameInfo);
        if (NT_SUCCESS(Status))
        {
            /* If it succeed, all fine, quit */
            Ret = TRUE;
            _SEH2_LEAVE;
        }
        /* If we failed for any other reason than not the same device, fail
         * If we failed because of different devices, only allow renaming if user allowed copy
         */
        if (Status != STATUS_NOT_SAME_DEVICE || !(dwFlags & MOVEFILE_COPY_ALLOWED))
        {
            /* ReactOS hack! To be removed once all FSD have proper renaming support
             * Just leave status to error and leave 
             */
            if (Status == STATUS_NOT_IMPLEMENTED)
            {
                DPRINT1("Forcing copy, renaming not supported by FSD\n");
            }
            else
            {
                BaseSetLastNTError(Status);
                _SEH2_LEAVE;
            }
        }

        /* Close source file */
        NtClose(SourceHandle);
        SourceHandle = INVALID_HANDLE_VALUE;

        /* Issue the copy of the file */
        CopyContext.Flags = dwFlags;
        CopyContext.UserRoutine = lpProgressRoutine;
        CopyContext.UserData = lpData;
        NewHandle = INVALID_HANDLE_VALUE;
        ExistingHandle = INVALID_HANDLE_VALUE;

        Ret = BasepCopyFileExW(lpExistingFileName,
                               lpNewFileName,
                               BasepMoveFileCopyProgress,
                               &CopyContext,
                               NULL,
                               (ReplaceIfExists == 0) | COPY_FILE_OPEN_SOURCE_FOR_WRITE,
                               0,
                               &ExistingHandle,
                               &NewHandle);
        if (!Ret)
        {
            /* If it failed, don't leak any handle */
            if (ExistingHandle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(ExistingHandle);
                ExistingHandle = INVALID_HANDLE_VALUE;
            }
        }
        else if (ExistingHandle != INVALID_HANDLE_VALUE)
        {
            if (NewHandle != INVALID_HANDLE_VALUE)
            {
                /* If copying succeed, notify */
                Status = BasepNotifyTrackingService(&ExistingHandle, &ObjectAttributes, NewHandle, &NewPathU);
                if (!NT_SUCCESS(Status))
                {
                    /* Fail in case it had to succeed */
                    if (dwFlags & MOVEFILE_FAIL_IF_NOT_TRACKABLE)
                    {
                        if (NewHandle != INVALID_HANDLE_VALUE)
                            CloseHandle(NewHandle);
                        NewHandle = INVALID_HANDLE_VALUE;
                        DeleteFileW(lpNewFileName);
                        Ret = FALSE;
                        BaseSetLastNTError(Status);
                    }
                }
            }

            CloseHandle(ExistingHandle);
            ExistingHandle = INVALID_HANDLE_VALUE;
        }

        /* In case copy worked, close file */
        if (NewHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(NewHandle);
            NewHandle = INVALID_HANDLE_VALUE;
        }

        /* If it succeed, delete source file */
        if (Ret)
        {
            if (!DeleteFileW(lpExistingFileName))
            {
                /* Reset file attributes if required */
                SetFileAttributesW(lpExistingFileName, FILE_ATTRIBUTE_NORMAL);
                DeleteFileW(lpExistingFileName);
            }
        }
    }
    _SEH2_FINALLY
    {
        if (SourceHandle != INVALID_HANDLE_VALUE)
            NtClose(SourceHandle);

        RtlFreeHeap(RtlGetProcessHeap(), 0, ExistingPathU.Buffer);
        RtlFreeHeap(RtlGetProcessHeap(), 0, NewPathU.Buffer);
    }
    _SEH2_END;

    return Ret;
}


/*
 * @implemented
 */
BOOL
WINAPI
MoveFileWithProgressA(IN LPCSTR lpExistingFileName,
                      IN LPCSTR lpNewFileName OPTIONAL,
                      IN LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
                      IN LPVOID lpData OPTIONAL,
                      IN DWORD dwFlags)
{
    BOOL Ret;
    UNICODE_STRING ExistingFileNameW, NewFileNameW;

    if (!Basep8BitStringToDynamicUnicodeString(&ExistingFileNameW, lpExistingFileName))
    {
        return FALSE;
    }

    if (lpNewFileName)
    {
        if (!Basep8BitStringToDynamicUnicodeString(&NewFileNameW, lpNewFileName))
        {
            RtlFreeUnicodeString(&ExistingFileNameW);
            return FALSE;
        }
    }
    else
    {
        NewFileNameW.Buffer = NULL;
    }

    Ret = MoveFileWithProgressW(ExistingFileNameW.Buffer, NewFileNameW.Buffer, lpProgressRoutine, lpData, dwFlags);

    RtlFreeUnicodeString(&ExistingFileNameW);
    RtlFreeUnicodeString(&NewFileNameW);

    return Ret;
}


/*
 * @implemented
 */
BOOL
WINAPI
MoveFileW(IN LPCWSTR lpExistingFileName,
          IN LPCWSTR lpNewFileName)
{
    return MoveFileWithProgressW(lpExistingFileName,
                                 lpNewFileName,
                                 NULL,
                                 NULL,
                                 MOVEFILE_COPY_ALLOWED);
}


/*
 * @implemented
 */
BOOL
WINAPI
MoveFileExW(IN LPCWSTR lpExistingFileName,
            IN LPCWSTR lpNewFileName OPTIONAL,
            IN DWORD dwFlags)
{
    return MoveFileWithProgressW(lpExistingFileName,
	                         lpNewFileName,
	                         NULL,
	                         NULL,
	                         dwFlags);
}


/*
 * @implemented
 */
BOOL
WINAPI
MoveFileA(IN LPCSTR lpExistingFileName,
          IN LPCSTR lpNewFileName)
{
    return MoveFileWithProgressA(lpExistingFileName,
                                 lpNewFileName,
	                         NULL,
	                         NULL,
                                 MOVEFILE_COPY_ALLOWED);
}


/*
 * @implemented
 */
BOOL
WINAPI
MoveFileExA(IN LPCSTR lpExistingFileName,
            IN LPCSTR lpNewFileName OPTIONAL,
            IN DWORD dwFlags)
{
    return MoveFileWithProgressA(lpExistingFileName,
                                 lpNewFileName,
                                 NULL,
                                 NULL,
                                 dwFlags);
}

/*
 * @implemented
 */
BOOL
WINAPI
ReplaceFileA(IN LPCSTR lpReplacedFileName,
             IN LPCSTR lpReplacementFileName,
             IN LPCSTR lpBackupFileName OPTIONAL,
             IN DWORD dwReplaceFlags,
             IN LPVOID lpExclude,
             IN LPVOID lpReserved)
{
    BOOL Ret;
    UNICODE_STRING ReplacedFileNameW, ReplacementFileNameW, BackupFileNameW;

    if (!lpReplacedFileName || !lpReplacementFileName || lpExclude || lpReserved || dwReplaceFlags & ~(REPLACEFILE_WRITE_THROUGH | REPLACEFILE_IGNORE_MERGE_ERRORS))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString(&ReplacedFileNameW, lpReplacedFileName))
    {
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString(&ReplacementFileNameW, lpReplacementFileName))
    {
        RtlFreeUnicodeString(&ReplacedFileNameW);
        return FALSE;
    }

    if (lpBackupFileName)
    {
        if (!Basep8BitStringToDynamicUnicodeString(&BackupFileNameW, lpBackupFileName))
        {
            RtlFreeUnicodeString(&ReplacementFileNameW);
            RtlFreeUnicodeString(&ReplacedFileNameW);
            return FALSE;
        }
    }
    else
    {
        BackupFileNameW.Buffer = NULL;
    }

    Ret = ReplaceFileW(ReplacedFileNameW.Buffer, ReplacementFileNameW.Buffer, BackupFileNameW.Buffer, dwReplaceFlags, 0, 0);

    if (lpBackupFileName)
    {
        RtlFreeUnicodeString(&BackupFileNameW);
    }
    RtlFreeUnicodeString(&ReplacementFileNameW);
    RtlFreeUnicodeString(&ReplacedFileNameW);

    return Ret;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
ReplaceFileW(
    LPCWSTR lpReplacedFileName,
    LPCWSTR lpReplacementFileName,
    LPCWSTR lpBackupFileName,
    DWORD   dwReplaceFlags,
    LPVOID  lpExclude,
    LPVOID  lpReserved
    )
{
    HANDLE hReplaced = NULL, hReplacement = NULL;
    UNICODE_STRING NtReplacedName = { 0, 0, NULL };
    UNICODE_STRING NtReplacementName = { 0, 0, NULL };
    DWORD Error = ERROR_SUCCESS;
    NTSTATUS Status;
    BOOL Ret = FALSE;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PVOID Buffer = NULL ;

    if (dwReplaceFlags)
        FIXME("Ignoring flags %x\n", dwReplaceFlags);

    /* First two arguments are mandatory */
    if (!lpReplacedFileName || !lpReplacementFileName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /* Back it up */
    if(lpBackupFileName)
    {
        if(!CopyFileW(lpReplacedFileName, lpBackupFileName, FALSE))
        {
            Error = GetLastError();
            goto Cleanup ;
        }
    }

    /* Open the "replaced" file for reading and writing */
    if (!(RtlDosPathNameToNtPathName_U(lpReplacedFileName, &NtReplacedName, NULL, NULL)))
    {
        Error = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &NtReplacedName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenFile(&hReplaced,
                        GENERIC_READ | GENERIC_WRITE | DELETE | SYNCHRONIZE | WRITE_DAC,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            Error = ERROR_FILE_NOT_FOUND;
        else
            Error = ERROR_UNABLE_TO_REMOVE_REPLACED;
        goto Cleanup;
    }

    /* Blank it */
    SetEndOfFile(hReplaced) ;

    /*
     * Open the replacement file for reading, writing, and deleting
     * (deleting is needed when finished)
     */
    if (!(RtlDosPathNameToNtPathName_U(lpReplacementFileName, &NtReplacementName, NULL, NULL)))
    {
        Error = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &NtReplacementName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenFile(&hReplacement,
                        GENERIC_READ | DELETE | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        0,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_DELETE_ON_CLOSE);

    if (!NT_SUCCESS(Status))
    {
        Error = RtlNtStatusToDosError(Status);
        goto Cleanup;
    }

    Buffer = RtlAllocateHeap(RtlGetProcessHeap(), HEAP_ZERO_MEMORY, 0x10000) ;
    if (!Buffer)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup ;
    }
    while (Status != STATUS_END_OF_FILE)
    {
        Status = NtReadFile(hReplacement, NULL, NULL, NULL, &IoStatusBlock, Buffer, 0x10000, NULL, NULL) ;
        if (NT_SUCCESS(Status))
        {
            Status = NtWriteFile(hReplaced, NULL, NULL, NULL, &IoStatusBlock, Buffer,
                    IoStatusBlock.Information, NULL, NULL) ;
            if (!NT_SUCCESS(Status))
            {
                Error = RtlNtStatusToDosError(Status);
                goto Cleanup;
            }
        }
        else if (Status != STATUS_END_OF_FILE)
        {
            Error = RtlNtStatusToDosError(Status);
            goto Cleanup;
        }
    }

    Ret = TRUE;

    /* Perform resource cleanup */
Cleanup:
    if (hReplaced) NtClose(hReplaced);
    if (hReplacement) NtClose(hReplacement);
    if (Buffer) RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);

    if (NtReplacementName.Buffer)
        RtlFreeHeap(GetProcessHeap(), 0, NtReplacementName.Buffer);
    if (NtReplacedName.Buffer)
        RtlFreeHeap(GetProcessHeap(), 0, NtReplacedName.Buffer);

    /* If there was an error, set the error code */
    if(!Ret)
    {
        TRACE("ReplaceFileW failed (error=%lu)\n", Error);
        SetLastError(Error);
    }
    return Ret;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
PrivMoveFileIdentityW(DWORD Unknown1, DWORD Unknown2, DWORD Unknown3)
{
    STUB;
    return FALSE;
}

/* EOF */
