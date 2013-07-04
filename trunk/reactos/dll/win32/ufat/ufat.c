/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         FAT File System Management
 * FILE:            reactos/dll/win32/fmifs/init.c
 * PURPOSE:         Initialisation
 *
 * PROGRAMMERS:     Herv� Poussineau (hpoussin@reactos.org)
 */

#include <windef.h>

INT WINAPI
DllMain(
	IN HINSTANCE hinstDLL,
	IN DWORD     dwReason,
	IN LPVOID    lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(dwReason);
	UNREFERENCED_PARAMETER(lpvReserved);

	return TRUE;
}
