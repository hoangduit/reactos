/*
 * Unit tests for code page to/from unicode translations
 *
 * Copyright (c) 2002 Dmitry Timoshkov
 * Copyright (c) 2008 Colin Finck
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

#include <stdarg.h>
#include <limits.h>

#include "wine/test.h"
#include "windef.h"
#include "winbase.h"
#include "winnls.h"

static const WCHAR foobarW[] = {'f','o','o','b','a','r',0};

static void test_destination_buffer(void)
{
    LPSTR   buffer;
    INT     maxsize;
    INT     needed;
    INT     len;

    SetLastError(0xdeadbeef);
    needed = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, NULL, 0, NULL, NULL);
    ok( (needed > 0), "returned %d with %u (expected '> 0')\n",
        needed, GetLastError());

    maxsize = needed*2;
    buffer = HeapAlloc(GetProcessHeap(), 0, maxsize);
    if (buffer == NULL) return;

    maxsize--;
    memset(buffer, 'x', maxsize);
    buffer[maxsize] = '\0';
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buffer, needed+1, NULL, NULL);
    ok( (len > 0), "returned %d with %u and '%s' (expected '> 0')\n",
        len, GetLastError(), buffer);

    memset(buffer, 'x', maxsize);
    buffer[maxsize] = '\0';
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buffer, needed, NULL, NULL);
    ok( (len > 0), "returned %d with %u and '%s' (expected '> 0')\n",
        len, GetLastError(), buffer);

    memset(buffer, 'x', maxsize);
    buffer[maxsize] = '\0';
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buffer, needed-1, NULL, NULL);
    ok( !len && (GetLastError() == ERROR_INSUFFICIENT_BUFFER),
        "returned %d with %u and '%s' (expected '0' with "
        "ERROR_INSUFFICIENT_BUFFER)\n", len, GetLastError(), buffer);

    memset(buffer, 'x', maxsize);
    buffer[maxsize] = '\0';
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buffer, 1, NULL, NULL);
    ok( !len && (GetLastError() == ERROR_INSUFFICIENT_BUFFER),
        "returned %d with %u and '%s' (expected '0' with "
        "ERROR_INSUFFICIENT_BUFFER)\n", len, GetLastError(), buffer);

    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buffer, 0, NULL, NULL);
    ok( (len > 0), "returned %d with %u (expected '> 0')\n",
        len, GetLastError());

    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, NULL, needed, NULL, NULL);
    ok( !len && (GetLastError() == ERROR_INVALID_PARAMETER),
        "returned %d with %u (expected '0' with "
        "ERROR_INVALID_PARAMETER)\n", len, GetLastError());

    HeapFree(GetProcessHeap(), 0, buffer);
}


static void test_null_source(void)
{
    int len;
    DWORD GLE;

    SetLastError(0);
    len = WideCharToMultiByte(CP_ACP, 0, NULL, 0, NULL, 0, NULL, NULL);
    GLE = GetLastError();
    ok(!len && GLE == ERROR_INVALID_PARAMETER,
        "WideCharToMultiByte returned %d with GLE=%u (expected 0 with ERROR_INVALID_PARAMETER)\n",
        len, GLE);

    SetLastError(0);
    len = WideCharToMultiByte(CP_ACP, 0, NULL, -1, NULL, 0, NULL, NULL);
    GLE = GetLastError();
    ok(!len && GLE == ERROR_INVALID_PARAMETER,
        "WideCharToMultiByte returned %d with GLE=%u (expected 0 with ERROR_INVALID_PARAMETER)\n",
        len, GLE);
}

static void test_negative_source_length(void)
{
    int len;
    char buf[10];
    WCHAR bufW[10];

    /* Test, whether any negative source length works as strlen() + 1 */
    SetLastError( 0xdeadbeef );
    memset(buf,'x',sizeof(buf));
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -2002, buf, 10, NULL, NULL);
    ok(len == 7 && GetLastError() == 0xdeadbeef,
       "WideCharToMultiByte(-2002): len=%d error=%u\n", len, GetLastError());
    ok(!lstrcmpA(buf, "foobar"),
       "WideCharToMultiByte(-2002): expected \"foobar\" got \"%s\"\n", buf);

    SetLastError( 0xdeadbeef );
    memset(bufW,'x',sizeof(bufW));
    len = MultiByteToWideChar(CP_ACP, 0, "foobar", -2002, bufW, 10);
    ok(len == 7 && !lstrcmpW(bufW, foobarW) && GetLastError() == 0xdeadbeef,
       "MultiByteToWideChar(-2002): len=%d error=%u\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    memset(bufW, 'x', sizeof(bufW));
    len = MultiByteToWideChar(CP_ACP, 0, "foobar", -1, bufW, 6);
    ok(len == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER,
       "MultiByteToWideChar(-1): len=%d error=%u\n", len, GetLastError());
}

