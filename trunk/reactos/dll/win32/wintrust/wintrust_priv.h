/*
 * Copyright 2007 Juan Lang
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
#ifndef __WINTRUST_PRIV_H__
#define __WINTRUST_PRIV_H__

#include <config.h>

#include <stdarg.h>
#include <stdio.h>

#define _INC_WINDOWS
#define COM_NO_WINDOWS_H

#define NONAMELESSUNION

#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <winreg.h>
#include <winternl.h>
#include <wintrust.h>
#include <softpub.h>
#include <mscat.h>
#include <mssip.h>

#include <wine/debug.h>
#include <wine/unicode.h>

void * WINAPI WINTRUST_Alloc(DWORD cb) __WINE_ALLOC_SIZE(1) DECLSPEC_HIDDEN;
void WINAPI WINTRUST_Free(void *p) DECLSPEC_HIDDEN;
BOOL WINAPI WINTRUST_AddStore(CRYPT_PROVIDER_DATA *data, HCERTSTORE store) DECLSPEC_HIDDEN;
BOOL WINAPI WINTRUST_AddSgnr(CRYPT_PROVIDER_DATA *data,
 BOOL fCounterSigner, DWORD idxSigner, CRYPT_PROVIDER_SGNR *sgnr) DECLSPEC_HIDDEN;
BOOL WINAPI WINTRUST_AddCert(CRYPT_PROVIDER_DATA *data, DWORD idxSigner,
 BOOL fCounterSigner, DWORD idxCounterSigner, PCCERT_CONTEXT pCert2Add) DECLSPEC_HIDDEN;
BOOL WINAPI WINTRUST_AddPrivData(CRYPT_PROVIDER_DATA *data,
 CRYPT_PROVIDER_PRIVDATA *pPrivData2Add) DECLSPEC_HIDDEN;

#endif /* ndef __WINTRUST_PRIV_H__ */
