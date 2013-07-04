@echo off

:: This is needed so as to avoid static expansion of environment variables
:: inside if (...) conditionals.
:: See http://stackoverflow.com/questions/305605/weird-scope-issue-in-bat-file
:: for more explanation.
:: Precisely needed for configuring Visual Studio Environment.
setlocal enabledelayedexpansion

:: Special case %1 = arm_hosttools %2 = vcvarsall.bat %3 = %CMAKE_GENERATOR%
if /I "%1" == "arm_hosttools" (
    echo Configuring x86 host tools for ARM cross build

    :: This launches %VSINSTALLDIR%VS\vcvarsall.bat
    call %2 x86

    :: Configure host tools for x86
    cmake -G %3 -DARCH=i386 %~dp0
    exit
)

:: Get the source root directory
set REACTOS_SOURCE_DIR=%~dp0
set USE_VSCMD=0

:: Detect presence of cmake
cmd /c cmake --version 2>&1 | find "cmake version" > NUL || goto cmake_notfound

:: Detect build environment (MinGW, VS, WDK, ...)
if defined ROS_ARCH (
    echo Detected RosBE for %ROS_ARCH%
    set BUILD_ENVIRONMENT=MinGW
    set ARCH=%ROS_ARCH%
    if /I "%1" == "Codeblocks" (
        set CMAKE_GENERATOR="CodeBlocks - MinGW Makefiles"
    ) else if /I "%1" == "Eclipse" (
        set CMAKE_GENERATOR="Eclipse CDT4 - MinGW Makefiles"
    ) else if /I "%1" == "Makefiles" (
        set CMAKE_GENERATOR="MinGW Makefiles"
    ) else (
        set CMAKE_GENERATOR="Ninja"
    )

) else if defined DDK_TARGET_OS (
    echo Detected DDK/WDK for %DDK_TARGET_OS%-%_BUILDARCH%%
    set BUILD_ENVIRONMENT=WDK
    if "%_BUILDARCH%" == "x86" (
        set ARCH=i386
    ) else if "%_BUILDARCH%" == "AMD64" (
        set ARCH=amd64
    )
    set USE_VSCMD=1
    set USE_WDK_HEADERS=0

) else if defined VCINSTALLDIR (
    :: VS command prompt does not put this in environment vars
    cl 2>&1 | find "x86" > NUL && set ARCH=i386
    cl 2>&1 | find "x64" > NUL && set ARCH=amd64
    cl 2>&1 | find "ARM" > NUL && set ARCH=arm
    cl 2>&1 | find " 14." > NUL && set BUILD_ENVIRONMENT=VS8
    cl 2>&1 | find " 15." > NUL && set BUILD_ENVIRONMENT=VS9
    cl 2>&1 | find " 16." > NUL && set BUILD_ENVIRONMENT=VS10
    cl 2>&1 | find " 17." > NUL && set BUILD_ENVIRONMENT=VS11
    if not defined BUILD_ENVIRONMENT (
        echo Error: Visual Studio version too old or version detection failed.
        exit /b
    )

    echo Detected Visual Studio Environment !BUILD_ENVIRONMENT!-!ARCH!
    if /I "%1" == "VSSolution" (
        if "!BUILD_ENVIRONMENT!" == "VS8" (
            if "!ARCH!" == "amd64" (
                set CMAKE_GENERATOR="Visual Studio 8 2005 Win64"
            ) else (
                set CMAKE_GENERATOR="Visual Studio 8 2005"
            )
        ) else if "!BUILD_ENVIRONMENT!" == "VS9" (
            if "!ARCH!" == "amd64" (
                set CMAKE_GENERATOR="Visual Studio 9 2008 Win64"
            ) else (
                set CMAKE_GENERATOR="Visual Studio 9 2008"
            )
        ) else if "!BUILD_ENVIRONMENT!" == "VS10" (
            if "!ARCH!" == "amd64" (
                set CMAKE_GENERATOR="Visual Studio 10 Win64"
            ) else (
                set CMAKE_GENERATOR="Visual Studio 10"
            )
        ) else if "!BUILD_ENVIRONMENT!" == "VS11" (
            if "!ARCH!" == "amd64" (
                set CMAKE_GENERATOR="Visual Studio 11 Win64"
            ) else if "!ARCH!" == "arm" (
                set CMAKE_GENERATOR="Visual Studio 11 ARM"
            ) else (
                set CMAKE_GENERATOR="Visual Studio 11"
            )
        )
    ) else (
        set USE_VSCMD=1
        echo This script defaults to Ninja. To use Visual Studio GUI specify "VSSolution" as a parameter.
    )

) else if defined sdkdir (
    echo Detected Windows SDK %TARGET_PLATFORM%-%TARGET_CPU%
    if "%TARGET_CPU%" == "x86" (
        set ARCH=i386
    ) else if "%TARGET_CPU%" == "x64" (
        set ARCH=amd64
    )

    set BUILD_ENVIRONMENT=SDK
    set USE_VSCMD=1

) else (
    echo Error: Unable to detect build environment. Configure script failure.
    exit /b
)

