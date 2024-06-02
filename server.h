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
    Who Who;// ˭���͵�
    int Size;
    char Type[2];// ���͵�����
    char Command[10];
    char Info[DATA_BUFMID];// Ҫ���͵�����
}SERVER;
// �ļ���С
typedef struct _FILE {
    FILE* fp;
    BOOL RecvFile; // �ļ�״̬
    int FileSize;// �ܴ�С
    int GetSize;// �õ��Ĵ�С
    char name[200];
}SAVE_FILE;

// ���ؽṹ
typedef struct _NETWORK {
    // ��Ϣ
    SOCKET Socket; // �����׽���
    BOOL ID = FALSE;// ���
    char Name[20];// ���֣�IP����Ϣ
    // �ṹ
    char Info[DATA_BUFMAX]; // ���շ��͹����Ľṹ����Ϣ
    SERVER SendInfo;// Ҫ���͵Ľṹ
    SAVE_FILE SaveFile;// Ҫ���͵��ļ��ṹ
    WSABUF DataBuf;// ������Ҫ�Ľṹ�� �����͵�Э��
    // ���Ͳ���
    BOOL Sure;
    DWORD SendSize;// Ҫ���͵��������С
    DWORD ClientSize;// ���յ����������С

}NetWork, * network;

// ����
void System();
void Server_dispose(int Goal);
// ͨ��
void Radio_HOST(int Goal, const char Type[], const char* Command, const char Send_Info[]);
void Radio_CLIENT(int Goal, const char Type[], const char* Command, const char Send_Info[]);
// ��ӡ
void user_Information(const char* Sender, const char* Target, const char* Information);
void system_Information(const char* info);
void warn_Information(const char* Info);
void success_Information(const char* Info);
void prompt_Information(const char* Info);
void error(int error);
// ����
void* Input(void* arg);