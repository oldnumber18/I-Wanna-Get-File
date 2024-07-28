#pragma warning(disable:4996)
#include "server.h"
extern bool SYSTEM_Log;// 日志开关
extern bool SYSTEM_Bell;// 响铃
// 时间
std::string Get_time() {
    char Now_time[28] = {0};
    time_t GET_time;
    time(&GET_time);
    struct tm* p;
    p = gmtime(&GET_time);
    sprintf(Now_time, "[%02d月%02d日 %02d:%02d:%02d]", 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
    return Now_time;
}
void Log(const char *Info) {
    FILE *fp = fopen("Log.txt", "a");
    fprintf(fp,"%s",Info);
    fclose(fp);
}
// 12红色 9蓝色 14黄色 10绿色
// 系统消息
void system_Information(const char* Info) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
    cout << "\r";
    cout << Get_time() << " [SYSTEM]:" << Info << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    if (SYSTEM_Log == true) {
        // 因为不确定长度，所以用string
        string Log_Info;
        Log_Info.append(Get_time()).append(" ").append("[SYSTEM]:").append(Info).append("\n");
        Log(Log_Info.c_str());
    }
}
// 用户消息
void user_Information(const char* Sender, const char* Target, const char* Info) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
    cout << "\r";
    cout << Get_time() << " ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    cout << Sender << "-->" << Target << "|" << Info << endl;
    if (SYSTEM_Log == true) {
        string Log_Info;
        Log_Info.append(Get_time()).append(" ").append(Sender).append("-->").append(Target).append(Info).append("\n");
        Log(Log_Info.c_str());
    }
}
// 警告消息
void warn_Information(const char* Info) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
    cout << "\r";
    cout << Info << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    if (SYSTEM_Log == true) {
        string Log_Info;
        Log_Info.append(Info).append("\n");
        Log(Log_Info.c_str());
    }
}
void prompt_Information(const char* Info) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
    cout << "\r";
    cout << Info;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    if (SYSTEM_Log == true) {
        string Log_Info;
        Log_Info.append(Info).append("\n");
        Log(Log_Info.c_str());
    }
}
// 绿色成功消息
void success_Information(const char* Info) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
    cout << "\r";
    cout << Info << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    if (SYSTEM_Log == true) {
        string Log_Info;
        Log_Info.append(Info).append("\n");
        Log(Log_Info.c_str());
    }
}
// 本地错误
void error(int error) {
    if (error == 1) {
        system_Information("致命错误:WSAStartup失败");
        exit(0);
    }
    else if (error == 2) {
        system_Information("致命错误:创造监听套接字失败");
        exit(0);
    }
    else if (error == 3) {
        system_Information("致命错误:绑定到本地地址和端口失败");
        exit(0);
    }
    else if (error == 4) {
        system_Information("致命错误:监听失败");
        exit(0);
    }
    else if (error == 5) {
        system_Information("致命错误:设置非阻塞模式失败");
        exit(0);
    }
    else if (error == 6) {
        system_Information("接收新连接失败");
    }
    else if (error == 7) {
        system_Information("发送数据失败,取消连接");
    }
    else if (error == 8) {
        system_Information("输入的字数超限！");
    }
    else if (error == 9) {
        system_Information("接收信息失败，取消连接！");
    }
    else if (error == 10) {
        system_Information("发送信息失败，取消连接！");
    }
    else if (error == 11) {
        system_Information("致命错误，分配空间错误！");
        exit(0);
    }
}