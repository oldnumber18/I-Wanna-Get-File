#include "server.h"
extern bool SYSTEM_Log;// ��־����
extern bool SYSTEM_Bell;// ����
extern int Socket_Num;
extern network Socket_Array[FD_SETSIZE];
// ˭���͹���
void Server_dispose(int Goal) {
	network Socket_Info = Socket_Array[Goal];
	SERVER* ClientInfo = (SERVER*)&Socket_Info->Info;
	if (Goal != 0) {
		user_Information(Socket_Info->Name, "SYSTEM", ClientInfo->Info);
	}
	char Send_Info[sizeof(Socket_Info->SendInfo.Info)] = {0};
	char Send_Type[2] = "I";
	char Command[10] = { 0 };
	if (strcmp(ClientInfo->Type, "I") == 0 && Goal != 0) {
		user_Information(Socket_Info->Name, "SYSTEM", ClientInfo->Info);
	}
	// L --> look up ����
	else if (strcmp(ClientInfo->Type, "L") == 0) {
		if (strcmp(ClientInfo->Info, "help") == 0) {

		}
		else if (strcmp(ClientInfo->Info, "clientlist") == 0) {
			if (Socket_Num == 1) {
				strcpy(Send_Info,"��ʱû�������ӵĿͻ���");
			}
			else {
				sprintf(Send_Info, "Ŀǰ������:%d", Socket_Num - 1);
				for (int i = 1; i < Socket_Num; i++) {
					char Info[50] = { 0 };
					sprintf(Info, "\n�� %d ������|%s", i, Socket_Array[i]->Name);
					strcat(Send_Info, Info);
				}
			}
		}
		else if (strcmp(ClientInfo->Info, "any") == 0) {
			// client log bell 
			if (SYSTEM_Log == TRUE) {
				strcpy(Send_Info,"1.��־�ѿ���");
			}
			else {
				strcpy(Send_Info, "1.��־�ѹر�");
			}
			if (SYSTEM_Bell == TRUE) {
				strcat(Send_Info, " 2.�����ѿ���");
			}
			else {
				strcat(Send_Info, " 2.�����ѹر�");
			}
		}
		else if (strcmp(ClientInfo->Info, "log") == 0) {
			if (SYSTEM_Bell == TRUE) sprintf(Send_Info,"��Ϣ��¼���ڿ���״̬");
			else sprintf(Send_Info, "��Ϣ��¼���ڹر�״̬");
		}
		else if (strcmp(ClientInfo->Info, "bell") == 0) {
			if (SYSTEM_Bell == TRUE) strcat(Send_Info, "��Ϣ�������ڿ���״̬");
			else strcat(Send_Info, "��Ϣ�������ڹر�״̬");
		}
		else {
			strcpy(Send_Info ,"������δָ֪�������Ҫ������ \"/help\"");
		}
	}
	else {
		strcpy(Send_Info, "������δָ֪�������Ҫ������ \"/help\"");
	}
	Send:
	if (Goal == 0) {
		if (Send_Info[0] != '\0') user_Information("SYSTEM", "SYSTEM", Send_Info);
	}
	else Radio_HOST(Goal,Send_Type, Command, Send_Info);
}