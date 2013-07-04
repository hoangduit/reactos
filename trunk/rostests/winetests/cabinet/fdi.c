/*
 * Unit tests for the File Decompression Interface
 *
 * Copyright (C) 2006 James Hawkins
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

//#include <stdio.h>
//#include <windows.h>
#include <wine/test.h>
#include <fci.h>
#include <fdi.h>

/* make the max size large so there is only one cab file */
#define MEDIA_SIZE          999999999
#define FOLDER_THRESHOLD    900000

static CHAR CURR_DIR[MAX_PATH];

/* FDI callbacks */

static void * CDECL fdi_alloc(ULONG cb)
{
    return HeapAlloc(GetProcessHeap(), 0, cb);
}

static void * CDECL fdi_alloc_bad(ULONG cb)
{
    return NULL;
}

static void CDECL fdi_free(void *pv)
{
    HeapFree(GetProcessHeap(), 0, pv);
}

static INT_PTR CDECL fdi_open(char *pszFile, int oflag, int pmode)
{
    HANDLE handle;
    handle = CreateFileA(pszFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                          OPEN_EXISTING, 0, NULL );
    if (handle == INVALID_HANDLE_VALUE)
        return 0;
    return (INT_PTR) handle;
}

static UINT CDECL fdi_read(INT_PTR hf, void *pv, UINT cb)
{
    HANDLE handle = (HANDLE) hf;
    DWORD dwRead;
    if (ReadFile(handle, pv, cb, &dwRead, NULL))
        return dwRead;
    return 0;
}

static UINT CDECL fdi_write(INT_PTR hf, void *pv, UINT cb)
{
    HANDLE handle = (HANDLE) hf;
    DWORD dwWritten;
    if (WriteFile(handle, pv, cb, &dwWritten, NULL))
        return dwWritten;
    return 0;
}

static int CDECL fdi_close(INT_PTR hf)
{
    HANDLE handle = (HANDLE) hf;
    return CloseHandle(handle) ? 0 : -1;
}

static LONG CDECL fdi_seek(INT_PTR hf, LONG dist, int seektype)
{
    HANDLE handle = (HANDLE) hf;
    return SetFilePointer(handle, dist, NULL, seektype);
}

