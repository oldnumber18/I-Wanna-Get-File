#pragma warning(disable:4996)
#include "server.h"
extern bool SYSTEM_Log;// ��־����
extern bool SYSTEM_Bell;// ����
// ʱ��
std::string Get_time() {
    char Now_time[28] = {0};
    time_t GET_time;
    time(&GET_time);
    struct tm* p;
    p = gmtime(&GET_time);
    sprintf(Now_time, "[%02d��%02d�� %02d:%02d:%02d]", 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
    return Now_time;
}
void Log(const char *Info) {
    FILE *fp = fopen("Log.txt", "a");
    fprintf(fp,"%s",Info);
    fclose(fp);
}
// 12��ɫ 9��ɫ 14��ɫ 10��ɫ
// ϵͳ��Ϣ
void system_Information(const char* Info) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
    cout << "\r";
    cout << Get_time() << " [SYSTEM]:" << Info << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    if (SYSTEM_Log == true) {
        // ��Ϊ��ȷ�����ȣ�������string
        string Log_Info;
        Log_Info.append(Get_time()).append(" ").append("[SYSTEM]:").append(Info).append("\n");
        Log(Log_Info.c_str());
    }
}
// �û���Ϣ
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
// ������Ϣ
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
// ��ɫ�ɹ���Ϣ
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
// ���ش���
void error(int error) {
    if (error == 1) {
        system_Information("��������:WSAStartupʧ��");
        exit(0);
    }
    else if (error == 2) {
        system_Information("��������:��������׽���ʧ��");
        exit(0);
    }
    else if (error == 3) {
        system_Information("��������:�󶨵����ص�ַ�Ͷ˿�ʧ��");
        exit(0);
    }
    else if (error == 4) {
        system_Information("��������:����ʧ��");
        exit(0);
    }
    else if (error == 5) {
        system_Information("��������:���÷�����ģʽʧ��");
        exit(0);
    }
    else if (error == 6) {
        system_Information("����������ʧ��");
    }
    else if (error == 7) {
        system_Information("��������ʧ��,ȡ������");
    }
    else if (error == 8) {
        system_Information("������������ޣ�");
    }
    else if (error == 9) {
        system_Information("������Ϣʧ�ܣ�ȡ�����ӣ�");
    }
    else if (error == 10) {
        system_Information("������Ϣʧ�ܣ�ȡ�����ӣ�");
    }
    else if (error == 11) {
        system_Information("�������󣬷���ռ����");
        exit(0);
    }
}