/*
 *  FreeLoader
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

VOID
XboxMachInit(const char *CmdLine)
{
  /* Set LEDs to red before anything is initialized */
  XboxSetLED("rrrr");

  /* Initialize our stuff */
  XboxMemInit();
  XboxVideoInit();

  /* Setup vtbl */
  MachVtbl.ConsPutChar = XboxConsPutChar;
  MachVtbl.ConsKbHit = XboxConsKbHit;
  MachVtbl.ConsGetCh = XboxConsGetCh;
  MachVtbl.VideoClearScreen = XboxVideoClearScreen;
  MachVtbl.VideoSetDisplayMode = XboxVideoSetDisplayMode;
  MachVtbl.VideoGetDisplaySize = XboxVideoGetDisplaySize;
  MachVtbl.VideoGetBufferSize = XboxVideoGetBufferSize;
  MachVtbl.VideoHideShowTextCursor = XboxVideoHideShowTextCursor;
  MachVtbl.VideoPutChar = XboxVideoPutChar;
  MachVtbl.VideoCopyOffScreenBufferToVRAM = XboxVideoCopyOffScreenBufferToVRAM;
  MachVtbl.VideoIsPaletteFixed = XboxVideoIsPaletteFixed;
  MachVtbl.VideoSetPaletteColor = XboxVideoSetPaletteColor;
  MachVtbl.VideoGetPaletteColor = XboxVideoGetPaletteColor;
  MachVtbl.VideoSync = XboxVideoSync;
  MachVtbl.Beep = PcBeep;
  MachVtbl.PrepareForReactOS = XboxPrepareForReactOS;
  MachVtbl.GetMemoryMap = XboxMemGetMemoryMap;
  MachVtbl.DiskGetBootPath = DiskGetBootPath;
  MachVtbl.DiskReadLogicalSectors = XboxDiskReadLogicalSectors;
  MachVtbl.DiskGetDriveGeometry = XboxDiskGetDriveGeometry;
  MachVtbl.DiskGetCacheableBlockCount = XboxDiskGetCacheableBlockCount;
  MachVtbl.GetTime = XboxGetTime;
  MachVtbl.InitializeBootDevices = XboxInitializeBootDevices;
  MachVtbl.HwDetect = XboxHwDetect;
  MachVtbl.HwIdle = XboxHwIdle;

  /* Set LEDs to orange after init */
  XboxSetLED("oooo");
}

VOID
XboxPrepareForReactOS(IN BOOLEAN Setup)
{
    //
    // On XBOX, prepare video and turn off the floppy motor
    //
    XboxVideoPrepareForReactOS(Setup);
    DiskStopFloppyMotor();
}
