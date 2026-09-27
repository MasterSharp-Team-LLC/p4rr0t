#include "utils.h"

#pragma comment(linker, "/subsystem:windows")
#pragma comment(linker, "/ENTRY:WinMainCRTStartup")

void PatchExplorer();
void KillProcess(const char *processName);
void InjectDLL(const char *processName, const char *dllName);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char *dllName = "p4rr0t_dll.dll";
    char targetProcess[256] = "notepad.exe";

    printf("p4rr0t by 85cs - Exploit by Itelcan3 (aka. @MasterSharp3210)\n");
    
    printf("\nTARGET: %s\n", targetProcess);
    printf("DLL: %s\n", dllName);

    printf("Starting process kill exploit first...\n");
    KillProcess(targetProcess);

    Sleep(1000);

    DWORD checkPid = FindProcessId(targetProcess);
    if (checkPid != 0) {
        printf("ERROR: Process %s is still running.\n", targetProcess);
        printf("Switching next exploit (DLL Injection)...\n\n");

        printf("Triggering kernel handles for process: %s\n", targetProcess);
        Sleep(1000);
        printf("Attaching DLL %s\n", dllName);
        printf("Injecting DLL %s\n", dllName);
        InjectDLL(targetProcess, dllName);

        printf("\nStarting daemon... Running in background\n");
        Sleep(2000);
            
        Sleep(1500);
        checkPid = FindProcessId(targetProcess);
        if (checkPid != 0) {
            printf("ERROR: Process %s is still running after DLL injection.\n", targetProcess);
            printf("Switching next exploit...\n");

            char exePath[MAX_PATH];
            GetModuleFileNameA(NULL, exePath, MAX_PATH);
            char *lastSlash = strrchr(exePath, '\\');
            if (lastSlash) {
                *(lastSlash + 1) = '\0';
                strcat_s(exePath, MAX_PATH, "piccione.exe");
            } else {
                strcpy_s(exePath, MAX_PATH, "piccione.exe");
            }

            ShellExecuteA(NULL, "open", exePath, NULL, NULL, SW_SHOWNORMAL);

        } else {
            printf("Process now closed! Respringing Explorer to patch...\n");
            PatchExplorer();
        }

    } else {
        printf("Process now closed! Respringing Explorer to patch...\n");
        PatchExplorer();
    }
    return 0;
}
