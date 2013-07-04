/* General includes */
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

#define WIN32_NO_STATUS
#define _INC_WINDOWS
#define COM_NO_WINDOWS_H
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <reason.h>
#include <shlobj.h>
#include <wininet.h>

#include <ndk/rtlfuncs.h>
#include <reactos/buildno.h>

/* Class includes */
#include "auto_array_ptr.h"
#include "CConfiguration.h"
#include "CFatalException.h"
#include "CInvalidParameterException.h"
#include "CProcess.h"
#include "CSimpleException.h"
#include "CTestInfo.h"
#include "CTest.h"
#include "CTestList.h"
#include "CJournaledTestList.h"
#include "CVirtualTestList.h"
#include "CWebService.h"
#include "CWineTest.h"

/* Useful macros */
#define EXCEPTION(Message)   throw CSimpleException(Message)
#define FATAL(Message)       throw CFatalException(__FILE__, __LINE__, Message)
#define SSEXCEPTION          throw CSimpleException(ss.str().c_str())

/* main.c */
extern CConfiguration Configuration;

/* shutdown.c */
bool ShutdownSystem();

/* tools.c */
wstring AsciiToUnicode(const char* AsciiString);
wstring AsciiToUnicode(const string& AsciiString);
string EscapeString(const char* Input);
string EscapeString(const string& Input);
string GetINIValue(PCWCH AppName, PCWCH KeyName, PCWCH FileName);
bool IsNumber(const char* Input);
string StringOut(const string& String, bool forcePrint = true);
string UnicodeToAscii(PCWSTR UnicodeString);
string UnicodeToAscii(const wstring& UnicodeString);
