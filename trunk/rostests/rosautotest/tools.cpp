/*
 * PROJECT:     ReactOS Automatic Testing Utility
 * LICENSE:     GNU GPLv2 or any later version as published by the Free Software Foundation
 * PURPOSE:     Various helper functions
 * COPYRIGHT:   Copyright 2008-2009 Colin Finck <colin@reactos.org>
 */

#include "precomp.h"

#define DBGPRINT_BUFSIZE   511
static const char HexCharacters[] = "0123456789ABCDEF";

/**
 * Escapes a string according to RFC 1738.
 * Required for passing parameters to the web service.
 *
 * @param Input
 * Constant pointer to a char array, which contains the input buffer to escape.
 *
 * @return
 * The escaped string as std::string.
 */
string
EscapeString(const char* Input)
{
    string ReturnedString;

    do
    {
        if(strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~", *Input))
        {
            /* It's a character we don't need to escape, just add it to the output string */
            ReturnedString += *Input;
        }
        else
        {
            /* We need to escape this character */
            ReturnedString += '%';
            ReturnedString += HexCharacters[((UCHAR)*Input >> 4) % 16];
            ReturnedString += HexCharacters[(UCHAR)*Input % 16];
        }
    }
    while(*++Input);

    return ReturnedString;
}

/**
 * Escapes a string according to RFC 1738.
 * Required for passing parameters to the web service.
 *
 * @param Input
 * Pointer to a std::string, which contains the input buffer to escape.
 *
 * @return
 * The escaped string as std::string.
 */
string
EscapeString(const string& Input)
{
    return EscapeString(Input.c_str());
}

/**
 * Determines whether a string contains entirely numeric values.
 *
 * @param Input
 * Constant pointer to a char array containing the input to check.
 *
 * @return
 * true if the string is entirely numeric, false otherwise.
 */
bool
IsNumber(const char* Input)
{
    do
    {
        if(!isdigit(*Input))
            return false;

        ++Input;
    }
    while(*Input);

    return true;
}

/**
 * Outputs a string through the standard output and the debug output.
 * The string may have LF or CRLF line endings.
 *
 * @param String
 * The std::string to output
 */
string
StringOut(const string& String, bool forcePrint)
{
    char DbgString[DBGPRINT_BUFSIZE + 1];
    size_t i, start = 0, last_newline = 0, size = 0, curr_pos = 0;
    string NewString;

    /* Unify the line endings (the piped output of the tests may use CRLF) */
    for(i = 0; i < String.size(); i++)
    {
        /* If this is a CRLF line-ending, only copy a \n to the new string and skip the next character */
        if(String[i] == '\r' && String[i + 1] == '\n')
        {
            NewString += '\n';
            ++i;
        }
        else
        {
            /* Otherwise copy the string */
            NewString += String[i];
        }

        curr_pos = NewString.size();

        /* Try to print whole lines but obey the 512 bytes chunk size limit*/
        if(NewString[curr_pos - 1] == '\n' || (curr_pos - start) == DBGPRINT_BUFSIZE)
        {
            if((curr_pos - start) >= DBGPRINT_BUFSIZE)
            {
                /* No newlines so far, or the string just fits */
                if(last_newline <= start || ((curr_pos - start == DBGPRINT_BUFSIZE) && NewString[curr_pos - 1] == '\n'))
                {
                    size = curr_pos - start;
                    memcpy(DbgString, NewString.c_str() + start, size);
                    start = curr_pos;
                }
                else
                {
                    size = last_newline - start;
                    memcpy(DbgString, NewString.c_str() + start, size);
                    start = last_newline;
                }

                DbgString[size] = 0;
                OutputDebugStringA(DbgString);
            }

            last_newline = curr_pos;
        }
    }

    size = curr_pos - start;

    /* Only print if forced to or if the rest is a whole line */
    if(forcePrint == true || NewString[curr_pos - 1] == '\n')
    {
        /* Output the whole string */
        cout << NewString;

        memcpy(DbgString, NewString.c_str() + start, size);
        DbgString[size] = 0;
        OutputDebugStringA(DbgString);

        NewString.clear();
        return NewString;
    }

    /* Output full lines only */
    cout << NewString.substr(0, start);

    /* Return the remaining chunk */
    return NewString.substr(start, size);
}

/**
 * Gets a value from a specified INI file and returns it converted to ASCII.
 *
 * @param AppName
 * Constant pointer to a WCHAR array with the INI section to look in (lpAppName parameter passed to GetPrivateProfileStringW)
 *
 * @param KeyName
 * Constant pointer to a WCHAR array containing the key to look for in the specified section (lpKeyName parameter passed to GetPrivateProfileStringW)
 *
 * @param FileName
 * Constant pointer to a WCHAR array containing the path to the INI file
 *
 * @return
 * Returns the data of the value as std::string or an empty string if no data could be retrieved.
 */
string
GetINIValue(PCWCH AppName, PCWCH KeyName, PCWCH FileName)
{
    DWORD Length;
    PCHAR AsciiBuffer;
    string ReturnedString;
    WCHAR Buffer[2048];

    /* Load the value into a temporary Unicode buffer */
    Length = GetPrivateProfileStringW(AppName, KeyName, NULL, Buffer, sizeof(Buffer) / sizeof(WCHAR), FileName);

    if(Length)
    {
        /* Convert the string to ASCII charset */
        AsciiBuffer = new char[Length + 1];
        WideCharToMultiByte(CP_ACP, 0, Buffer, Length + 1, AsciiBuffer, Length + 1, NULL, NULL);

        ReturnedString = AsciiBuffer;
        delete[] AsciiBuffer;
    }

    return ReturnedString;
}

/**
 * Converts an ASCII string to a Unicode one.
 *
 * @param AsciiString
 * Constant pointer to a char array containing the ASCII string
 *
 * @return
 * The Unicode string as std::wstring
 */
wstring
AsciiToUnicode(const char* AsciiString)
{
    DWORD Length;
    PWSTR UnicodeString;
    wstring ReturnString;

    Length = MultiByteToWideChar(CP_ACP, 0, AsciiString, -1, NULL, 0);

    UnicodeString = new WCHAR[Length];
    MultiByteToWideChar(CP_ACP, 0, AsciiString, -1, UnicodeString, Length);
    ReturnString = UnicodeString;
    delete UnicodeString;

    return ReturnString;
}

/**
 * Converts an ASCII string to a Unicode one.
 *
 * @param AsciiString
 * Pointer to a std::string containing the ASCII string
 *
 * @return
 * The Unicode string as std::wstring
 */
wstring
AsciiToUnicode(const string& AsciiString)
{
    return AsciiToUnicode(AsciiString.c_str());
}

/**
 * Converts a Unicode string to an ASCII one.
 *
 * @param UnicodeString
 * Constant pointer to a WCHAR array containing the Unicode string
 *
 * @return
 * The ASCII string as std::string
 */
string
UnicodeToAscii(PCWSTR UnicodeString)
{
    DWORD Length;
    PCHAR AsciiString;
    string ReturnString;

    Length = WideCharToMultiByte(CP_ACP, 0, UnicodeString, -1, NULL, 0, NULL, NULL);

    AsciiString = new char[Length];
    WideCharToMultiByte(CP_ACP, 0, UnicodeString, -1, AsciiString, Length, NULL, NULL);
    ReturnString = AsciiString;
    delete AsciiString;

    return ReturnString;
}

/**
 * Converts a Unicode string to an ASCII one.
 *
 * @param UnicodeString
 * Pointer to a std::wstring containing the Unicode string
 *
 * @return
 * The ASCII string as std::string
 */
string
UnicodeToAscii(const wstring& UnicodeString)
{
    return UnicodeToAscii(UnicodeString.c_str());
}
