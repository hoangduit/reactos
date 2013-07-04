/*
 *  FreeLoader
 *
 *  Copyright (C) 2009       Aleksey Bragin  <aleksey@reactos.org>
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

#include <freeldr.h>

#include <ndk/ldrtypes.h>
#include <arc/setupblk.h>

#include <debug.h>

DBG_DEFAULT_CHANNEL(WINDOWS);

void
WinLdrSetupMachineDependent(PLOADER_PARAMETER_BLOCK LoaderBlock);

VOID
WinLdrSetProcessorContext(void);

// TODO: Move to .h
VOID AllocateAndInitLPB(PLOADER_PARAMETER_BLOCK *OutLoaderBlock);

static VOID
SetupLdrLoadNlsData(PLOADER_PARAMETER_BLOCK LoaderBlock, HINF InfHandle, LPCSTR SearchPath)
{
    INFCONTEXT InfContext;
    BOOLEAN Status;
    LPCSTR AnsiName, OemName, LangName;

    /* Get ANSI codepage file */
    if (!InfFindFirstLine(InfHandle, "NLS", "AnsiCodepage", &InfContext))
    {
        ERR("Failed to find 'NLS/AnsiCodepage'\n");
        return;
    }
    if (!InfGetDataField(&InfContext, 1, &AnsiName))
    {
        ERR("Failed to get load options\n");
        return;
    }

    /* Get OEM codepage file */
    if (!InfFindFirstLine(InfHandle, "NLS", "OemCodepage", &InfContext))
    {
        ERR("Failed to find 'NLS/AnsiCodepage'\n");
        return;
    }
    if (!InfGetDataField(&InfContext, 1, &OemName))
    {
        ERR("Failed to get load options\n");
        return;
    }

    if (!InfFindFirstLine(InfHandle, "NLS", "UnicodeCasetable", &InfContext))
    {
        ERR("Failed to find 'NLS/AnsiCodepage'\n");
        return;
    }
    if (!InfGetDataField(&InfContext, 1, &LangName))
    {
        ERR("Failed to get load options\n");
        return;
    }

    Status = WinLdrLoadNLSData(LoaderBlock, SearchPath, AnsiName, OemName, LangName);
    TRACE("NLS data loaded with status %d\n", Status);
}

static VOID
SetupLdrScanBootDrivers(PLIST_ENTRY BootDriverListHead, HINF InfHandle, LPCSTR SearchPath)
{
    INFCONTEXT InfContext, dirContext;
    BOOLEAN Status;
    LPCSTR Media, DriverName, dirIndex, ImagePath;
    WCHAR ServiceName[256];
    WCHAR ImagePathW[256];

    /* Open inf section */
    if (!InfFindFirstLine(InfHandle, "SourceDisksFiles", NULL, &InfContext))
        return;

    /* Load all listed boot drivers */
    do
    {
        if (InfGetDataField(&InfContext, 7, &Media) &&
            InfGetDataField(&InfContext, 0, &DriverName) &&
            InfGetDataField(&InfContext, 13, &dirIndex))
        {
            if ((strcmp(Media, "x") == 0) &&
                InfFindFirstLine(InfHandle, "Directories", dirIndex, &dirContext) &&
                InfGetDataField(&dirContext, 1, &ImagePath))
            {
                /* Convert name to widechar */
                swprintf(ServiceName, L"%S", DriverName);

                /* Prepare image path */
                swprintf(ImagePathW, L"%S", ImagePath);
                wcscat(ImagePathW, L"\\");
                wcscat(ImagePathW, ServiceName);

                /* Remove .sys extension */
                ServiceName[wcslen(ServiceName) - 4] = 0;

                /* Add it to the list */
                Status = WinLdrAddDriverToList(BootDriverListHead,
                    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\",
                    ImagePathW,
                    ServiceName);

                if (!Status)
                {
                    ERR("could not add boot driver %s, %s\n", SearchPath, DriverName);
                    return;
                }
            }
        }
    } while (InfFindNextLine(&InfContext, &InfContext));
}

VOID
LoadReactOSSetup(IN OperatingSystemItem* OperatingSystem,
                 IN USHORT OperatingSystemVersion)
{
    CHAR FileName[512];
    CHAR BootPath[512];
    LPCSTR LoadOptions, BootOptions;
    BOOLEAN BootFromFloppy;
    ULONG i, ErrorLine;
    HINF InfHandle;
    INFCONTEXT InfContext;
    PLOADER_PARAMETER_BLOCK LoaderBlock;
    PSETUP_LOADER_BLOCK SetupBlock;
    LPCSTR SystemPath;
    LPCSTR SourcePaths[] =
    {
        "\\", /* Only for floppy boot */
#if defined(_M_IX86)
        "\\I386\\",
#elif defined(_M_MPPC)
        "\\PPC\\",
#elif defined(_M_MRX000)
        "\\MIPS\\",
#endif
        "\\reactos\\",
        NULL
    };

    /* Get boot path */
    MachDiskGetBootPath(BootPath, sizeof(BootPath));

    /* And check if we booted from floppy */
    BootFromFloppy = strstr(BootPath, "fdisk") != NULL;

    /* Open 'txtsetup.sif' from any of source paths */
    for (i = BootFromFloppy ? 0 : 1; ; i++)
    {
        SystemPath = SourcePaths[i];
        if (!SystemPath)
        {
            ERR("Failed to open txtsetup.sif\n");
            return;
        }
        sprintf(FileName, "%stxtsetup.sif", SystemPath);
        if (InfOpenFile (&InfHandle, FileName, &ErrorLine))
        {
            strcat(BootPath, SystemPath);
            break;
        }
    }

    TRACE("BootPath: '%s', SystemPath: '%s'\n", BootPath, SystemPath);

    /* Get Load options - debug and non-debug */
    if (!InfFindFirstLine(InfHandle, "SetupData", "OsLoadOptions", &InfContext))
    {
        ERR("Failed to find 'SetupData/OsLoadOptions'\n");
        return;
    }

    if (!InfGetDataField(&InfContext, 1, &LoadOptions))
    {
        ERR("Failed to get load options\n");
        return;
    }

    BootOptions = LoadOptions;

#if DBG
    /* Get debug load options and use them */
    if (InfFindFirstLine(InfHandle, "SetupData", "DbgOsLoadOptions", &InfContext))
    {
        if (InfGetDataField(&InfContext, 1, &LoadOptions))
            BootOptions = LoadOptions;
    }
#endif

    TRACE("BootOptions: '%s'\n", BootOptions);

    UiDrawStatusText("Setup is loading...");

    /* Allocate and minimalistic-initialize LPB */
    AllocateAndInitLPB(&LoaderBlock);

    /* Allocate and initialize setup loader block */
    SetupBlock = &WinLdrSystemBlock->SetupBlock;
    LoaderBlock->SetupLdrBlock = SetupBlock;

    /* Set textmode setup flag */
    SetupBlock->Flags = SETUPLDR_TEXT_MODE;

    /* Load NLS data, they are in system32 */
    strcpy(FileName, BootPath);
    strcat(FileName, "system32\\");
    SetupLdrLoadNlsData(LoaderBlock, InfHandle, FileName);

    /* Get a list of boot drivers */
    SetupLdrScanBootDrivers(&LoaderBlock->BootDriverListHead, InfHandle, BootPath);

    /* Load ReactOS */
    LoadAndBootWindowsCommon(_WIN32_WINNT_WS03,
                             LoaderBlock,
                             BootOptions,
                             BootPath,
                             TRUE);
}
