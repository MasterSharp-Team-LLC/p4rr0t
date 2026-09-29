# p4rr0t (a pigeon that bypasses!?)
![P4rr0t](parrot.png)
A lightweight, multi-exploit process management utility with an integrated TCP/Firebase receiver.

## Project Structure
- `src/`: Core C source files (`main.c`, `utils.c`, `dll_inject.c`, `process_kill.c`, `receiver.c`)
- `include/`: Header files (`utils.h`)
- `sender/`: Linux-compatible Python sender (`sender.py`)
- `build/`: Output directory for compiled binaries (`p4rr0t.exe`)

## Exploits & Architecture
1. **Normal Process Kill** (`KillProcess`)
2. **DLL Injection** (`p4rr0t_dll.dll`) with internal crash/exit
3. **Integrated Receiver Fallback**: If the first two methods fail, `p4rr0t.exe` starts an integrated TCP socket receiver (listening on port `9000`) paired via Firebase Realtime Database. This replaces bulky WebRTC libraries with a ultra-lightweight (~147 KB) single binary.

## How to Build (Windows)
Run the PowerShell build script:
```powershell
.\build.ps1
```
*(Automatically compiles with MSVC, sets up Windows SDK paths, and cleans up temporary object files).*

## Testing with Respawner Guard
To test all exploit fallbacks while keeping the target process active, run the 20-second test guard on Windows:
```powershell
python respawn_test.py
```
And then run:
```powershell
.\build\p4rr0t.exe
```

## Running the Linux Sender
On your Linux control machine (e.g. Raspberry Pi), run:
```bash
python3 sender/sender.py <target_windows_ip> 9000
```

## Thanks to <3
Thanks 85cs/Itelcan3 (aka. @MasterSharp3210) for **main** and **dll project** <3
Thanks franciplay (aka. @franciplay) for **piccione** exploit <3