static void test_FDICreate(void)
{
    HFDI hfdi;
    ERF erf;

    /* native crashes if pfnalloc is NULL */

    /* FDICreate does not crash with a NULL pfnfree,
     * but FDIDestroy will crash when it tries to access it.
     */
    if (0)
    {
        SetLastError(0xdeadbeef);
        erf.erfOper = 0x1abe11ed;
        erf.erfType = 0x5eed1e55;
        erf.fError = 0x1ead1e55;
        hfdi = FDICreate(fdi_alloc, NULL, fdi_open, fdi_read,
                         fdi_write, fdi_close, fdi_seek,
                         cpuUNKNOWN, &erf);
        ok(hfdi != NULL, "Expected non-NULL context\n");
        ok(GetLastError() == 0xdeadbeef,
           "Expected 0xdeadbeef, got %d\n", GetLastError());
        ok(erf.erfOper == 0x1abe11ed, "Expected 0x1abe11ed, got %d\n", erf.erfOper);
        ok(erf.erfType == 0x5eed1e55, "Expected 0x5eed1e55, got %d\n", erf.erfType);
        ok(erf.fError == 0x1ead1e55, "Expected 0x1ead1e55, got %d\n", erf.fError);

        FDIDestroy(hfdi);
    }

    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, NULL, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok((erf.erfOper == 0x1abe11ed || erf.erfOper == 0 /* Vista */), "Expected 0x1abe11ed or 0, got %d\n", erf.erfOper);
    ok((erf.erfType == 0x5eed1e55 || erf.erfType == 0 /* Vista */), "Expected 0x5eed1e55 or 0, got %d\n", erf.erfType);
    ok((erf.fError == 0x1ead1e55 || erf.fError == 0 /* Vista */), "Expected 0x1ead1e55 or 0, got %d\n", erf.fError);

    FDIDestroy(hfdi);

    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, NULL,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok((erf.erfOper == 0x1abe11ed || erf.erfOper == 0 /* Vista */), "Expected 0x1abe11ed or 0, got %d\n", erf.erfOper);
    ok((erf.erfType == 0x5eed1e55 || erf.erfType == 0 /* Vista */), "Expected 0x5eed1e55 or 0, got %d\n", erf.erfType);
    ok((erf.fError == 0x1ead1e55 || erf.fError == 0 /* Vista */), "Expected 0x1ead1e55 or 0, got %d\n", erf.fError);

    FDIDestroy(hfdi);

    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     NULL, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok((erf.erfOper == 0x1abe11ed || erf.erfOper == 0 /* Vista */), "Expected 0x1abe11ed or 0, got %d\n", erf.erfOper);
    ok((erf.erfType == 0x5eed1e55 || erf.erfType == 0 /* Vista */), "Expected 0x5eed1e55 or 0, got %d\n", erf.erfType);
    ok((erf.fError == 0x1ead1e55 || erf.fError == 0 /* Vista */), "Expected 0x1ead1e55 or 0, got %d\n", erf.fError);

    FDIDestroy(hfdi);

    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, NULL, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok((erf.erfOper == 0x1abe11ed || erf.erfOper == 0 /* Vista */), "Expected 0x1abe11ed or 0, got %d\n", erf.erfOper);
    ok((erf.erfType == 0x5eed1e55 || erf.erfType == 0 /* Vista */), "Expected 0x5eed1e55 or 0, got %d\n", erf.erfType);
    ok((erf.fError == 0x1ead1e55 || erf.fError == 0 /* Vista */), "Expected 0x1ead1e55 or 0, got %d\n", erf.fError);

    FDIDestroy(hfdi);

    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, NULL,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok((erf.erfOper == 0x1abe11ed || erf.erfOper == 0 /* Vista */), "Expected 0x1abe11ed or 0, got %d\n", erf.erfOper);
    ok((erf.erfType == 0x5eed1e55 || erf.erfType == 0 /* Vista */), "Expected 0x5eed1e55 or 0, got %d\n", erf.erfType);
    ok((erf.fError == 0x1ead1e55 || erf.fError == 0 /* Vista */), "Expected 0x1ead1e55 or 0, got %d\n", erf.fError);

    FDIDestroy(hfdi);

    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, NULL);
    /* XP sets hfdi to a non-NULL value, but Vista sets it to NULL! */
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    /* NULL is passed to FDICreate instead of &erf, so don't retest the erf member values. */

    FDIDestroy(hfdi);

    /* bad cpu type */
    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     0xcafebabe, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok((erf.erfOper == 0x1abe11ed || erf.erfOper == 0 /* Vista */), "Expected 0x1abe11ed or 0, got %d\n", erf.erfOper);
    ok((erf.erfType == 0x5eed1e55 || erf.erfType == 0 /* Vista */), "Expected 0x5eed1e55 or 0, got %d\n", erf.erfType);
    ok((erf.fError == 0x1ead1e55 || erf.fError == 0 /* Vista */), "Expected 0x1ead1e55 or 0, got %d\n", erf.fError);

    FDIDestroy(hfdi);

    /* pfnalloc fails */
    SetLastError(0xdeadbeef);
    erf.erfOper = 0x1abe11ed;
    erf.erfType = 0x5eed1e55;
    erf.fError = 0x1ead1e55;
    hfdi = FDICreate(fdi_alloc_bad, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi == NULL, "Expected NULL context, got %p\n", hfdi);
    ok(erf.erfOper == FDIERROR_ALLOC_FAIL,
       "Expected FDIERROR_ALLOC_FAIL, got %d\n", erf.erfOper);
    ok(erf.fError == TRUE, "Expected TRUE, got %d\n", erf.fError);
    ok(GetLastError() == 0xdeadbeef,
       "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok(erf.erfType == 0, "Expected 0, got %d\n", erf.erfType);
}

static void test_FDIDestroy(void)
{
    HFDI hfdi;
    ERF erf;
    BOOL ret;

    /* native crashes if hfdi is NULL or invalid */

    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");

    /* successfully destroy hfdi */
    ret = FDIDestroy(hfdi);
    ok(ret == TRUE, "Expected TRUE, got %d\n", ret);

    /* native crashes if you try to destroy hfdi twice */
    if (0)
    {
        /* try to destroy hfdi again */
        ret = FDIDestroy(hfdi);
        ok(ret == TRUE, "Expected TRUE, got %d\n", ret);
    }
}

