/*
 *  ReactOS kernel
 *  Copyright (C) 2003 ReactOS Team
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS text-mode setup
 * FILE:            subsys/system/usetup/registry.h
 * PURPOSE:         Registry creation functions
 * PROGRAMMER:      Eric Kohl
 */

#pragma once

BOOLEAN
ImportRegistryFile(PWSTR Filename,
		   PWSTR Section,
		   LCID LocaleId,
		   BOOLEAN Delete);

BOOLEAN
SetInstallPathValue(PUNICODE_STRING InstallPath);

BOOLEAN
SetMountedDeviceValue(CHAR Letter, ULONG Signature, LARGE_INTEGER StartingOffset);

VOID
SetDefaultPagefile(WCHAR Drive);

/* EOF */
