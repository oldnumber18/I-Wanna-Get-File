#pragma warning(disable:4996)
#pragma warning(disable:4312)
#include "server.h"
// 函数
int New_Socket(SOCKET Socket);// 新连接
void Del_Socket(int Index);// 处理连接
// 参数
bool SYSTEM_Log = false;// 日志开关
bool SYSTEM_Bell = true;// 响铃
extern int Input_Goal;
int Socket_Num = 0;// 套接字总数
network Socket_Array[FD_SETSIZE];// 套接字指针
void start() {
    system_Information("初始化中......");
    FILE *fp = fopen("system.txt", "r");
    if (fp == NULL) {
        fp = fopen("system.txt", "w");
        fprintf(fp,"0 1\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
    }
    else {
        int Bell = 0, Log = 1;
        if (fscanf(fp, "%d%d", &Bell, &Log) != 2) {
            fp = fopen("system.txt", "w");
            fprintf(fp, "0 1\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
        }
        if (Bell == 0) {
            SYSTEM_Bell = false;
            success_Information("系统消息提示时铃声已关闭");
        }
        else {
            SYSTEM_Bell = true;
            success_Information("系统消息提示时铃声已开启");
        }
        if (Log == 0) {
            SYSTEM_Log = false;
            success_Information("日志已关闭");
        }
        else {
            SYSTEM_Log = true;
            success_Information("日志已开启");
        }
    }
    fclose(fp);
}
void System() {
    FILE* fp = fopen("system.txt", "w");
    if (fp == NULL) {
        fp = fopen("system.txt", "w");
        fprintf(fp, "0 1\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
    }
    else {
        if (SYSTEM_Bell == false) {
            if (SYSTEM_Log == false) {
                fprintf(fp, "0 0\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
            }
            else {
                fprintf(fp, "0 1\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
            }
        }
        else {
            if (SYSTEM_Log == false) {
                fprintf(fp, "1 0\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
            }
            else {
                fprintf(fp, "1 1\n注释:参数（0为关闭 1为开启）分别为 ：系统提示是否发出铃声 是否开启日志 \n铃声默认关闭 日志默认开启\n");
            }
        }
    }
    fclose(fp);
}
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(NULL);
    SetConsoleOutputCP(936); // 设置编码
    system("title SERVER by:lonely OLD");
    start();
    int Total = 0;
    SOCKET AcceptSocket;// 与客户端通信的套接字
// 初始化
    // 初始化WinSock环境
    WSADATA wsaData;
    if ((Total = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) error(1);
    // 创建用于监听的套接字
    SOCKET tcp_Listen;
    if ((tcp_Listen = WSASocket(AF_INET, SOCK_STREAM,IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) error(2);
    // 设置
    sockaddr_in clientAdder;// 客户端信息
    sockaddr_in serverAdder;// 服务器端信息
    serverAdder.sin_family = AF_INET;
    serverAdder.sin_port = htons(9999);
    serverAdder.sin_addr.s_addr = htonl(INADDR_ANY);
    int clientAddersize = sizeof(sockaddr_in);
    // 绑定
    if (bind(tcp_Listen, (PSOCKADDR)&serverAdder, sizeof(serverAdder)) == SOCKET_ERROR) error(3);
    // 监听
    if (listen(tcp_Listen, 10)) error(4);
    // 设置非阻塞模式
    u_long mode = 1;
    if (ioctlsocket(tcp_Listen, FIONBIO, &mode) == SOCKET_ERROR) error(5);
    New_Socket(tcp_Listen);
    strcpy(Socket_Array[0]->Name, "SYSTEM");
// 开启监听
    system_Information("开始运行程序");
    pthread_t thread;
    pthread_create(&thread, NULL, Input, NULL);
    pthread_detach(thread);
    FD_SET ReadSet;
    FD_SET WriteSet;
    while (true) {
        FD_ZERO(&ReadSet);// 清空,初始化套接字状态
        FD_ZERO(&WriteSet);
        FD_SET(tcp_Listen, &ReadSet);// 添加
        for (int i = 0; i < Socket_Num; i++) {
            network Socket_Info = Socket_Array[i];
            FD_SET(Socket_Info->Socket, &ReadSet);
            FD_SET(Socket_Info->Socket, &WriteSet);
        }
        // 阻塞
        if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR) error(5);
//处理消息
        for (int i = 0; i < Socket_Num; i++) {
            network Socket_Info = Socket_Array[i];
            //select将更新这个集合,把其中不可读的套节字去掉,只保留可读的套节字在这个集合里面。再检查套接字是不是可读的
            if (FD_ISSET(Socket_Info->Socket, &ReadSet)) { // 检查Socket是否在这个集合里面
                // 新连接
                if (Socket_Info->Socket == tcp_Listen) { // 监听套接字的可读表示有新的连接
                    if ((AcceptSocket = accept(tcp_Listen, (struct sockaddr*)&clientAdder, &clientAddersize)) != INVALID_SOCKET) {
                        mode = 1;
                        if ((ioctlsocket(AcceptSocket, FIONBIO, &mode)) == SOCKET_ERROR) {
                            error(5);
                            return -1;
                        }
                        int Location = New_Socket(AcceptSocket);
                        char name[20] = { 0 };
                        sprintf(name, "%s:%d", inet_ntoa(clientAdder.sin_addr), ntohs(clientAdder.sin_port));
                        strcpy(Socket_Array[Location]->Name, name);
                        char List[sizeof(Socket_Info->SendInfo.Info)] = { 0 };
                        sprintf(List, "广播消息:新连接:IP[%s],目前一共有连接: %d 个客户端", Socket_Array[Location]->Name, Socket_Num-1);
                        for (int i = 1; i < Socket_Num; i++) {
                            char Info[50] = { 0 };
                            sprintf(Info, "\n第 %d 个连接|%s ID:%d", i, Socket_Array[i]->Name, Socket_Array[i]->ID);
                            strcat(List, Info);
                        }
                        system_Information(List);
                        for (int Client_Now = 1; Client_Now < Socket_Num; Client_Now++) Radio_HOST(Client_Now, "R", NULL, List);
                    }
                    else error(6);
                }
                // 接收消息
                else {
                    memset(Socket_Info->Info, 0, DATA_BUFMAX);
                    Socket_Info->DataBuf.buf = Socket_Info->Info;
                    Socket_Info->DataBuf.len = DATA_BUFMAX;
                    DWORD Flags = 0;
                    if (WSARecv(Socket_Info->Socket, &Socket_Info->DataBuf, 1, &Socket_Info->ClientSize, &Flags, NULL, NULL) == SOCKET_ERROR) {
                        error(9);
                        Del_Socket(i);
                        continue;
                    }
                    else {
                        // 处理消息,写入对方结构
                        if (Socket_Info->ClientSize == 0) {
                            error(9);
                            Del_Socket(i);
                            continue;
                        }
                        SERVER* ClientInfo = (SERVER*)&Socket_Info->Info;// 解析
                        network SendInfo = Socket_Array[ClientInfo->Who.Num];// 目标
                        if (SendInfo == NULL) { Del_Socket(i); continue; }
                        // File F类型
                        if (strcmp(ClientInfo->Type, "F") == 0) {
                            if (strcmp(ClientInfo->Command, "FileInfo") == 0) {
                                char Info[200] = { 0 };
                                Socket_Info->SaveFile.FileSize = ClientInfo->Size;
                                Socket_Info->SaveFile.fp = fopen(ClientInfo->Info, "wb+");
                                Socket_Info->SaveFile.GetSize = 0;
                                memset(Socket_Info->SaveFile.name, 0, sizeof(Socket_Info->SaveFile.name));
                                strcpy(Socket_Info->SaveFile.name, ClientInfo->Info);
                                if (ClientInfo->Who.Num == 0) {
                                    sprintf(Info, "准备接收文件。文件名字:%s 大小:%d Bite(%dMB)\n", ClientInfo->Info, ClientInfo->Size, ClientInfo->Size / 1024 / 1024);
                                    prompt_Information(Info);
                                    Radio_CLIENT(i, "F", "", "RecvFileInfoSure");// 反馈已收到
                                    printf("已设置反馈信息\n");
                                    continue;
                                }
                            }
                            else if (strcmp(ClientInfo->Command, "RecvFile") == 0) {
                                if (Socket_Info->SaveFile.fp != NULL) {
                                    fwrite(ClientInfo->Info, sizeof(char), ClientInfo->Size, Socket_Info->SaveFile.fp);
                                    Socket_Info->SaveFile.GetSize += ClientInfo->Size;
                                    char Info[100] = { 0 };
                                    sprintf(Info, "文件:%s 大小:%d 已接收:%d", Socket_Info->SaveFile.name, Socket_Info->SaveFile.FileSize, Socket_Info->SaveFile.GetSize);
                                    prompt_Information(Info);
                                    if (Socket_Info->SaveFile.FileSize == Socket_Info->SaveFile.GetSize) {
                                        fclose(Socket_Info->SaveFile.fp);
                                        success_Information("\n文件接收完毕!");
                                        Socket_Info->SaveFile.FileSize = 0;
                                        Socket_Info->SaveFile.GetSize = 0;
                                        goto Send;
                                    }
                                    if (ClientInfo->Who.Num == 0) {
                                        // 反馈接收成功，继续接收
                                        Radio_CLIENT(i, "F", "Sure", "RecvFileSure");
                                    }
                                }
                                else {
                                    system_Information("接收到未知文件发送申请");
                                }
                            }
                            else if (strcmp(ClientInfo->Info, "continue") == 0) Socket_Info->SaveFile.RecvFile = TRUE;
                        }
                        Send:
                        if (ClientInfo->Who.Num == 0) {
                            if (Socket_Info->ID == TRUE) Server_dispose(i);
                            else
                            {
                                if (strcmp(ClientInfo->Type, "G") == 0) {
                                    char Send_Info[100] = { 0 };
                                    char Info[100] = { 0 };
                                    sprintf(Info, "%s尝试提升权限", Socket_Info->Name);
                                    system_Information(Info);
                                    if (strcmp(ClientInfo->Info, "OLD") == 0) {
                                        sprintf(Info, "%s成功提升为使用者，输入为:%s", Socket_Info->Name, ClientInfo->Info);
                                        Socket_Info->ID = TRUE;
                                        Radio_CLIENT(i, "G", "SUCCESS", "SUCCESS");
                                    }
                                    else {
                                        sprintf(Info, "%s提升失败，输入为:%s", Socket_Info->Name, ClientInfo->Info);
                                        system_Information(Info);
                                        Radio_CLIENT(i, "G", "ERROR", "输入密码错误");
                                    }
                                }
                            }
                            if (strcmp(ClientInfo->Type,"F") != 0)
                                user_Information(Socket_Info->Name, "SYSTEM", ClientInfo->Info);
                        }
                        else {
                            if (0 < ClientInfo->Who.Num && ClientInfo->Who.Num < Socket_Num) {
                                // 保存到目标结构，设置是谁发送的
                                SendInfo->SendInfo.Who.Num = i;
                                strcpy(SendInfo->SendInfo.Who.Name, Socket_Info->Name);
                                SendInfo->SendInfo.Size = ClientInfo->Size;
                                strcpy(SendInfo->SendInfo.Type, ClientInfo->Type);
                                strcpy(SendInfo->SendInfo.Info, ClientInfo->Info);
                                SendInfo->Sure = TRUE;
                            }
                        }
                    }
                }
            }
            else {
                // 如果当前套接字在WriteSet集合中，可以发送数据
                if (FD_ISSET(Socket_Info->Socket, &WriteSet)) {
                    if (Socket_Info->Sure == TRUE) {
                        Socket_Info->DataBuf.buf = (char*)&Socket_Info->SendInfo;
                        Socket_Info->DataBuf.len = DATA_BUFMAX;
                        if (WSASend(Socket_Info->Socket, &Socket_Info->DataBuf, 1, &Socket_Info->SendSize, 0, NULL, NULL) == SOCKET_ERROR) {
                                error(10);
                                Del_Socket(i);
                                continue;
                        };
                        Socket_Info->Sure = FALSE;
                        if (strcmp(Socket_Info->SendInfo.Command,"RecvFile") != 0)
                            if (strcmp(Socket_Info->SendInfo.Type, "F") != 0)
                            user_Information(Socket_Info->SendInfo.Who.Name, Socket_Info->Name, Socket_Info->SendInfo.Info);
                    }
                }
            }
        }
    }
}
// 创造套接字信息
int New_Socket(SOCKET Socket) {
    network Socket_Info = NULL;
    // 分配内存
    if ((Socket_Info = (network)GlobalAlloc(GPTR, sizeof(NetWork))) == NULL) error(11);
    Socket_Info->Socket = Socket;
    Socket_Info->Sure = FALSE;
    Socket_Info->SaveFile.RecvFile = FALSE;
    Socket_Info->SaveFile.GetSize = 0;
    Socket_Info->SaveFile.FileSize = 0;
    Socket_Array[Socket_Num] = Socket_Info;
    Socket_Num++;
    return Socket_Num-1;// 返回在数组的位置
}
void Del_Socket(int Index) {
    Socket_Num--;
    network Socket_Info = Socket_Array[Index];
    if (Socket_Info == NULL) {
        return;
    }
    // 广播消息
    char List[sizeof(Socket_Info->SendInfo.Info)] = {0};
    sprintf(List, "广播消息:%s 的连接已退出,目前在线人数:%d", Socket_Info->Name, Socket_Num - 1);
    for (int i = 1; i < Socket_Num; i++) {
        char Info[50] = { 0 };
        sprintf(Info, "\n第 %d 个连接|%s ID:%d", i, Socket_Array[i]->Name, Socket_Array[i]->ID);
        strcat(List, Info);
    }
    system_Information(List);
    for (int Client_Now = 1; Client_Now < Socket_Num; Client_Now++) Radio_HOST(Client_Now, "R", NULL, List);
    if (Socket_Num > Index && Index > 0) {
        closesocket(Socket_Info->Socket);
        GlobalFree(Socket_Info);
        for (int i = Index; i < Socket_Num; i++) {
            Socket_Array[i] = Socket_Array[i + 1];
        }
        // 调整本地发送目标
        if (Input_Goal == Index) {
            Input_Goal = 0;
            system_Information("您选择发送的目标已取消连接。自动帮助你调整目标为 服务器本地");
        }
        else if (Input_Goal > Index) {
            Input_Goal--;
            system_Information("您选择发送的目标已移动至前一位。自动帮助你调整目标");
        }
        // 这里是修改目标
        for (int i = Index; i < Socket_Num; i++) {
            Socket_Info = Socket_Array[i];
            if (Socket_Info->SendInfo.Who.Num == Index) {
                Socket_Info->SendInfo.Who.Num = 0;
            }
            else if (Socket_Info->SendInfo.Who.Num > Index) {
                Socket_Info->SendInfo.Who.Num--;
            }
        }
    }
}
void Radio_HOST(int Goal, const char Type[], const char *Command, const char Send_Info[]) {
    if (Goal == 0) return;
    network Socket_Info = Socket_Array[Goal];
    if (Socket_Info == NULL) { Del_Socket(Goal); return; }
    if (Socket_Info->ID == FALSE) return;
    strcpy(Socket_Info->SendInfo.Who.Name, "SERVER");
    Socket_Info->SendInfo.Who.Num = 0;
    Socket_Info->SendInfo.Size = strlen(Send_Info);
    strcpy(Socket_Info->SendInfo.Type, Type);
    strcpy(Socket_Info->SendInfo.Command, Command);
    strcpy(Socket_Info->SendInfo.Info, Send_Info);
    Socket_Info->Sure = TRUE;
}
void Radio_CLIENT(int Goal, const char Type[], const char* Command, const char Send_Info[]) {
    network Socket_Info = Socket_Array[Goal];
    if (Socket_Info == NULL) { Del_Socket(Goal); return; }
    strcpy(Socket_Info->SendInfo.Who.Name, "SERVER");
    Socket_Info->SendInfo.Who.Num = 0;
    Socket_Info->SendInfo.Size = strlen(Send_Info);
    strcpy(Socket_Info->SendInfo.Type, Type);
    strcpy(Socket_Info->SendInfo.Command, Command);
    strcpy(Socket_Info->SendInfo.Info, Send_Info);
    Socket_Info->Sure = TRUE;
}