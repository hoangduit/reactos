#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <shellapi.h>
#include <tchar.h>

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
  static const WCHAR szROS[] = { 'R','e','a','c','t','O','S',0 };
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(hInstance);
  ShellAbout(0, szROS, 0, 0);
  return 1;
}