:: Checkpoint
if not defined ARCH (
    echo Unknown build architecture
    exit /b
)

:: Detect VS command line generator
if %USE_VSCMD% == 1 (
    if /I "%1" == "CodeBlocks" (
        set CMAKE_GENERATOR="CodeBlocks - NMake Makefiles"
    ) else if /I "%1" == "Eclipse" (
        set CMAKE_GENERATOR="Eclipse CDT4 - NMake Makefiles"
    ) else if /I "%1" == "Makefiles" (
        set CMAKE_GENERATOR="NMake Makefiles"
    ) else (
        set CMAKE_GENERATOR="Ninja"
    )
)

:: Create directories
set REACTOS_OUTPUT_PATH=output-%BUILD_ENVIRONMENT%-%ARCH%
if "%REACTOS_SOURCE_DIR%" == "%CD%\" (
    echo Creating directories in %REACTOS_OUTPUT_PATH%

    if not exist %REACTOS_OUTPUT_PATH% (
        mkdir %REACTOS_OUTPUT_PATH%
    )
    cd %REACTOS_OUTPUT_PATH%
)

if not exist host-tools (
    mkdir host-tools
)
if not exist reactos (
    mkdir reactos
)

echo Preparing host tools...
cd host-tools
if EXIST CMakeCache.txt (
    del CMakeCache.txt /q
)
set REACTOS_BUILD_TOOLS_DIR=%CD%

:: Use x86 for ARM host tools
if "%ARCH%" == "arm" (
    :: Launch new script instance for x86 host tools configuration
    start "Preparing host tools for ARM cross build..." /I /B /WAIT %~dp0configure.cmd arm_hosttools "%VSINSTALLDIR%VC\vcvarsall.bat" %CMAKE_GENERATOR%
) else (
    cmake -G %CMAKE_GENERATOR% -DARCH=%ARCH% "%REACTOS_SOURCE_DIR%"
)

cd..

echo Preparing reactos...
cd reactos
if EXIST CMakeCache.txt (
    del CMakeCache.txt /q
)

if "%BUILD_ENVIRONMENT%" == "MinGW" (
    cmake -G %CMAKE_GENERATOR% -DENABLE_CCACHE=0 -DPCH=0 -DCMAKE_TOOLCHAIN_FILE=toolchain-gcc.cmake -DARCH=%ARCH% -DREACTOS_BUILD_TOOLS_DIR:DIR="%REACTOS_BUILD_TOOLS_DIR%" "%REACTOS_SOURCE_DIR%"
) else if "%BUILD_ENVIRONMENT%" == "WDK" (
    cmake -G %CMAKE_GENERATOR% -DCMAKE_TOOLCHAIN_FILE=toolchain-msvc.cmake -DUSE_WDK_HEADERS=%USE_WDK_HEADERS% -DARCH=%ARCH% -DREACTOS_BUILD_TOOLS_DIR:DIR="%REACTOS_BUILD_TOOLS_DIR%" "%REACTOS_SOURCE_DIR%"
) else (
    cmake -G %CMAKE_GENERATOR% -DCMAKE_TOOLCHAIN_FILE=toolchain-msvc.cmake -DARCH=%ARCH% -DREACTOS_BUILD_TOOLS_DIR:DIR="%REACTOS_BUILD_TOOLS_DIR%" "%REACTOS_SOURCE_DIR%"
)

cd..

echo Configure script complete^^! Enter directories and execute appropriate build commands (ex: ninja, make, nmake, etc...).
exit /b

:cmake_notfound
echo Unable to find cmake, if it is installed, check your PATH variable.
exit /b
