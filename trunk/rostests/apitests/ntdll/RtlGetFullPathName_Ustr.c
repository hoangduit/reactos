/*
 * PROJECT:         ReactOS api tests
 * LICENSE:         GPLv2+ - See COPYING in the top level directory
 * PURPOSE:         Test for RtlGetFullPathName_Ustr
 * PROGRAMMER:      Thomas Faber <thfabba@gmx.de>
 */

#define WIN32_NO_STATUS
#define UNICODE
#include <wine/test.h>
#include <pseh/pseh2.h>
#include <ndk/rtlfuncs.h>

/*
ULONG
NTAPI
RtlGetFullPathName_Ustr(
    IN PCUNICODE_STRING FileName,
    IN ULONG Size,
    IN PWSTR Buffer,
    OUT PCWSTR *ShortName,
    OUT PBOOLEAN InvalidName,
    OUT RTL_PATH_TYPE* PathType
);
*/

/* This seems to be a struct of some kind in Windows 7... returns 0 or 32 in the second member */
typedef struct _PATH_TYPE_AND_UNKNOWN
{
    RTL_PATH_TYPE Type;
    ULONG Unknown;
} PATH_TYPE_AND_UNKNOWN;

static
ULONG
(NTAPI
*RtlGetFullPathName_Ustr)(
    IN PCUNICODE_STRING FileName,
    IN ULONG Size,
    IN PWSTR Buffer,
    OUT PCWSTR *ShortName,
    OUT PBOOLEAN InvalidName,
    OUT PATH_TYPE_AND_UNKNOWN *PathType
)
//= (PVOID)0x7c83086c // 2003 sp1 x86
//= (PVOID)0x77ef49f0 // 2003 sp1 x64
//= (PVOID)0x7769a3dd // win7 sp1 wow64
;

#define StartSeh()                  ExceptionStatus = STATUS_SUCCESS; _SEH2_TRY {
#define EndSeh(ExpectedStatus)      } _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER) { ExceptionStatus = _SEH2_GetExceptionCode(); } _SEH2_END; ok(ExceptionStatus == ExpectedStatus, "Exception %lx, expected %lx\n", ExceptionStatus, ExpectedStatus)

#define ok_eq_ustr(str1, str2) do {                                                     \
        ok((str1)->Buffer        == (str2)->Buffer,        "Buffer modified\n");        \
        ok((str1)->Length        == (str2)->Length,        "Length modified\n");        \
        ok((str1)->MaximumLength == (str2)->MaximumLength, "MaximumLength modified\n"); \
    } while (0)

static
BOOLEAN
CheckStringBuffer(
    PCWSTR Buffer,
    SIZE_T Length,
    SIZE_T MaximumLength,
    PCWSTR Expected)
{
    SIZE_T ExpectedLength = wcslen(Expected) * sizeof(WCHAR);
    SIZE_T EqualLength;
    BOOLEAN Result = TRUE;
    SIZE_T i;

    if (Length != ExpectedLength)
    {
        ok(0, "String length is %lu, expected %lu\n", (ULONG)Length, (ULONG)ExpectedLength);
        Result = FALSE;
    }

    EqualLength = RtlCompareMemory(Buffer, Expected, Length);
    if (EqualLength != Length)
    {
        ok(0, "String is '%S', expected '%S'\n", Buffer, Expected);
        Result = FALSE;
    }

    if (Buffer[Length / sizeof(WCHAR)] != UNICODE_NULL)
    {
        ok(0, "Not null terminated\n");
        Result = FALSE;
    }

    /* The function nulls the rest of the buffer! */
    for (i = Length + sizeof(UNICODE_NULL); i < MaximumLength; i++)
    {
        UCHAR Char = ((PUCHAR)Buffer)[i];
        if (Char != 0)
        {
            ok(0, "Found 0x%x at offset %lu, expected 0x%x\n", Char, (ULONG)i, 0);
            /* Don't count this as a failure unless the string was actually wrong */
            //Result = FALSE;
            /* Don't flood the log */
            break;
        }
    }

    return Result;
}

static
BOOLEAN
CheckBuffer(
    PVOID Buffer,
    SIZE_T Size,
    UCHAR Value)
{
    PUCHAR Array = Buffer;
    SIZE_T i;

    for (i = 0; i < Size; i++)
        if (Array[i] != Value)
        {
            trace("Expected %x, found %x at offset %lu\n", Value, Array[i], (ULONG)i);
            return FALSE;
        }
    return TRUE;
}

