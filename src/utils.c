#include "../include/utils.h"

DWORD FindProcessId(const char *targetProcess) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    DWORD pid = 0;
    do {
        if (_stricmp(pe32.szExeFile, targetProcess) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return pid;
}

void PatchExplorer() {
    HWND hwnd = FindWindowA("Shell_TrayWnd", NULL);
    DWORD pid = 0;

    if (hwnd != NULL) {
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid != 0) {
            HANDLE h_explorer = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
            if (h_explorer != NULL) {
                TerminateProcess(h_explorer, 2);
                CloseHandle(h_explorer);
                printf("Explorer terminated successfully.\n");
            }
        }
    }

    Sleep(1000);
    ShellExecuteA(NULL, "open", "explorer.exe", NULL, NULL, SW_SHOWNORMAL);
}

void KillProcess(const char *targetProcess) {
    DWORD pid = FindProcessId(targetProcess);
    if (pid != 0) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    } else {
        printf("ERROR: Process not found: %s\n", targetProcess);
    }
}
