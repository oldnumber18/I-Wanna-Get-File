#include <iostream>
#include <winSock2.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string>
#define DATA_BUFMAX 2048
#define DATA_BUFMIN 45
#define DATA_BUFMID 2000
#define PORT 9999
#define INPUT_BUFSIZE 256
using namespace std;
typedef struct _WHO {
    int Num;
    char Name[20];
}Who;
typedef struct _SERVER {
    Who Who;// 谁发送的
    int Size;
    char Type[2];// 发送的类型
    char Command[10];
    char Info[DATA_BUFMID];// 要发送的内容
}SERVER;
// 文件大小
typedef struct _FILE {
    FILE* fp;
    BOOL RecvFile; // 文件状态
    int FileSize;// 总大小
    int GetSize;// 得到的大小
    char name[200];
}SAVE_FILE;

// 本地结构
typedef struct _NETWORK {
    // 信息
    SOCKET Socket; // 主体套接字
    BOOL ID = FALSE;// 身份
    char Name[20];// 名字（IP）信息
    // 结构
    char Info[DATA_BUFMAX]; // 接收发送过来的结构体信息
    SERVER SendInfo;// 要发送的结构
    SAVE_FILE SaveFile;// 要发送的文件结构
    WSABUF DataBuf;// 发送需要的结构体 处理发送的协议
    // 发送参数
    BOOL Sure;
    DWORD SendSize;// 要发送的数据体大小
    DWORD ClientSize;// 接收到的数据体大小

}NetWork, * network;

// 设置
void System();
void Server_dispose(int Goal);
// 通信
void Radio_HOST(int Goal, const char Type[], const char* Command, const char Send_Info[]);
void Radio_CLIENT(int Goal, const char Type[], const char* Command, const char Send_Info[]);
// 打印
void user_Information(const char* Sender, const char* Target, const char* Information);
void system_Information(const char* info);
void warn_Information(const char* Info);
void success_Information(const char* Info);
void prompt_Information(const char* Info);
void error(int error);
// 输入
void* Input(void* arg);