#define RtlPathTypeNotSet 123
#define InvalidPointer ((PVOID)0x0123456789ABCDEFULL)

/* winetest_platform is "windows" for us, so broken() doesn't do what it should :( */
#undef broken
#define broken(x) 0

typedef enum
{
    PrefixNone,
    PrefixCurrentDrive,
    PrefixCurrentPath,
    PrefixCurrentPathWithoutLastPart
} PREFIX_TYPE;

static
VOID
RunTestCases(VOID)
{
    /* TODO: don't duplicate this in the other tests */
    /* TODO: Drive Relative tests don't work yet if the current drive isn't C: */
    struct
    {
        PCWSTR FileName;
        PREFIX_TYPE PrefixType;
        PCWSTR FullPathName;
        RTL_PATH_TYPE PathType;
        PREFIX_TYPE FilePartPrefixType;
        SIZE_T FilePartSize;
    } TestCases[] =
    {
        { L"C:",                 PrefixCurrentPath, L"", RtlPathTypeDriveRelative, PrefixCurrentPathWithoutLastPart },
        { L"C:\\",               PrefixNone, L"C:\\", RtlPathTypeDriveAbsolute },
        { L"C:\\test",           PrefixNone, L"C:\\test", RtlPathTypeDriveAbsolute, PrefixCurrentDrive },
        { L"C:\\test\\",         PrefixNone, L"C:\\test\\", RtlPathTypeDriveAbsolute },
        { L"C:/test/",           PrefixNone, L"C:\\test\\", RtlPathTypeDriveAbsolute },

        { L"C:\\\\test",         PrefixNone, L"C:\\test", RtlPathTypeDriveAbsolute, PrefixCurrentDrive },
        { L"test",               PrefixCurrentPath, L"\\test", RtlPathTypeRelative, PrefixCurrentPath, sizeof(WCHAR) },
        { L"\\test",             PrefixCurrentDrive, L"test", RtlPathTypeRooted, PrefixCurrentDrive },
        { L"/test",              PrefixCurrentDrive, L"test", RtlPathTypeRooted, PrefixCurrentDrive },
        { L".\\test",            PrefixCurrentPath, L"\\test", RtlPathTypeRelative, PrefixCurrentPath, sizeof(WCHAR) },

        { L"\\.",                PrefixCurrentDrive, L"", RtlPathTypeRooted },
        { L"\\.\\",              PrefixCurrentDrive, L"", RtlPathTypeRooted },
        { L"\\\\.",              PrefixNone, L"\\\\.\\", RtlPathTypeRootLocalDevice },
        { L"\\\\.\\",            PrefixNone, L"\\\\.\\", RtlPathTypeLocalDevice },
        { L"\\\\.\\Something\\", PrefixNone, L"\\\\.\\Something\\", RtlPathTypeLocalDevice },

        { L"\\??\\",             PrefixCurrentDrive, L"??\\", RtlPathTypeRooted },
        { L"\\??\\C:",           PrefixCurrentDrive, L"??\\C:", RtlPathTypeRooted, PrefixCurrentDrive, 3 * sizeof(WCHAR) },
        { L"\\??\\C:\\",         PrefixCurrentDrive, L"??\\C:\\", RtlPathTypeRooted },
        { L"\\??\\C:\\test",     PrefixCurrentDrive, L"??\\C:\\test", RtlPathTypeRooted, PrefixCurrentDrive, 6 * sizeof(WCHAR) },
        { L"\\??\\C:\\test\\",   PrefixCurrentDrive, L"??\\C:\\test\\", RtlPathTypeRooted },

        { L"\\\\??\\",           PrefixNone, L"\\\\??\\", RtlPathTypeUncAbsolute },
        { L"\\\\??\\C:",         PrefixNone, L"\\\\??\\C:", RtlPathTypeUncAbsolute },
        { L"\\\\??\\C:\\",       PrefixNone, L"\\\\??\\C:\\", RtlPathTypeUncAbsolute },
        { L"\\\\??\\C:\\test",   PrefixNone, L"\\\\??\\C:\\test", RtlPathTypeUncAbsolute, PrefixNone, sizeof(L"\\\\??\\C:\\") },
        { L"\\\\??\\C:\\test\\", PrefixNone, L"\\\\??\\C:\\test\\", RtlPathTypeUncAbsolute },
    };
    NTSTATUS ExceptionStatus;
    ULONG Length;
    UNICODE_STRING FileName;
    WCHAR FullPathNameBuffer[MAX_PATH];
    UNICODE_STRING TempString;
    const WCHAR *ShortName;
    BOOLEAN NameInvalid;
    PATH_TYPE_AND_UNKNOWN PathType;
    WCHAR ExpectedPathName[MAX_PATH];
    SIZE_T ExpectedFilePartSize;
    const WCHAR *ExpectedShortName;
    const INT TestCount = sizeof(TestCases) / sizeof(TestCases[0]);
    INT i;
    BOOLEAN Okay;

    for (i = 0; i < TestCount; i++)
    {
        trace("i = %d\n", i);
        switch (TestCases[i].PrefixType)
        {
            case PrefixNone:
                ExpectedPathName[0] = UNICODE_NULL;
                break;
            case PrefixCurrentDrive:
                GetCurrentDirectoryW(sizeof(ExpectedPathName) / sizeof(WCHAR), ExpectedPathName);
                ExpectedPathName[3] = UNICODE_NULL;
                break;
            case PrefixCurrentPath:
            {
                ULONG Length;
                Length = GetCurrentDirectoryW(sizeof(ExpectedPathName) / sizeof(WCHAR), ExpectedPathName);
                if (Length == 3 && TestCases[i].FullPathName[0])
                    ExpectedPathName[2] = UNICODE_NULL;
                break;
            }
            default:
                skip(0, "Invalid test!\n");
                continue;
        }
        wcscat(ExpectedPathName, TestCases[i].FullPathName);
        RtlInitUnicodeString(&FileName, TestCases[i].FileName);
        RtlFillMemory(FullPathNameBuffer, sizeof(FullPathNameBuffer), 0xAA);
        TempString = FileName;
        PathType.Type = RtlPathTypeNotSet;
        PathType.Unknown = 1234;
        ShortName = InvalidPointer;
        NameInvalid = (BOOLEAN)-1;
        Length = 1234;
        StartSeh()
            Length = RtlGetFullPathName_Ustr(&FileName,
                                             sizeof(FullPathNameBuffer),
                                             FullPathNameBuffer,
                                             &ShortName,
                                             &NameInvalid,
                                             &PathType);
        EndSeh(STATUS_SUCCESS);
        ok_eq_ustr(&FileName, &TempString);
        Okay = CheckStringBuffer(FullPathNameBuffer, Length, sizeof(FullPathNameBuffer), ExpectedPathName);
        ok(Okay, "Wrong path name '%S', expected '%S'\n", FullPathNameBuffer, ExpectedPathName);
        switch (TestCases[i].FilePartPrefixType)
        {
            case PrefixNone:
                ExpectedFilePartSize = 0;
                break;
            case PrefixCurrentDrive:
                ExpectedFilePartSize = sizeof(L"C:\\");
                break;
            case PrefixCurrentPath:
                ExpectedFilePartSize = GetCurrentDirectoryW(0, NULL) * sizeof(WCHAR);
                if (ExpectedFilePartSize == sizeof(L"C:\\"))
                    ExpectedFilePartSize -= sizeof(WCHAR);
                break;
            case PrefixCurrentPathWithoutLastPart:
            {
                WCHAR CurrentPath[MAX_PATH];
                PCWSTR BackSlash;
                ExpectedFilePartSize = GetCurrentDirectoryW(sizeof(CurrentPath) / sizeof(WCHAR), CurrentPath) * sizeof(WCHAR) + sizeof(UNICODE_NULL);
                if (ExpectedFilePartSize == sizeof(L"C:\\"))
                    ExpectedFilePartSize = 0;
                else
                {
                    BackSlash = wcsrchr(CurrentPath, L'\\');
                    if (BackSlash)
                        ExpectedFilePartSize -= wcslen(BackSlash + 1) * sizeof(WCHAR);
                    else
                        ok(0, "GetCurrentDirectory returned %S\n", CurrentPath);
                }
                break;
            }
            default:
                skip(0, "Invalid test!\n");
                continue;
        }
        ExpectedFilePartSize += TestCases[i].FilePartSize;
        if (ExpectedFilePartSize == 0)
        {
            ExpectedShortName = NULL;
        }
        else
        {
            ExpectedFilePartSize = (ExpectedFilePartSize - sizeof(UNICODE_NULL)) / sizeof(WCHAR);
            ExpectedShortName = FullPathNameBuffer + ExpectedFilePartSize;
        }
        ok(ShortName == ExpectedShortName,
            "ShortName = %p, expected %p\n", ShortName, ExpectedShortName);
        ok(NameInvalid == FALSE, "NameInvalid = %u\n", NameInvalid);
        ok(PathType.Type == TestCases[i].PathType, "PathType = %d, expected %d\n", PathType.Type, TestCases[i].PathType);
        ok(PathType.Unknown == 1234 ||
            broken(PathType.Unknown == 0) ||
            broken(PathType.Unknown == 32), "Unknown = %d\n", PathType.Unknown);
    }
}

