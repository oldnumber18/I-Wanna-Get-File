#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#pragma warning(disable:4996)
#pragma warning(disable:4101)
#pragma warning(disable:4200)
// Windows 头文件
#include <windows.h>
#include <winSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;
#define DATA_BUFMAX 4096
#define DATA_BUFMID 2048
#define DATA_BUFMIN 1024
#define DATA_MIN 512
#define IP_PORT 9999
#define IP_ADDR "127.0.0.1"
#pragma comment(lib,"ws2_32.lib")
extern "C" {
    __declspec(dllexport) int ClientDLL();
}
// 基本包
typedef struct _WHO {
    int Num;
    char Name[20];
}WHO;
typedef struct _COMMAND {
    char Type[2];
    char Command[10];
}COMMAND;
// 
typedef struct _GetSystemInfo {// 解析包
    WHO ID_Info;// 接收:谁发生过来的信息 发送:发送给谁？-->信息反馈目标
    COMMAND Command;
    char Info[];// 要发送的内容
}GetServerInfo;
typedef struct _SendSystemInfo {// 发送包
    WHO ID_Info;// 接收:谁发生过来的信息 发送:发送给谁？-->信息反馈目标
    COMMAND Command;
    char *Info;// 要发送的内容
}SendServerInfo;
// FILE
typedef struct _SAVEFILE {// 保存文件结构
    FILE* fp;
    signed long int SizeSUM;
    signed long int ClientSUMSize;
    char Name[DATA_MIN];
}SAVEFILE;
typedef struct _FILE {// 文件信息
    signed long int Size;
    char Name[DATA_MIN];
}INFOFILE;
typedef struct _CLIENTFILE {// 接收/发送 文件内容
    signed int Size;
    char Note[];
}FILETEXT;