#define LONGBUFLEN 100000
static void test_negative_dest_length(void)
{
    int len, i;
    static char buf[LONGBUFLEN];
    static WCHAR originalW[LONGBUFLEN];
    static char originalA[LONGBUFLEN];
    DWORD theError;

    /* Test return on -1 dest length */
    SetLastError( 0xdeadbeef );
    memset(buf,'x',sizeof(buf));
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buf, -1, NULL, NULL);
    todo_wine {
      ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER,
         "WideCharToMultiByte(destlen -1): len=%d error=%x\n", len, GetLastError());
    }

    /* Test return on -1000 dest length */
    SetLastError( 0xdeadbeef );
    memset(buf,'x',sizeof(buf));
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buf, -1000, NULL, NULL);
    todo_wine {
      ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER,
         "WideCharToMultiByte(destlen -1000): len=%d error=%x\n", len, GetLastError());
    }

    /* Test return on INT_MAX dest length */
    SetLastError( 0xdeadbeef );
    memset(buf,'x',sizeof(buf));
    len = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, buf, INT_MAX, NULL, NULL);
    ok(len == 7 && !lstrcmpA(buf, "foobar") && GetLastError() == 0xdeadbeef,
       "WideCharToMultiByte(destlen INT_MAX): len=%d error=%x\n", len, GetLastError());

    /* Test return on INT_MAX dest length and very long input */
    SetLastError( 0xdeadbeef );
    memset(buf,'x',sizeof(buf));
    for (i=0; i < LONGBUFLEN - 1; i++) {
        originalW[i] = 'Q';
        originalA[i] = 'Q';
    }
    originalW[LONGBUFLEN-1] = 0;
    originalA[LONGBUFLEN-1] = 0;
    len = WideCharToMultiByte(CP_ACP, 0, originalW, -1, buf, INT_MAX, NULL, NULL);
    theError = GetLastError();
    ok(len == LONGBUFLEN && !lstrcmpA(buf, originalA) && theError == 0xdeadbeef,
       "WideCharToMultiByte(srclen %d, destlen INT_MAX): len %d error=%x\n", LONGBUFLEN, len, theError);

}

static void test_other_invalid_parameters(void)
{
    char c_string[] = "Hello World";
    size_t c_string_len = sizeof(c_string);
    WCHAR w_string[] = {'H','e','l','l','o',' ','W','o','r','l','d',0};
    size_t w_string_len = sizeof(w_string) / sizeof(WCHAR);
    BOOL used;
    INT len;

    /* srclen=0 => ERROR_INVALID_PARAMETER */
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, w_string, 0, c_string, c_string_len, NULL, NULL);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    len = MultiByteToWideChar(CP_ACP, 0, c_string, 0, w_string, w_string_len);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());


    /* dst=NULL but dstlen not 0 => ERROR_INVALID_PARAMETER */
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_ACP, 0, w_string, w_string_len, NULL, c_string_len, NULL, NULL);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    len = MultiByteToWideChar(CP_ACP, 0, c_string, c_string_len, NULL, w_string_len);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());


    /* CP_UTF7, CP_UTF8, or CP_SYMBOL and defchar not NULL => ERROR_INVALID_PARAMETER */
    /* CP_SYMBOL's behavior here is undocumented */
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_UTF7, 0, w_string, w_string_len, c_string, c_string_len, c_string, NULL);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_UTF8, 0, w_string, w_string_len, c_string, c_string_len, c_string, NULL);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_SYMBOL, 0, w_string, w_string_len, c_string, c_string_len, c_string, NULL);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());


    /* CP_UTF7, CP_UTF8, or CP_SYMBOL and used not NULL => ERROR_INVALID_PARAMETER */
    /* CP_SYMBOL's behavior here is undocumented */
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_UTF7, 0, w_string, w_string_len, c_string, c_string_len, NULL, &used);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_UTF8, 0, w_string, w_string_len, c_string, c_string_len, NULL, &used);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());

    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_SYMBOL, 0, w_string, w_string_len, c_string, c_string_len, NULL, &used);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());


    /* CP_UTF7, flags not 0 and used not NULL => ERROR_INVALID_PARAMETER */
    /* (tests precedence of ERROR_INVALID_PARAMETER over ERROR_INVALID_FLAGS) */
    /* The same test with CP_SYMBOL instead of CP_UTF7 gives ERROR_INVALID_FLAGS
       instead except on Windows NT4 */
    SetLastError(0xdeadbeef);
    len = WideCharToMultiByte(CP_UTF7, 1, w_string, w_string_len, c_string, c_string_len, NULL, &used);
    ok(len == 0 && GetLastError() == ERROR_INVALID_PARAMETER, "len=%d error=%x\n", len, GetLastError());
}

