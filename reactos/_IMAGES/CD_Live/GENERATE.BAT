REM COPYRIGHT:       See COPYING in the top level directory
REM PROJECT:         ReactOS 
REM FILE:            GENERATE.BAT
REM PURPOSE:         Visual Studio's executes this file to generate a live CD.
REM PROGRAMMERS:     J. C. Jones

REM The following XCOPY command has the purpose of pulling a virgin LiveCD.ISO from https://www.reactos.org/getbuilds and extracting it in
REM the current directory so that its root directory is named "root". Then, we copy our own generated projects over the virgin projects.
REM Then we use the cdmake.exe command to generate our ReactOS_CD_Live.iso.
REM Finally, we execute our ReactOS_CD_Live.iso using VirtualBox or similar.
REM If the execution of ReactOS_CD_Live.iso within VirtualBox (or similar) results in any visible errors, we systematically determine
REM which of our own projects is the culprit. As you can see below, there are several of our own projects whose copy command is
REM preceded by a "REM" statement. These projects are known culprits. By remove the "REM" prefix on a copy command you can see how that
REM file ruins the boot process of ReactOS. Sometimes you might get a blank gray screen. Sometimes you will get a blank blue screen.
REM Sometimes the screen will be black. And sometimes the shell (icons/etc.) will be messed up. Sometimes and application, such as CALC.EXE,
REM will refuse to run. The objective is to clean up each of our defective projects to get a clean ReactOS boot.

XCOPY /E /I /Y "root" "%outdir%root"

cd "%outdir%"

mkdir root
cd root
call :copy_to_CD "%solutiondir%boot\freeldr\bootsect\%outdir%isoboot.bin" .

mkdir Loader
cd loader
call :copy_to_CD "%solutiondir%boot\freeldr\freeldr\%outdir%freeldr.sys" setupldr.sys

cd ..

call :copy_to_CD "%solutiondir%boot\bootdata\livecd.ini" FREELDR.INI

mkdir Profiles
cd Profiles
echo We would write profile files here.

cd ..

mkdir reactos
cd reactos
REM call :copy_to_CD "%solutiondir%base\shell\explorer\%outdir%EXPLORER.EXE" .
REM call :copy_to_CD "%solutiondir%base\shell\explorer\explorer-cfg-template.xml" .

mkdir system32
cd system32

REM call :copy_to_CD "%solutiondir%ntoskrnl\%outdir%NTOSKRNL.EXE" .
call :copy_to_CD "%solutiondir%ntkrnlmp\%outdir%NTKRNLMP.EXE" .
call :copy_to_CD "%solutiondir%HAL\%outdir%HAL.DLL" .
call :copy_to_CD "%solutiondir%Drivers\Base\BootVid\%outdir%BootVid.DLL" .
call :copy_to_CD "%solutiondir%Drivers\Base\KDCOM\%outdir%KDCOM.DLL" .
call :copy_to_CD "%solutiondir%base\applications\ATACTL\%outdir%ATACTL.EXE" .
call :copy_to_CD "%solutiondir%base\applications\cacls\%outdir%CACLS.EXE" .
call :copy_to_CD "%solutiondir%base\applications\sc\%outdir%sc.EXE" .
REM "call :copy_to_CD %solutiondir%base\applications\CALC\%outdir%CALC.EXE" .
call :copy_to_CD "%solutiondir%media\nls\*.nls" .

mkdir drivers
cd drivers

call :copy_to_CD "%solutiondir%drivers\base\beep\%outdir%BEEP.SYS" .
call :copy_to_CD "%solutiondir%drivers\filesystems\CDFS\%outdir%CDFS.SYS" .
call :copy_to_CD "%solutiondir%drivers\bus\PCI\%outdir%PCI.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbuhci\%outdir%USBUHCI.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbd\%outdir%USBD.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbccgp\%outdir%USBCCGP.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbehci\%outdir%USBEHCI.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbhub\%outdir%USBHUB.SYS" .
call :copy_to_CD "%solutiondir%drivers\bus\acpi\%outdir%ACPI.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbohci\%outdir%USBOHCI.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\ide\uniata\%outdir%UNIATA.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\scsiport\%outdir%SCSIPORT.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\port\buslogic\%outdir%BUSLOGIC.SYS" .
call :copy_to_CD "%solutiondir%drivers\usb\usbstor\%outdir%USBSTOR.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\class\class2\%outdir%CLASS2.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\class\cdrom\%outdir%CDROM.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\class\disk\%outdir%DISK.SYS" .
call :copy_to_CD "%solutiondir%drivers\storage\class\ramdisk\%outdir%RAMDISK.SYS" .
call :copy_to_CD "%solutiondir%drivers\base\nmidebug\%outdir%NMIDEBUG.SYS" .
call :copy_to_CD "%solutiondir%drivers\filesystems\mup\%outdir%MUP.SYS" .
call :copy_to_CD "%solutiondir%drivers\network\ndis\%outdir%NDIS.SYS" .
call :copy_to_CD "%solutiondir%drivers\battery\battc\%outdir%BATTC.SYS" .
call :copy_to_CD "%solutiondir%drivers\sac\%outdir%SACDRV.SYS" .
call :copy_to_CD "%solutiondir%win32ss\drivers\videoprt\%outdir%VIDEOPRT.SYS" .


cd ..

