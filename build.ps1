Write-Host "Building unified p4rr0t (with integrated background TCP receiver) using direct MSVC paths..." -ForegroundColor Cyan

$msvcBin = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64"
$msvcInclude = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\include"
$msvcLib = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\lib\x64"

$sdkShared = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared"
$sdkUcrt = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt"
$sdkUm = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um"

$sdkUcrtLib = "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64"
$sdkUmLib = "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"

if (-not (Test-Path "$msvcBin\cl.exe")) {
    Write-Error "Visual Studio compiler not found at $msvcBin"
    exit 1
}

# Set up environment variables for cl.exe
$env:PATH = "$msvcBin;" + $env:PATH
$env:INCLUDE = "$msvcInclude;$sdkShared;$sdkUcrt;$sdkUm"
$env:LIB = "$msvcLib;$sdkUcrtLib;$sdkUmLib"

if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Write-Host "Compiling p4rr0t.exe for background execution..." -ForegroundColor Cyan
& "$msvcBin\cl.exe" src/main.c src/utils.c src/dll_inject.c src/process_kill.c src/receiver.c /Iinclude user32.lib shell32.lib advapi32.lib ws2_32.lib wininet.lib /Fe:build/p4rr0t.exe /link /subsystem:windows /ENTRY:mainCRTStartup

if ($LASTEXITCODE -eq 0) {
    Remove-Item *.obj -ErrorAction SilentlyContinue
    Write-Host "Build successful! Background executable located at build/p4rr0t.exe" -ForegroundColor Green
} else {
    Write-Error "Build failed with exit code $LASTEXITCODE"
}