static void test_overlapped_buffers(void)
{
    static const WCHAR strW[] = {'j','u','s','t',' ','a',' ','t','e','s','t',0};
    static const char strA[] = "just a test";
    char buf[256];
    int ret;

    SetLastError(0xdeadbeef);
    memcpy(buf + 1, strW, sizeof(strW));
    ret = WideCharToMultiByte(CP_ACP, 0, (WCHAR *)(buf + 1), -1, buf, sizeof(buf), NULL, NULL);
    ok(ret == sizeof(strA), "unexpected ret %d\n", ret);
    ok(!memcmp(buf, strA, sizeof(strA)), "conversion failed: %s\n", buf);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());
}

static void test_string_conversion(LPBOOL bUsedDefaultChar)
{
    char mbc;
    char mbs[5];
    int ret;
    WCHAR wc1 = 228;                           /* Western Windows-1252 character */
    WCHAR wc2 = 1088;                          /* Russian Windows-1251 character not displayable for Windows-1252 */
    static const WCHAR wcs[] = {'T', 'h', 1088, 'i', 0}; /* String with ASCII characters and a Russian character */
    static const WCHAR dbwcs[] = {28953, 25152, 0}; /* String with Chinese (codepage 950) characters */

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, &wc1, 1, &mbc, 1, NULL, bUsedDefaultChar);
    ok(ret == 1, "ret is %d\n", ret);
    ok(mbc == '\xe4', "mbc is %d\n", mbc);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, &wc2, 1, &mbc, 1, NULL, bUsedDefaultChar);
    ok(ret == 1, "ret is %d\n", ret);
    ok(mbc == 63, "mbc is %d\n", mbc);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    if (IsValidCodePage(1251))
    {
        SetLastError(0xdeadbeef);
        ret = WideCharToMultiByte(1251, 0, &wc2, 1, &mbc, 1, NULL, bUsedDefaultChar);
        ok(ret == 1, "ret is %d\n", ret);
        ok(mbc == '\xf0', "mbc is %d\n", mbc);
        if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
        ok(GetLastError() == 0xdeadbeef ||
           broken(GetLastError() == 0), /* win95 */
           "GetLastError() is %u\n", GetLastError());

        SetLastError(0xdeadbeef);
        ret = WideCharToMultiByte(1251, 0, &wc1, 1, &mbc, 1, NULL, bUsedDefaultChar);
        ok(ret == 1, "ret is %d\n", ret);
        ok(mbc == 97, "mbc is %d\n", mbc);
        if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
        ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());
    }
    else
        skip("Codepage 1251 not available\n");

    /* This call triggers the last Win32 error */
    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, wcs, -1, &mbc, 1, NULL, bUsedDefaultChar);
    ok(ret == 0, "ret is %d\n", ret);
    ok(mbc == 84, "mbc is %d\n", mbc);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == ERROR_INSUFFICIENT_BUFFER, "GetLastError() is %u\n", GetLastError());

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, wcs, -1, mbs, sizeof(mbs), NULL, bUsedDefaultChar);
    ok(ret == 5, "ret is %d\n", ret);
    ok(!strcmp(mbs, "Th?i"), "mbs is %s\n", mbs);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());
    mbs[0] = 0;

    /* WideCharToMultiByte mustn't add any null character automatically.
       So in this case, we should get the same string again, even if we only copied the first three bytes. */
    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, wcs, 3, mbs, sizeof(mbs), NULL, bUsedDefaultChar);
    ok(ret == 3, "ret is %d\n", ret);
    ok(!strcmp(mbs, "Th?i"), "mbs is %s\n", mbs);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());
    ZeroMemory(mbs, 5);

    /* Now this shouldn't be the case like above as we zeroed the complete string buffer. */
    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, wcs, 3, mbs, sizeof(mbs), NULL, bUsedDefaultChar);
    ok(ret == 3, "ret is %d\n", ret);
    ok(!strcmp(mbs, "Th?"), "mbs is %s\n", mbs);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    /* Double-byte tests */
    ret = WideCharToMultiByte(1252, 0, dbwcs, 3, mbs, sizeof(mbs), NULL, bUsedDefaultChar);
    ok(ret == 3, "ret is %d\n", ret);
    ok(!strcmp(mbs, "??"), "mbs is %s\n", mbs);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);

    /* Length-only tests */
    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, &wc2, 1, NULL, 0, NULL, bUsedDefaultChar);
    ok(ret == 1, "ret is %d\n", ret);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(1252, 0, wcs, -1, NULL, 0, NULL, bUsedDefaultChar);
    ok(ret == 5, "ret is %d\n", ret);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == TRUE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    if (!IsValidCodePage(950))
    {
        skip("Codepage 950 not available\n");
        return;
    }

    /* Double-byte tests */
    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(950, 0, dbwcs, -1, mbs, sizeof(mbs), NULL, bUsedDefaultChar);
    ok(ret == 5, "ret is %d\n", ret);
    ok(!strcmp(mbs, "\xb5H\xa9\xd2"), "mbs is %s\n", mbs);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(950, 0, dbwcs, 1, &mbc, 1, NULL, bUsedDefaultChar);
    ok(ret == 0, "ret is %d\n", ret);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == ERROR_INSUFFICIENT_BUFFER, "GetLastError() is %u\n", GetLastError());
    ZeroMemory(mbs, 5);

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(950, 0, dbwcs, 1, mbs, sizeof(mbs), NULL, bUsedDefaultChar);
    ok(ret == 2, "ret is %d\n", ret);
    ok(!strcmp(mbs, "\xb5H"), "mbs is %s\n", mbs);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    /* Length-only tests */
    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(950, 0, dbwcs, 1, NULL, 0, NULL, bUsedDefaultChar);
    ok(ret == 2, "ret is %d\n", ret);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());

    SetLastError(0xdeadbeef);
    ret = WideCharToMultiByte(950, 0, dbwcs, -1, NULL, 0, NULL, bUsedDefaultChar);
    ok(ret == 5, "ret is %d\n", ret);
    if(bUsedDefaultChar) ok(*bUsedDefaultChar == FALSE, "bUsedDefaultChar is %d\n", *bUsedDefaultChar);
    ok(GetLastError() == 0xdeadbeef, "GetLastError() is %u\n", GetLastError());
}

