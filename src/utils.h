#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DWORD FindProcessId(const char *targetProcess);
void PatchExplorer();
void KillProcess(const char *targetProcess);
void InjectDLL(const char *targetProcess, const char *dllName);
void StartReceiver(const char *pairCode);

#endif
