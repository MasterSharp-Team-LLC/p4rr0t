@echo off
setlocal

:: Check if cl.exe is already available in PATH (e.g. if run from Developer Command Prompt)
where cl.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo MSVC environment already active.
    goto :compile
)

echo Looking for Visual Studio...

set "VCVARS="
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"

if not defined VCVARS (
    echo Error: Visual Studio MSVC compiler not found automatically.
    echo Please run this script from the "Developer Command Prompt for VS".
    pause
    exit /b 1
)

echo Initializing MSVC environment from: %VCVARS%
call "%VCVARS%" x64

:compile
echo Building p4rr0t...
if not exist build mkdir build
cl.exe src/main.c src/utils.c src/dll_inject.c src/process_kill.c /Iinclude user32.lib shell32.lib advapi32.lib /Fe:build/p4rr0t.exe /link /subsystem:windows

if %errorlevel% equ 0 (
    echo Build successful! Executable located at build/p4rr0t.exe
) else (
    echo Build failed with error code %errorlevel%.
)
endlocal
pause
