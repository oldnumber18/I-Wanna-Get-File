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
#define GetServerInfo_MAX_SumSize 4132
#define GetServerInfo_Other_SumSize 36
#define SendServerInfo_Other_SumSize 26
#define Note_MAX_Size 4130
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
// 发送到服务器结构
typedef struct _WHO {
    int Num;
    char Name[20];
}who;
typedef struct _GetSystemInfo {// 解析包
    who ID;// 接收:谁发生过来的信息 发送:发送给谁？-->信息反馈目标
    char Type[2];// 发送的类型 第一次区分指令
    char Command[10];// 第二次区分指令
    char Info[];// 要发送的内容
}GetServerInfo;
typedef struct _SendSystemInfo {// 发送包
    who ID;
    char Type[2];
    char Command[10];// 第二次区分指令
    char *Info;
}SendServerInfo;
// FILE
typedef struct _FILE {// 文件信息
    signed long int Size;
    char Name[];
}INFOFILE;
typedef struct _SAVEFILE {// 保存文件结构
    FILE* fp;
    signed long SizeSUM;
    signed long ClientSUMSize;
    char Name[];
}SAVEFILE;
typedef struct _CLIENTFILE {// 接收/发送 文件
    signed int Size;
    char Note[];
}CLIENTFILE;