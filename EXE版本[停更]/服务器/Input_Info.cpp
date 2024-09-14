#include "server.h"
extern int Socket_Num;
extern network Socket_Array[FD_SETSIZE];
extern bool SYSTEM_Log;// 日志开关
extern bool SYSTEM_Bell;// 响铃
int Input_Goal = 0;
void* Input(void *arg) {
	// 初始化
	char Type[2] = "I";// 类型
	while (1) {
		start:
		network Socket_Info = nullptr;
		Socket_Info = Socket_Array[Input_Goal];
		if (Socket_Info == nullptr) {
			Input_Goal = 0;
			Socket_Info = Socket_Array[Input_Goal];
			system_Information("指向错误，自动调整目标为服务器");
			continue;
		}
		// 输入刷新
		char Get_Info[sizeof(Socket_Info->SendInfo.Info)] = {0};
		char Get_Command[10] = {0};
		char Input_ID[50] = {0};
		sprintf(Input_ID, "<SYSTEM>%s-->%s:", Type, Socket_Info->Name);
		int Input_size = 0;// 记录输入的数组位置
		while (1) {
			char Input_char = '\0';
			Input_char = getch();
			if (Input_char != 13 && Input_char != 8) {
				Get_Info[Input_size] = Input_char;
				Input_size++;
				//printf("\r%s%s", Input_ID, Get_Info);
				cout << "\r" << Input_ID << Get_Info;
			}
			else if (Input_char == 8) {// 回退键
				if (Input_size == 0) continue;
				cout << "\r";
				for (int i = 0; i <= Input_size+strlen(Input_ID); i++) cout<<" ";
				Input_size--;
				Get_Info[Input_size] = '\0';
				cout << "\r" << Input_ID << Get_Info;
				//printf("\r%s%s", Input_ID, Get_Info);
			}
			else {
				if (Input_size == 0) {
					system_Information("你是否真的输入了消息？:)");
					continue;
				}

				cout << endl;
				break;
			}
		}
		if (Get_Info[0] == '/') {
			// 输入了系统预购指令
			if (strncmp(Get_Info, "/help", sizeof("/help")) == 0) {
				warn_Information("自己不会看代码???");
				continue;
			}
			else if (strncmp(Get_Info, "/look", sizeof("/look")-1) == 0) {
				if (strcmp(Get_Info, "/look client") == 0) {
					string Info;
					Info.append("现在一共有 ");
					Info += to_string(Socket_Num-1);
					Info += " 个已连接\n";
					for (int i = 1; i < Socket_Num; i++) {
						network Socket_Name = Socket_Array[i];
						Info.append("第 ").append(to_string(i)).append(" 个|").append(Socket_Name->Name).append("\n");
					}
					cout << Info;
				}
				else {
					warn_Information("输入了未知指令，如有需要请输入 \"/help\"");
				}
			}
			else if (strncmp(Get_Info, "/set", sizeof("/set")-1) == 0) {
				int Get_Info_size = strlen(Get_Info);
				int Sendsure = false;
				for (int i = sizeof("/set"); i < Get_Info_size; i++) {
					if (65 <= Get_Info[i] && Get_Info[i] <= 90) {
						sprintf(Type, "%c", Get_Info[i]);
						char Info[30] = { 0 };
						Sendsure = true;
						sprintf(Info, "类型已更改为 %c", Get_Info[i]);
						success_Information(Info);
						continue;
					}
					if (48 <= Get_Info[i] && Get_Info[i] <= 57) {
						int NewGoal = Get_Info[i] - 48;
						Sendsure = true;
						i++;
						while (48 <= Get_Info[i] && Get_Info[i] <= 57) {
							NewGoal = NewGoal * 10 + (Get_Info[i] - 48);
							i++;
						}
						if (NewGoal < Socket_Num) {
							Input_Goal = NewGoal;
							char Info[30] = { 0 };
							sprintf(Info, "目标已更改为 %d", NewGoal);
							success_Information(Info);
						}
						else {
							warn_Information("修改目标失败！");
						}
					}
				}
				if (Sendsure == false) {
					warn_Information("输入了未知指令，如有需要请输入 \"/help\"");
				}
			}
			else if (strncmp(Get_Info, "/system", sizeof("/system")-1) == 0) {
				if (strcmp(Get_Info, "/system log") == 0) {
					if (SYSTEM_Log == true) {
						SYSTEM_Log = false;
						success_Information("系统日志已关闭");
					}
					else {
						SYSTEM_Log = true;
						success_Information("系统日志已开启");
					}
					System();
				}
				else if (strcmp(Get_Info, "/system bell") == 0) {
					if (SYSTEM_Bell == true) {
						SYSTEM_Bell = false;
						success_Information("系统消息提示已关闭");
					}
					else {
						SYSTEM_Bell = true;
						success_Information("系统消息提示已开启");
					}
					System();
				}
				else warn_Information("输入了未知指令，如有需要请输入 \"/help\"");
			}
			else {
				warn_Information("输入了未知指令，如有需要请输入 \"/help\"");
			}
			continue;
		}
		else {
			// 间隔区分
			int space = 0;
			for (int i = 0; i < Input_size && i <=10; i++) {
				if (Get_Info[i] == ' ') {
					space = i;
					break;
				}
			}
			if (space != 0) {
				memcpy(Get_Command, Get_Info, space);
				strncpy_s(Get_Info, Get_Info+space+1, Input_size - space);
			}
		}
		if (Input_Goal == 0) {
			Server_dispose(0);
			continue;
		}
		else {
			if (strcmp(Get_Command, "RecvFile") == 0 && strcmp(Type, "F") == 0) {
				FILE* fp = fopen(Get_Info, "rb");
				if (fp == NULL) {
					system_Information("open file error!");
					continue;
				}
				else {
					fseek(fp, 0, SEEK_END);
					int fp_Size = ftell(fp);
					rewind(fp);
					char* p;
					strcpy(Get_Info, (p = strrchr(Get_Info, '\\')) ? p + 1 : Get_Info);
					//发送文件信息
					Socket_Info->SendInfo.Size = fp_Size;
					strcpy(Socket_Info->SendInfo.Type, "F");
					strcpy(Socket_Info->SendInfo.Command, "FileInfo");
					Socket_Info->SendInfo.Who.Num = 0;
					strcpy(Socket_Info->SendInfo.Who.Name, "SYSTEM");
					strcpy(Socket_Info->SendInfo.Info, Get_Info);
					Socket_Info->Sure = TRUE;
					char Info[500] = { 0 };
					sprintf(Info,"Will Send File。Name:%s Size:%d Bites\n", Socket_Info->SendInfo.Info, Socket_Info->SendInfo.Size);
					prompt_Information(Info);
					for (int i = 0; i < 500 && Socket_Info->SaveFile.RecvFile != TRUE;i++) {
						Sleep(10);
						prompt_Information("Wait Send File...");
						if (i == 499) {
							warn_Information("对方接收文件信息失败！将退出发送");
							goto start;
						}
					}
					// 发送文件
					strcpy(Socket_Info->SendInfo.Command, "RecvFile");
					int Sendsize = 0;
					while (!feof(fp)) {
						for (int i = 0; i < 50 && Socket_Info->SaveFile.RecvFile != TRUE; i++) {
							Sleep(100);
							char Info[500] = { 0 };
							if (i == 49) {
								warn_Information("\n对方接收文件内容失败！将退出发送");
								goto start;
							}
						}
						memset(Socket_Info->SendInfo.Info, 0, sizeof(Socket_Info->SendInfo.Info));
						Socket_Info->SendInfo.Size = fread(Socket_Info->SendInfo.Info, sizeof(char), sizeof(Socket_Info->SendInfo.Info), fp);
						Sendsize += Socket_Info->SendInfo.Size;
						Socket_Info->SaveFile.RecvFile = FALSE;
						Socket_Info->Sure = TRUE;
						sprintf(Info, "总大小:%d 已发送:%d Bites\r", fp_Size, Sendsize);
						prompt_Information(Info);
					}
					Socket_Info->SaveFile.RecvFile = FALSE;
					Socket_Info->SaveFile.fp = NULL;
					Socket_Info->SaveFile.FileSize = 0;
					Socket_Info->SaveFile.GetSize = 0;
					
					success_Information("\nSend File complete");
					fclose(fp);
					strcpy(Type, "I");
					strcpy(Get_Info, "Send File sucess.");
				}
			}
			if (strcmp(Get_Command, "path") == 0 && strcmp(Type, "L") == 0) {
				if (Get_Info[Input_size] == '\\' || Get_Info[Input_size] == '/') Get_Info[Input_size] = '\0';
			}
			Radio_CLIENT(Input_Goal,Type,Get_Command,Get_Info);
		}
	}
	return (void*)1;
}