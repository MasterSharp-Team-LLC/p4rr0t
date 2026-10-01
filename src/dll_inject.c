#include "utils.h"

void InjectDLL(const char *targetProcess, const char *dllName) {
    char absDllPath[MAX_PATH];
    GetFullPathNameA(dllName, MAX_PATH, absDllPath, NULL);

    DWORD pid = FindProcessId(targetProcess);
    if (pid != 0) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (hProcess != NULL) {
            LPVOID pDllPathRemote = VirtualAllocEx(hProcess, NULL, strlen(absDllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (pDllPathRemote != NULL) {
                if (WriteProcessMemory(hProcess, pDllPathRemote, (LPVOID)absDllPath, strlen(absDllPath) + 1, NULL)) {
                    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
                    if (pLoadLibrary != NULL) {
                        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPathRemote, 0, NULL);
                        if (hThread != NULL) {
                            WaitForSingleObject(hThread, INFINITE);
                            CloseHandle(hThread);
                        }
                    }
                }
                VirtualFreeEx(hProcess, pDllPathRemote, 0, MEM_RELEASE);
            }
            CloseHandle(hProcess);
        }
    } else {
        printf("ERROR: Process not found: %s\n", targetProcess);
    }
}