static void test_undefined_byte_char(void)
{
    static const struct tag_testset {
        INT codepage;
        LPCSTR str;
        BOOL is_error;
    } testset[] = {
        {  874, "\xdd", TRUE },
        {  932, "\xfe", TRUE },
        {  932, "\x80", FALSE },
        {  936, "\xff", TRUE },
        {  949, "\xff", TRUE },
        {  950, "\xff", TRUE },
        { 1252, "\x90", FALSE },
        { 1253, "\xaa", TRUE },
        { 1255, "\xff", TRUE },
        { 1257, "\xa5", TRUE },
    };
    INT i, ret;

    for (i = 0; i < (sizeof(testset) / sizeof(testset[0])); i++) {
        if (! IsValidCodePage(testset[i].codepage))
        {
            skip("Codepage %d not available\n", testset[i].codepage);
            continue;
        }

        SetLastError(0xdeadbeef);
        ret = MultiByteToWideChar(testset[i].codepage, MB_ERR_INVALID_CHARS,
                                  testset[i].str, -1, NULL, 0);
        if (testset[i].is_error) {
            ok(ret == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION,
               "ret is %d, GetLastError is %u (cp %d)\n",
               ret, GetLastError(), testset[i].codepage);
        }
        else {
            ok(ret == strlen(testset[i].str)+1 && GetLastError() == 0xdeadbeef,
               "ret is %d, GetLastError is %u (cp %d)\n",
               ret, GetLastError(), testset[i].codepage);
        }

        SetLastError(0xdeadbeef);
        ret = MultiByteToWideChar(testset[i].codepage, 0,
                                  testset[i].str, -1, NULL, 0);
        ok(ret == strlen(testset[i].str)+1 && GetLastError() == 0xdeadbeef,
           "ret is %d, GetLastError is %u (cp %d)\n",
           ret, GetLastError(), testset[i].codepage);
    }
}