START_TEST(RtlGetFullPathName_Ustr)
{
    NTSTATUS ExceptionStatus;
    ULONG Length;
    UNICODE_STRING FileName;
    UNICODE_STRING TempString;
    PCWSTR ShortName;
    BOOLEAN NameInvalid;
    BOOLEAN NameInvalidArray[sizeof(ULONGLONG)];
    PATH_TYPE_AND_UNKNOWN PathType;
    BOOLEAN Okay;

    if (!RtlGetFullPathName_Ustr)
    {
        RtlGetFullPathName_Ustr = (PVOID)GetProcAddress(GetModuleHandle(L"ntdll"), "RtlGetFullPathName_Ustr");
        if (!RtlGetFullPathName_Ustr)
        {
            skip("RtlGetFullPathName_Ustr unavailable\n");
            return;
        }
    }

    /* NULL parameters */
    StartSeh()
        RtlGetFullPathName_Ustr(NULL, 0, NULL, NULL, NULL, NULL);
    EndSeh(STATUS_ACCESS_VIOLATION);

    RtlInitUnicodeString(&FileName, NULL);
    TempString = FileName;
    StartSeh()
        RtlGetFullPathName_Ustr(&FileName, 0, NULL, NULL, NULL, NULL);
    EndSeh(STATUS_ACCESS_VIOLATION);
    ok_eq_ustr(&FileName, &TempString);

    RtlInitUnicodeString(&FileName, L"");
    TempString = FileName;
    StartSeh()
        RtlGetFullPathName_Ustr(&FileName, 0, NULL, NULL, NULL, NULL);
    EndSeh(STATUS_ACCESS_VIOLATION);
    ok_eq_ustr(&FileName, &TempString);

    PathType.Type = RtlPathTypeNotSet;
    PathType.Unknown = 1234;
    StartSeh()
        RtlGetFullPathName_Ustr(NULL, 0, NULL, NULL, NULL, &PathType);
    EndSeh(STATUS_ACCESS_VIOLATION);
    ok(PathType.Type == RtlPathTypeUnknown, "PathType = %d\n", PathType.Type);
    ok(PathType.Unknown == 1234 ||
        broken(PathType.Unknown == 0) /* Win7 */, "Unknown = %d\n", PathType.Unknown);

    /* check what else is initialized before it crashes */
    PathType.Type = RtlPathTypeNotSet;
    PathType.Unknown = 1234;
    ShortName = InvalidPointer;
    NameInvalid = (BOOLEAN)-1;
    StartSeh()
        RtlGetFullPathName_Ustr(NULL, 0, NULL, &ShortName, &NameInvalid, &PathType);
    EndSeh(STATUS_ACCESS_VIOLATION);
    ok(NameInvalid == FALSE, "NameInvalid = %u\n", NameInvalid);
    ok(ShortName == InvalidPointer ||
        broken(ShortName == NULL) /* Win7 */, "ShortName = %p\n", ShortName);
    ok(PathType.Type == RtlPathTypeUnknown, "PathType = %d\n", PathType.Type);
    ok(PathType.Unknown == 1234 ||
       broken(PathType.Unknown == 0) /* Win7 */, "Unknown = %d\n", PathType.Unknown);

    RtlInitUnicodeString(&FileName, L"");
    TempString = FileName;
    ShortName = InvalidPointer;
    NameInvalid = (BOOLEAN)-1;
    StartSeh()
        RtlGetFullPathName_Ustr(&FileName, 0, NULL, &ShortName, &NameInvalid, NULL);
    EndSeh(STATUS_ACCESS_VIOLATION);
    ok_eq_ustr(&FileName, &TempString);
    ok(ShortName == InvalidPointer ||
        broken(ShortName == NULL) /* Win7 */, "ShortName = %p\n", ShortName);
    ok(NameInvalid == FALSE, "NameInvalid = %u\n", NameInvalid);

    /* This is the first one that doesn't crash. FileName and PathType cannot be NULL */
    RtlInitUnicodeString(&FileName, NULL);
    TempString = FileName;
    PathType.Type = RtlPathTypeNotSet;
    PathType.Unknown = 1234;
    StartSeh()
        Length = RtlGetFullPathName_Ustr(&FileName, 0, NULL, NULL, NULL, &PathType);
        ok(Length == 0, "Length = %lu\n", Length);
    EndSeh(STATUS_SUCCESS);
    ok_eq_ustr(&FileName, &TempString);
    ok(PathType.Type == RtlPathTypeUnknown, "PathType = %d\n", PathType.Type);
    ok(PathType.Unknown == 1234 ||
       broken(PathType.Unknown == 0) /* Win7 */, "Unknown = %d\n", PathType.Unknown);

    RtlInitUnicodeString(&FileName, L"");
    TempString = FileName;
    PathType.Type = RtlPathTypeNotSet;
    PathType.Unknown = 1234;
    StartSeh()
        Length = RtlGetFullPathName_Ustr(&FileName, 0, NULL, NULL, NULL, &PathType);
        ok(Length == 0, "Length = %lu\n", Length);
    EndSeh(STATUS_SUCCESS);
    ok_eq_ustr(&FileName, &TempString);
    ok(PathType.Type == RtlPathTypeUnknown, "PathType = %d\n", PathType.Type);
    ok(PathType.Unknown == 1234 ||
       broken(PathType.Unknown == 0) /* Win7 */, "Unknown = %d\n", PathType.Unknown);

    /* Show that NameInvalid is indeed BOOLEAN */
    RtlInitUnicodeString(&FileName, L"");
    TempString = FileName;
    PathType.Type = RtlPathTypeNotSet;
    PathType.Unknown = 1234;
    RtlFillMemory(NameInvalidArray, sizeof(NameInvalidArray), 0x55);
    StartSeh()
        Length = RtlGetFullPathName_Ustr(&FileName, 0, NULL, NULL, NameInvalidArray, &PathType);
        ok(Length == 0, "Length = %lu\n", Length);
    EndSeh(STATUS_SUCCESS);
    ok_eq_ustr(&FileName, &TempString);
    ok(PathType.Type == RtlPathTypeUnknown, "PathType = %d\n", PathType.Type);
    ok(PathType.Unknown == 1234 ||
       broken(PathType.Unknown == 0) /* Win7 */, "Unknown = %d\n", PathType.Unknown);
    ok(NameInvalidArray[0] == FALSE, "NameInvalid = %u\n", NameInvalidArray[0]);
    Okay = CheckBuffer(NameInvalidArray + 1, sizeof(NameInvalidArray) - sizeof(NameInvalidArray[0]), 0x55);
    ok(Okay, "CheckBuffer failed\n");

    /* Give it a valid path */
    RtlInitUnicodeString(&FileName, L"C:\\test");
    TempString = FileName;
    PathType.Type = RtlPathTypeNotSet;
    PathType.Unknown = 1234;
    StartSeh()
        Length = RtlGetFullPathName_Ustr(&FileName, 0, NULL, NULL, NULL, &PathType);
        ok(Length == sizeof(L"C:\\test"), "Length = %lu\n", Length);
    EndSeh(STATUS_SUCCESS);
    ok_eq_ustr(&FileName, &TempString);
    ok(PathType.Type == RtlPathTypeDriveAbsolute, "PathType = %d\n", PathType.Type);
    ok(PathType.Unknown == 1234 ||
       broken(PathType.Unknown == 0) /* Win7 */, "Unknown = %d\n", PathType.Unknown);

    /* check the actual functionality with different paths */
    RunTestCases();
}