static void createTestFile(const CHAR *name)
{
    HANDLE file;
    DWORD written;

    file = CreateFileA(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    ok(file != INVALID_HANDLE_VALUE, "Failure to open file %s\n", name);
    WriteFile(file, name, strlen(name), &written, NULL);
    WriteFile(file, "\n", strlen("\n"), &written, NULL);
    CloseHandle(file);
}

static void create_test_files(void)
{
    DWORD len;

    len = GetCurrentDirectoryA(MAX_PATH, CURR_DIR);

    if(len && (CURR_DIR[len-1] == '\\'))
        CURR_DIR[len-1] = 0;

    createTestFile("a.txt");
    createTestFile("b.txt");
    CreateDirectoryA("testdir", NULL);
    createTestFile("testdir\\c.txt");
    createTestFile("testdir\\d.txt");
}

static void delete_test_files(void)
{
    DeleteFileA("a.txt");
    DeleteFileA("b.txt");
    DeleteFileA("testdir\\c.txt");
    DeleteFileA("testdir\\d.txt");
    RemoveDirectoryA("testdir");

    DeleteFileA("extract.cab");
}

/* FCI callbacks */

static void * CDECL mem_alloc(ULONG cb)
{
    return HeapAlloc(GetProcessHeap(), 0, cb);
}

static void CDECL mem_free(void *memory)
{
    HeapFree(GetProcessHeap(), 0, memory);
}

static BOOL CDECL get_next_cabinet(PCCAB pccab, ULONG  cbPrevCab, void *pv)
{
    return TRUE;
}

static LONG CDECL progress(UINT typeStatus, ULONG cb1, ULONG cb2, void *pv)
{
    return 0;
}

static int CDECL file_placed(PCCAB pccab, char *pszFile, LONG cbFile,
                             BOOL fContinuation, void *pv)
{
    return 0;
}

static INT_PTR CDECL fci_open(char *pszFile, int oflag, int pmode, int *err, void *pv)
{
    HANDLE handle;
    DWORD dwAccess = 0;
    DWORD dwShareMode = 0;
    DWORD dwCreateDisposition = OPEN_EXISTING;

    dwAccess = GENERIC_READ | GENERIC_WRITE;
    /* FILE_SHARE_DELETE is not supported by Windows Me/98/95 */
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

    if (GetFileAttributesA(pszFile) != INVALID_FILE_ATTRIBUTES)
        dwCreateDisposition = OPEN_EXISTING;
    else
        dwCreateDisposition = CREATE_NEW;

    handle = CreateFileA(pszFile, dwAccess, dwShareMode, NULL,
                         dwCreateDisposition, 0, NULL);

    ok(handle != INVALID_HANDLE_VALUE, "Failed to CreateFile %s\n", pszFile);

    return (INT_PTR)handle;
}

static UINT CDECL fci_read(INT_PTR hf, void *memory, UINT cb, int *err, void *pv)
{
    HANDLE handle = (HANDLE)hf;
    DWORD dwRead;
    BOOL res;

    res = ReadFile(handle, memory, cb, &dwRead, NULL);
    ok(res, "Failed to ReadFile\n");

    return dwRead;
}

static UINT CDECL fci_write(INT_PTR hf, void *memory, UINT cb, int *err, void *pv)
{
    HANDLE handle = (HANDLE)hf;
    DWORD dwWritten;
    BOOL res;

    res = WriteFile(handle, memory, cb, &dwWritten, NULL);
    ok(res, "Failed to WriteFile\n");

    return dwWritten;
}

static int CDECL fci_close(INT_PTR hf, int *err, void *pv)
{
    HANDLE handle = (HANDLE)hf;
    ok(CloseHandle(handle), "Failed to CloseHandle\n");

    return 0;
}

static LONG CDECL fci_seek(INT_PTR hf, LONG dist, int seektype, int *err, void *pv)
{
    HANDLE handle = (HANDLE)hf;
    DWORD ret;

    ret = SetFilePointer(handle, dist, NULL, seektype);
    ok(ret != INVALID_SET_FILE_POINTER, "Failed to SetFilePointer\n");

    return ret;
}

static int CDECL fci_delete(char *pszFile, int *err, void *pv)
{
    BOOL ret = DeleteFileA(pszFile);
    ok(ret, "Failed to DeleteFile %s\n", pszFile);

    return 0;
}

static BOOL CDECL get_temp_file(char *pszTempName, int cbTempName, void *pv)
{
    LPSTR tempname;

    tempname = HeapAlloc(GetProcessHeap(), 0, MAX_PATH);
    GetTempFileNameA(".", "xx", 0, tempname);

    if (tempname && (strlen(tempname) < (unsigned)cbTempName))
    {
        lstrcpyA(pszTempName, tempname);
        HeapFree(GetProcessHeap(), 0, tempname);
        return TRUE;
    }

    HeapFree(GetProcessHeap(), 0, tempname);

    return FALSE;
}

static INT_PTR CDECL get_open_info(char *pszName, USHORT *pdate, USHORT *ptime,
                                   USHORT *pattribs, int *err, void *pv)
{
    BY_HANDLE_FILE_INFORMATION finfo;
    FILETIME filetime;
    HANDLE handle;
    DWORD attrs;
    BOOL res;

    handle = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    ok(handle != INVALID_HANDLE_VALUE, "Failed to CreateFile %s\n", pszName);

    res = GetFileInformationByHandle(handle, &finfo);
    ok(res, "Expected GetFileInformationByHandle to succeed\n");

    FileTimeToLocalFileTime(&finfo.ftLastWriteTime, &filetime);
    FileTimeToDosDateTime(&filetime, pdate, ptime);

    attrs = GetFileAttributes(pszName);
    ok(attrs != INVALID_FILE_ATTRIBUTES, "Failed to GetFileAttributes\n");
    /* fixme: should convert attrs to *pattribs, make sure
     * have a test that catches the fact that we don't?
     */

    return (INT_PTR)handle;
}

static void add_file(HFCI hfci, char *file)
{
    char path[MAX_PATH];
    BOOL res;

    lstrcpyA(path, CURR_DIR);
    lstrcatA(path, "\\");
    lstrcatA(path, file);

    res = FCIAddFile(hfci, path, file, FALSE, get_next_cabinet, progress,
                     get_open_info, tcompTYPE_MSZIP);
    ok(res, "Expected FCIAddFile to succeed\n");
}

static void set_cab_parameters(PCCAB pCabParams)
{
    ZeroMemory(pCabParams, sizeof(CCAB));

    pCabParams->cb = MEDIA_SIZE;
    pCabParams->cbFolderThresh = FOLDER_THRESHOLD;
    pCabParams->setID = 0xbeef;
    lstrcpyA(pCabParams->szCabPath, CURR_DIR);
    lstrcatA(pCabParams->szCabPath, "\\");
    lstrcpyA(pCabParams->szCab, "extract.cab");
}

static void create_cab_file(void)
{
    CCAB cabParams;
    HFCI hfci;
    ERF erf;
    static CHAR a_txt[]         = "a.txt",
                b_txt[]         = "b.txt",
                testdir_c_txt[] = "testdir\\c.txt",
                testdir_d_txt[] = "testdir\\d.txt";
    BOOL res;

    set_cab_parameters(&cabParams);

    hfci = FCICreate(&erf, file_placed, mem_alloc, mem_free, fci_open,
                      fci_read, fci_write, fci_close, fci_seek, fci_delete,
                      get_temp_file, &cabParams, NULL);

    ok(hfci != NULL, "Failed to create an FCI context\n");

    add_file(hfci, a_txt);
    add_file(hfci, b_txt);
    add_file(hfci, testdir_c_txt);
    add_file(hfci, testdir_d_txt);

    res = FCIFlushCabinet(hfci, FALSE, get_next_cabinet, progress);
    ok(res, "Failed to flush the cabinet\n");

    res = FCIDestroy(hfci);
    ok(res, "Failed to destroy the cabinet\n");
}

static void test_FDIIsCabinet(void)
{
    ERF erf;
    BOOL ret;
    HFDI hfdi;
    INT_PTR fd;
    FDICABINETINFO cabinfo;
    char temp[] = "temp.txt";
    char extract[] = "extract.cab";

    create_test_files();
    create_cab_file();

    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);
    ok(hfdi != NULL, "Expected non-NULL context\n");

    /* native crashes if hfdi or cabinfo are NULL or invalid */

    /* invalid file handle */
    ZeroMemory(&cabinfo, sizeof(FDICABINETINFO));
    SetLastError(0xdeadbeef);
    ret = FDIIsCabinet(hfdi, -1, &cabinfo);
    ok(ret == FALSE, "Expected FALSE, got %d\n", ret);
    ok(GetLastError() == ERROR_INVALID_HANDLE,
       "Expected ERROR_INVALID_HANDLE, got %d\n", GetLastError());
    ok(cabinfo.cbCabinet == 0, "Expected 0, got %d\n", cabinfo.cbCabinet);
    ok(cabinfo.cFiles == 0, "Expected 0, got %d\n", cabinfo.cFiles);
    ok(cabinfo.cFolders == 0, "Expected 0, got %d\n", cabinfo.cFolders);
    ok(cabinfo.iCabinet == 0, "Expected 0, got %d\n", cabinfo.iCabinet);
    ok(cabinfo.setID == 0, "Expected 0, got %d\n", cabinfo.setID);

    createTestFile("temp.txt");
    fd = fdi_open(temp, 0, 0);

    /* file handle doesn't point to a cabinet */
    ZeroMemory(&cabinfo, sizeof(FDICABINETINFO));
    SetLastError(0xdeadbeef);
    ret = FDIIsCabinet(hfdi, fd, &cabinfo);
    ok(ret == FALSE, "Expected FALSE, got %d\n", ret);
    ok(GetLastError() == 0xdeadbeef, "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok(cabinfo.cbCabinet == 0, "Expected 0, got %d\n", cabinfo.cbCabinet);
    ok(cabinfo.cFiles == 0, "Expected 0, got %d\n", cabinfo.cFiles);
    ok(cabinfo.cFolders == 0, "Expected 0, got %d\n", cabinfo.cFolders);
    ok(cabinfo.iCabinet == 0, "Expected 0, got %d\n", cabinfo.iCabinet);
    ok(cabinfo.setID == 0, "Expected 0, got %d\n", cabinfo.setID);

    fdi_close(fd);
    DeleteFileA("temp.txt");

    /* try a real cab */
    fd = fdi_open(extract, 0, 0);
    ZeroMemory(&cabinfo, sizeof(FDICABINETINFO));
    SetLastError(0xdeadbeef);
    ret = FDIIsCabinet(hfdi, fd, &cabinfo);
    ok(ret == TRUE, "Expected TRUE, got %d\n", ret);
    ok(GetLastError() == 0xdeadbeef, "Expected 0xdeadbeef, got %d\n", GetLastError());
    ok(cabinfo.cFiles == 4, "Expected 4, got %d\n", cabinfo.cFiles);
    ok(cabinfo.cFolders == 1, "Expected 1, got %d\n", cabinfo.cFolders);
    ok(cabinfo.setID == 0xbeef, "Expected 0xbeef, got %d\n", cabinfo.setID);
    ok(cabinfo.cbCabinet == 182, "Expected 182, got %d\n", cabinfo.cbCabinet);
    ok(cabinfo.iCabinet == 0, "Expected 0, got %d\n", cabinfo.iCabinet);

    fdi_close(fd);
    FDIDestroy(hfdi);
    delete_test_files();
}


static INT_PTR __cdecl CopyProgress(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
    return 0;
}

static void test_FDICopy(void)
{
    CCAB cabParams;
    HFDI hfdi;
    HFCI hfci;
    ERF erf;
    BOOL ret;
    char name[] = "extract.cab";
    char path[MAX_PATH + 1];

    set_cab_parameters(&cabParams);

    hfci = FCICreate(&erf, file_placed, mem_alloc, mem_free, fci_open,
                     fci_read, fci_write, fci_close, fci_seek,
                     fci_delete, get_temp_file, &cabParams, NULL);

    ret = FCIFlushCabinet(hfci, FALSE, get_next_cabinet, progress);
    ok(ret, "Failed to flush the cabinet\n");

    FCIDestroy(hfci);

    lstrcpyA(path, CURR_DIR);

    /* path doesn't have a trailing backslash */
    if (lstrlenA(path) > 2)
    {
        hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                         fdi_write, fdi_close, fdi_seek,
                         cpuUNKNOWN, &erf);

        SetLastError(0xdeadbeef);
        ret = FDICopy(hfdi, name, path, 0, CopyProgress, NULL, 0);
        ok(ret == FALSE, "Expected FALSE, got %d\n", ret);
        ok(GetLastError() == ERROR_INVALID_HANDLE,
           "Expected ERROR_INVALID_HANDLE, got %d\n", GetLastError());

        FDIDestroy(hfdi);
    }
    else
        skip("Running on a root drive directory.\n");

    lstrcatA(path, "\\");

    hfdi = FDICreate(fdi_alloc, fdi_free, fdi_open, fdi_read,
                     fdi_write, fdi_close, fdi_seek,
                     cpuUNKNOWN, &erf);

    /* cabinet with no files or folders */
    SetLastError(0xdeadbeef);
    ret = FDICopy(hfdi, name, path, 0, CopyProgress, NULL, 0);
    ok(ret == TRUE, "Expected TRUE, got %d\n", ret);
    ok(GetLastError() == 0, "Expected 0f, got %d\n", GetLastError());

    FDIDestroy(hfdi);

    DeleteFileA(name);
}


START_TEST(fdi)
{
    test_FDICreate();
    test_FDIDestroy();
    test_FDIIsCabinet();
    test_FDICopy();
}