static void test_threadcp(void)
{
    static const LCID ENGLISH  = MAKELCID(MAKELANGID(LANG_ENGLISH,  SUBLANG_ENGLISH_US),         SORT_DEFAULT);
    static const LCID HINDI    = MAKELCID(MAKELANGID(LANG_HINDI,    SUBLANG_HINDI_INDIA),        SORT_DEFAULT);
    static const LCID GEORGIAN = MAKELCID(MAKELANGID(LANG_GEORGIAN, SUBLANG_GEORGIAN_GEORGIA),   SORT_DEFAULT);
    static const LCID RUSSIAN  = MAKELCID(MAKELANGID(LANG_RUSSIAN,  SUBLANG_RUSSIAN_RUSSIA),     SORT_DEFAULT);
    static const LCID JAPANESE = MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN),     SORT_DEFAULT);
    static const LCID CHINESE  = MAKELCID(MAKELANGID(LANG_CHINESE,  SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT);

    BOOL islead, islead_acp;
    CPINFOEXA cpi;
    UINT cp, acp;
    int  i, num;
    LCID last;
    BOOL ret;

    struct test {
        LCID lcid;
        UINT threadcp;
    } lcids[] = {
        { HINDI,    0    },
        { GEORGIAN, 0    },
        { ENGLISH,  1252 },
        { RUSSIAN,  1251 },
        { JAPANESE, 932  },
        { CHINESE,  936  }
    };

    struct test_islead_nocp {
        LCID lcid;
        BYTE testchar;
    } isleads_nocp[] = {
        { HINDI,    0x00 },
        { HINDI,    0x81 },
        { HINDI,    0xa0 },
        { HINDI,    0xe0 },

        { GEORGIAN, 0x00 },
        { GEORGIAN, 0x81 },
        { GEORGIAN, 0xa0 },
        { GEORGIAN, 0xe0 },
    };

    struct test_islead {
        LCID lcid;
        BYTE testchar;
        BOOL islead;
    } isleads[] = {
        { ENGLISH,  0x00, FALSE },
        { ENGLISH,  0x81, FALSE },
        { ENGLISH,  0xa0, FALSE },
        { ENGLISH,  0xe0, FALSE },

        { RUSSIAN,  0x00, FALSE },
        { RUSSIAN,  0x81, FALSE },
        { RUSSIAN,  0xa0, FALSE },
        { RUSSIAN,  0xe0, FALSE },

        { JAPANESE, 0x00, FALSE },
        { JAPANESE, 0x81,  TRUE },
        { JAPANESE, 0xa0, FALSE },
        { JAPANESE, 0xe0,  TRUE },

        { CHINESE,  0x00, FALSE },
        { CHINESE,  0x81,  TRUE },
        { CHINESE,  0xa0,  TRUE },
        { CHINESE,  0xe0,  TRUE },
    };

    last = GetThreadLocale();
    acp  = GetACP();

    for (i = 0; i < sizeof(lcids)/sizeof(lcids[0]); i++)
    {
        SetThreadLocale(lcids[i].lcid);

        cp = 0xdeadbeef;
        GetLocaleInfoA(lcids[i].lcid, LOCALE_IDEFAULTANSICODEPAGE|LOCALE_RETURN_NUMBER, (LPSTR)&cp, sizeof(cp));
        ok(cp == lcids[i].threadcp, "wrong codepage %u for lcid %04x, should be %u\n", cp, lcids[i].threadcp, cp);

        /* GetCPInfoEx/GetCPInfo - CP_ACP */
        SetLastError(0xdeadbeef);
        memset(&cpi, 0, sizeof(cpi));
        ret = GetCPInfoExA(CP_ACP, 0, &cpi);
        ok(ret, "GetCPInfoExA failed for lcid %04x, error %d\n", lcids[i].lcid, GetLastError());
        ok(cpi.CodePage == acp, "wrong codepage %u for lcid %04x, should be %u\n", cpi.CodePage, lcids[i].lcid, acp);

        /* WideCharToMultiByte - CP_ACP */
        num = WideCharToMultiByte(CP_ACP, 0, foobarW, -1, NULL, 0, NULL, NULL);
        ok(num == 7, "ret is %d (%04x)\n", num, lcids[i].lcid);

        /* MultiByteToWideChar - CP_ACP */
        num = MultiByteToWideChar(CP_ACP, 0, "foobar", -1, NULL, 0);
        ok(num == 7, "ret is %d (%04x)\n", num, lcids[i].lcid);

        /* GetCPInfoEx/GetCPInfo - CP_THREAD_ACP */
        SetLastError(0xdeadbeef);
        memset(&cpi, 0, sizeof(cpi));
        ret = GetCPInfoExA(CP_THREAD_ACP, 0, &cpi);
        ok(ret, "GetCPInfoExA failed for lcid %04x, error %d\n", lcids[i].lcid, GetLastError());
        if (lcids[i].threadcp)
            ok(cpi.CodePage == lcids[i].threadcp, "wrong codepage %u for lcid %04x, should be %u\n",
               cpi.CodePage, lcids[i].lcid, lcids[i].threadcp);
        else
            ok(cpi.CodePage == acp, "wrong codepage %u for lcid %04x, should be %u\n",
               cpi.CodePage, lcids[i].lcid, acp);

        /* WideCharToMultiByte - CP_THREAD_ACP */
        num = WideCharToMultiByte(CP_THREAD_ACP, 0, foobarW, -1, NULL, 0, NULL, NULL);
        ok(num == 7, "ret is %d (%04x)\n", num, lcids[i].lcid);

        /* MultiByteToWideChar - CP_THREAD_ACP */
        num = MultiByteToWideChar(CP_THREAD_ACP, 0, "foobar", -1, NULL, 0);
        ok(num == 7, "ret is %d (%04x)\n", num, lcids[i].lcid);
    }

    /* IsDBCSLeadByteEx - locales without codepage */
    for (i = 0; i < sizeof(isleads_nocp)/sizeof(isleads_nocp[0]); i++)
    {
        SetThreadLocale(isleads_nocp[i].lcid);

        islead_acp = IsDBCSLeadByteEx(CP_ACP,        isleads_nocp[i].testchar);
        islead     = IsDBCSLeadByteEx(CP_THREAD_ACP, isleads_nocp[i].testchar);

        ok(islead == islead_acp, "wrong islead %i for test char %x in lcid %04x.  should be %i\n",
            islead, isleads_nocp[i].testchar, isleads_nocp[i].lcid, islead_acp);
    }

    /* IsDBCSLeadByteEx - locales with codepage */
    for (i = 0; i < sizeof(isleads)/sizeof(isleads[0]); i++)
    {
        SetThreadLocale(isleads[i].lcid);

        islead = IsDBCSLeadByteEx(CP_THREAD_ACP, isleads[i].testchar);
        ok(islead == isleads[i].islead, "wrong islead %i for test char %x in lcid %04x.  should be %i\n",
            islead, isleads[i].testchar, isleads[i].lcid, isleads[i].islead);
    }

    SetThreadLocale(last);
}

START_TEST(codepage)
{
    BOOL bUsedDefaultChar;

    test_destination_buffer();
    test_null_source();
    test_negative_source_length();
    test_negative_dest_length();
    test_other_invalid_parameters();
    test_overlapped_buffers();

    /* WideCharToMultiByte has two code paths, test both here */
    test_string_conversion(NULL);
    test_string_conversion(&bUsedDefaultChar);

    test_undefined_byte_char();
    test_threadcp();
}