REM call :copy_to_CD "%solutiondir%dll\ntdll\%outdir%NTDLL.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\advapi32\%outdir%ADVAPI32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\browseui\%outdir%BROWSEUI.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\comctl32\%outdir%COMCTL32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\cryptui\%outdir%CRYPTUI.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\dbghelp\%outdir%DBGHELP.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\devmgr\%outdir%DEVMGR.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\dhcpcsvc\%outdir%DHCPCSVC.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\dnsapi\%outdir%DNSAPI.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\fmifs\%outdir%FMIFS.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\icmp\%outdir%ICMP.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\ieframe\%outdir%IEFRAME.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\imagehlp\%outdir%IMAGEHLP.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\imm32\%outdir%IMM32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\iphlpapi\%outdir%IPHLPAPI.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\kernel32\%outdir%KERNEL32.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\lsasrv\%outdir%LSASRV.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\mlang\%outdir%MLANG.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\mpr\%outdir%MPR.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\msimg32\%outdir%MSIMG32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\msvcrt\%outdir%MSVCRT.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\newdev\%outdir%NEWDEV.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\ole32\%outdir%OLE32.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\oleaut32\%outdir%OLEAUT32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\olecli32\%outdir%OLECLI32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\olesvr32\%outdir%OLESVR32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\olethk32\%outdir%OLETHK32.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\rpcrt4\%outdir%RPCRT4.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\samsrv\%outdir%SAMSRV.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\shdocvw\%outdir%SHDOCVW.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\shell32\%outdir%SHELL32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\shlwapi\%outdir%SHLWAPI.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\propsys\%outdir%PROPSYS.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\psapi\%outdir%PSAPI.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\secur32\%outdir%SECUR32.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\setupapi\%outdir%SETUPAPI.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\url\%outdir%URL.DLL" .
REM call :copy_to_CD "%solutiondir%dll\win32\urlmon\%outdir%URLMON.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\userenv\%outdir%USERENV.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\usp10\%outdir%USP10.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\uxtheme\%outdir%UXTHEME.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\version\%outdir%VERSION.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\windowscodecs\%outdir%WINDOWSCODECS.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\wininet\%outdir%WININET.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\winmm\%outdir%WINMM.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\winspool\%outdir%WINSPOOL.DRV" .
call :copy_to_CD "%solutiondir%dll\win32\wintrust\%outdir%WINTRUST.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\ws2help\%outdir%WS2HELP.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\ws2_32\%outdir%WS2_32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\crypt32\%outdir%CRYPT32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\advpack\%outdir%ADVPACK.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\comdlg32\%outdir%COMDLG32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\lz32\%outdir%LZ32.DLL" .
call :copy_to_CD "%solutiondir%dll\win32\wldap32\%outdir%WLDAP32.DLL" .
call :copy_to_CD "%solutiondir%base\applications\atactl\%outdir%ATACTL.EXE" .
call :copy_to_CD "%solutiondir%base\system\autochk\%outdir%AUTOCHK.EXE" .
REM call :copy_to_CD "%solutiondir%base\system\lsass\%outdir%LSASS.EXE" .
REM call :copy_to_CD "%solutiondir%base\system\smss\%outdir%SMSS.EXE" .
REM call :copy_to_CD "%solutiondir%base\system\winlogon\%outdir%WINLOGON.EXE" .
REM call :copy_to_CD "%solutiondir%base\system\userinit\%outdir%USERINIT.EXE" .
REM call :copy_to_CD "%solutiondir%base\system\services\%outdir%SERVICES.EXE" .
REM call :copy_to_CD "%solutiondir%base\shell\cmd\%outdir%CMD.EXE" .
call :copy_to_CD "%solutiondir%base\shell\explorer\notifyhook\%outdir%NOTIFYHOOK.DLL" .
call :copy_to_CD "%solutiondir%win32ss\drivers\font\ftfd\%outdir%FTFD.DLL" .
call :copy_to_CD "%solutiondir%win32ss\drivers\displays\framebuf\%outdir%FRAMEBUF.DLL" .
call :copy_to_CD "%solutiondir%win32ss\drivers\displays\vga\%outdir%VGADDI.DLL" .
call :copy_to_CD "%solutiondir%win32ss\win32k\%outdir%WIN32K.SYS" .
REM call :copy_to_CD "%solutiondir%win32ss\user\user32\%outdir%USER32.DLL" .
REM call :copy_to_CD "%solutiondir%win32ss\user\winsrv\%outdir%WINSRV.DLL" .
call :copy_to_CD "%solutiondir%win32ss\gdi\gdi32\%outdir%GDI32.DLL" .
REM call :copy_to_CD "%solutiondir%subsystems\win\basesrv\%outdir%BASESRV.DLL" .
REM call :copy_to_CD "%solutiondir%subsystems\win32\csrsrv\%outdir%CSRSRV.DLL" .
REM call :copy_to_CD "%solutiondir%subsystems\win32\csrss\%outdir%CSRSS.EXE" .

GOTO :make_CD

mkdir config
cd "%solutiondir%boot\bootdata"
REM "%solutiondir%tools\mkhive\build\x86_32\$(Configuration)\mkhive.exe" "%targetdir%\root\reactos\system32\config" hivecls.inf hivedef.inf hivesft.inf hivesys.inf livecd.inf hiveinst.inf

:make_CD
echo Creating ReactOS CD Live, writing Master-Boot-Rectord (MBR) in the process:
"%solutiondir%tools\cdmake\%outdir%cdmake.exe" -j -m -b "%solutiondir%boot\freeldr\bootsect\%outdir%isoboot.bin" "%targetdir%\root"  ReactOSLive "%targetdir%\ReactOS_CD_Live.iso"
GOTO:EOF

:copy_to_CD
echo %~1
copy "%~1" "%~2" 1>nul
IF NOT EXIST "%~1" ECHO ********** FILE NOT FOUND: %~1 **********
GOTO:EOF
