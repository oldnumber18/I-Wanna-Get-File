#include "server.h"
extern bool SYSTEM_Log;// 日志开关
extern bool SYSTEM_Bell;// 响铃
extern int Socket_Num;
extern network Socket_Array[FD_SETSIZE];
// 谁发送过来
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
	// L --> look up 查阅
	else if (strcmp(ClientInfo->Type, "L") == 0) {
		if (strcmp(ClientInfo->Info, "help") == 0) {

		}
		else if (strcmp(ClientInfo->Info, "clientlist") == 0) {
			if (Socket_Num == 1) {
				strcpy(Send_Info,"暂时没有已连接的客户端");
			}
			else {
				sprintf(Send_Info, "目前连接数:%d", Socket_Num - 1);
				for (int i = 1; i < Socket_Num; i++) {
					char Info[50] = { 0 };
					sprintf(Info, "\n第 %d 个连接|%s", i, Socket_Array[i]->Name);
					strcat(Send_Info, Info);
				}
			}
		}
		else if (strcmp(ClientInfo->Info, "any") == 0) {
			// client log bell 
			if (SYSTEM_Log == TRUE) {
				strcpy(Send_Info,"1.日志已开启");
			}
			else {
				strcpy(Send_Info, "1.日志已关闭");
			}
			if (SYSTEM_Bell == TRUE) {
				strcat(Send_Info, " 2.铃声已开启");
			}
			else {
				strcat(Send_Info, " 2.铃声已关闭");
			}
		}
		else if (strcmp(ClientInfo->Info, "log") == 0) {
			if (SYSTEM_Bell == TRUE) sprintf(Send_Info,"消息记录处于开启状态");
			else sprintf(Send_Info, "消息记录处于关闭状态");
		}
		else if (strcmp(ClientInfo->Info, "bell") == 0) {
			if (SYSTEM_Bell == TRUE) strcat(Send_Info, "消息铃声处于开启状态");
			else strcat(Send_Info, "消息铃声处于关闭状态");
		}
		else {
			strcpy(Send_Info ,"输入了未知指令，如有需要请输入 \"/help\"");
		}
	}
	else {
		strcpy(Send_Info, "输入了未知指令，如有需要请输入 \"/help\"");
	}
	Send:
	if (Goal == 0) {
		if (Send_Info[0] != '\0') user_Information("SYSTEM", "SYSTEM", Send_Info);
	}
	else Radio_HOST(Goal,Send_Type, Command, Send_Info);
}