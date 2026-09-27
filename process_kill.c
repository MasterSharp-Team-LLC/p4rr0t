#include "utils.h"

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
                printf("Explorer terminated successfully!\n");
            }
        }
    }

    Sleep(1000);
    ShellExecuteA(NULL, "open", "explorer.exe", NULL, NULL, SW_SHOWNORMAL);
}

void KillProcess(const char *processName) {
    DWORD pid = FindProcessId(processName);
    if (pid != 0) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    } else {
        printf("ERROR: Process not found: %s\n", processName);
    }
}
