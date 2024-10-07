#pragma once
// S_Server.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。
#pragma warning(disable:4996)
#pragma once
#define DATA_BUFMAX 4096
#include <iostream>
#include <winSock2.h>
#include <vector>
#define IP_PORT 9999
// TODO: 在此处引用程序需要的其他标头。
// 基本包
typedef struct _WHO {
    int Num;
    char Name[20];
}WHO;
typedef struct _COMMAND {
    char Type[2];
    char Command[10];
}COMMAND;
typedef struct _NETWORK {
    SOCKET Socket;
    sockaddr_in SockAddr;
}NETWORK;
// _A_SOCKET 
typedef struct __A_SOCKET {
    int Num;
    char ClientSocketInfo[1024];
}A_SOCKET;
// 发送信息
typedef struct _SEND {
    WHO Who;
    COMMAND Command;
    char Info[DATA_BUFMAX];
}SENDINFO;
typedef struct _SUMSEND {
    SENDINFO SendInfo;// 需要发送的东西
    WSABUF DataBuf;// 发送需要的结构体 指向 SendInfo
    BOOL IsSend;
    DWORD SendSize;
}SUMSEND;
// 接收信息
typedef struct _CLIENT {
    WHO Who;
    COMMAND Command;
    char Info[];
}CLIENTINFO;// 用来解析
typedef struct _SUMCLIENT {
    WSABUF DataBuf;// 发送需要的结构体 指向 SendInfo
    DWORD ClientSize;
    char Info[DATA_BUFMAX];
}SUMCLIENT;
// 总信息
typedef struct _SYSTEM {
    NETWORK NetWork; // 自己的结构
    WHO ID_Info; // 自己的ID
    SUMSEND SendInfo; // 发送信息
    SUMCLIENT ClientInfo;
}SUM_Info;
// FILE
