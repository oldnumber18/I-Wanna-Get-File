#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#pragma warning(disable:4996)
#pragma warning(disable:4101)
// Windows 头文件
#include <windows.h>
#include <winSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;
#define DATA_BUFMAX 2048
#define DATA_BUFMIN 45
#define DATA_BUFMID 2000
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
}IDINFO;
typedef struct _SERVER {
    IDINFO ID;// 接收:谁发生过来的信息 发送:发送给谁？-->信息反馈目标
    int Size;// 消息大小，在 Command 的消息为 FileInfo 时，为文件大小
    char Type[2];// 发送的类型 第一次区分指令
    char Command[10];// 第二次区分指令
    char Info[DATA_BUFMID];// 要发送的内容
}SERVER;
typedef struct _FILE {
    FILE* fp;
    int FileSize;// 总大小
    int GetSize;// 得到的大小
    char name[200];
}SAVE_FILE;