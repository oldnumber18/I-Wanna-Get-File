#include "Server.h"
using namespace std;
vector<SUM_Info*> SUMArray;
// printf
char* GetTime() {
	static char NowTime[50] = { 0 };
	time_t GetTime;
	time(&GetTime);
	struct tm* p;
	p = gmtime(&GetTime);
	sprintf(NowTime, "[%02d月%02d日 %02d:%02d:%02d]", 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
	return NowTime;
}
void System_Info(const char* Info,bool BUFF) {
	if (BUFF) cout << "\033[34m" << GetTime() << "\033[32m" <<"[INFO]" << "\033[0m" << "|SYSTEM-->" << Info << endl;
	else cout << "\033[34m" << GetTime() << "\033[31m" << "[ERROR]" << "\033[0m" << "|SYSTEM-->" << Info << endl;
}
void User_Info(WHO* Goal, WHO* Sender, COMMAND* Command,char* Info) {
	cout << "\033[34m" << GetTime() << "\033[0m" << "[" << Sender->Num << "]" << Sender->Name << "=>" << "[" << Goal->Num << "]" << Goal->Name << "|" << "Type:" << Command->Type << " Command:" << Command->Command;
	if (!strcmp(Command->Type, "R")) {
		cout << " Info:" << Info;
	}
	cout << endl;
}
// dispose Command
bool ReturnDisposeEnd(int ReturnAddrNum,const char* Type,const char* Command,void* Info,signed int Size) {
	SUMSEND* SendInfo = &SUMArray.at(ReturnAddrNum)->SendInfo;
	strcpy(SendInfo->SendInfo.Who.Name, "SERVER");
	SendInfo->SendInfo.Who.Num = 0;
	strcpy(SendInfo->SendInfo.Command.Type, Type);
	strcpy(SendInfo->SendInfo.Command.Command, Command);
	memset(SendInfo->SendInfo.Info, 0, sizeof(SendInfo->SendInfo.Info));
	memcpy(SendInfo->SendInfo.Info, Info, Size);
	SendInfo->SendSize = Size+sizeof(WHO)+sizeof(COMMAND);
	SendInfo->IsSend = TRUE;
	return true;
}
char* _A_SOCKET(int ReturnAddrNum, char* ReturnArray) {
	if (ReturnArray == nullptr) return 0;
	int Addr = 0;
	for (int i = 0; i < SUMArray.size(); i++) {
		if (i != ReturnAddrNum) {
			memcpy(ReturnArray, &SUMArray.at(i)->ID_Info, sizeof(WHO));
			ReturnArray += sizeof(WHO);
		}
	}
	return ReturnArray;
}
void DisposeSendToServerCommand(int ReturnAddrNum) {
	CLIENTINFO* ClientInfo = (CLIENTINFO*)SUMArray.at(0)->ClientInfo.Info;
	if (!strcmp(ClientInfo->Command.Type, "A")) {
		if (!strcmp(ClientInfo->Command.Command, "SOCKET")) {
			static A_SOCKET A_Socket;
			A_Socket.Num = SUMArray.size()-1;
			memset(A_Socket.ClientSocketInfo, 0, sizeof(A_Socket.ClientSocketInfo));
			_A_SOCKET(ReturnAddrNum, A_Socket.ClientSocketInfo);
#ifdef _DEBUG
			WHO* a = nullptr;
			a = (WHO*)A_Socket.ClientSocketInfo;
			System_Info("返回内容:", true);
			for (int i = 0; i < A_Socket.Num; i++) {
				printf("[%d]%s\n", a->Num, a->Name);
				a++;
			}
#endif
			int Size = sizeof(WHO) * (SUMArray.size() -1) + sizeof(int);
			ReturnDisposeEnd(ReturnAddrNum, "A", "SOCKET", &A_Socket, Size);
		}
	}
	else if (!strcmp(ClientInfo->Command.Type, "R")) {

	}
	return;
}

// NetWork
void NewClient(SOCKET Socket, sockaddr_in SockAddr) {
	SUMArray.push_back((SUM_Info*)malloc(sizeof(SUM_Info)));
	SUM_Info* NewInfo = SUMArray.back();
	// 填写 NetWork
	NewInfo->NetWork.Socket = Socket;
	NewInfo->NetWork.SockAddr = SockAddr;
	// 填写ID
	NewInfo->ID_Info.Num = SUMArray.size()-1;
	char Name[20] = { 0 };
	memset(NewInfo->ID_Info.Name,0,sizeof(Name));
	sprintf(Name, "%s:%d", inet_ntoa(SockAddr.sin_addr), ntohs(SockAddr.sin_port));
	strcpy(NewInfo->ID_Info.Name, Name);
	// 填写发送
	NewInfo->SendInfo.IsSend = FALSE;
	// 接收
	memset(NewInfo->ClientInfo.Info, 0, sizeof(NewInfo->ClientInfo.Info));
	NewInfo->ClientInfo.DataBuf.buf = NewInfo->ClientInfo.Info;
	NewInfo->ClientInfo.DataBuf.len = sizeof(NewInfo->ClientInfo.Info);
	// 通知
	static A_SOCKET A_Socket;
	A_Socket.Num = SUMArray.size() - 1;
	for (int i = 1; i < SUMArray.size(); i++) {
		memset(A_Socket.ClientSocketInfo, 0, sizeof(A_Socket.ClientSocketInfo));
		_A_SOCKET(i, A_Socket.ClientSocketInfo);
		int Size = sizeof(WHO) * (SUMArray.size()-1) + sizeof(int);
		ReturnDisposeEnd(i, "A", "SOCKET", &A_Socket, Size);
	}
	char NewClientInfo[50] = { 0 };
	sprintf(NewClientInfo, "[%d]%s 已连接到服务器.", NewInfo->ID_Info.Num, Name);
	System_Info(NewClientInfo,true);
}
void DelClient(int Num) {
	// 通知
	char DelClientInfo[50] = { 0 };
	sprintf(DelClientInfo, "%s 已断开服务器.", SUMArray.at(Num)->ID_Info.Name);
	System_Info(DelClientInfo,true);
	// 处理
	free(SUMArray.at(Num));
	SUMArray.erase(SUMArray.begin() + Num);// 删除第 Num+1个
	// 通知2
	static A_SOCKET A_Socket;
	A_Socket.Num = SUMArray.size() - 1;
	for (int i = 1; i < SUMArray.size(); i++) {
		memset(A_Socket.ClientSocketInfo, 0, sizeof(A_Socket.ClientSocketInfo));
		_A_SOCKET(i, A_Socket.ClientSocketInfo);
		int Size = sizeof(WHO) * (SUMArray.size() - 1) + sizeof(int);
		ReturnDisposeEnd(i, "A", "SOCKET", &A_Socket, Size);
	}
}
int main()
{
	System_Info("正在进行初始化",true);

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		System_Info("致命错误:WSAStartup失败.", false);
		exit(0);
	}
	SOCKET tcp_Listen;
	SOCKET AcceptSocket = NULL;// 与客户端通信的套接字
	int clientAddersize = sizeof(sockaddr_in);
	if ((tcp_Listen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		System_Info("致命错误:创造监听套接字失败.",false);
		exit(0);
	}
	sockaddr_in clientAdder;// 客户端信息
	sockaddr_in serverAdder;// 服务器端信息
	serverAdder.sin_family = AF_INET;
	serverAdder.sin_port = htons(IP_PORT);
	serverAdder.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(tcp_Listen, (PSOCKADDR)&serverAdder, sizeof(serverAdder)) == SOCKET_ERROR) {
		System_Info("致命错误:绑定失败",false);
		exit(0);
	}
	if (listen(tcp_Listen, 10)) {
		System_Info("致命错误:监听失败.",false);
		exit(0);
	}
	u_long mode = 1;
	if (ioctlsocket(tcp_Listen, FIONBIO, &mode) == SOCKET_ERROR) {
		System_Info("致命错误:设置非阻塞式失败.",false);
		exit(0);
	}
	NewClient(tcp_Listen, serverAdder);

	system("title SERVER && cls");
	System_Info("初始化成功,开始运行.",true);
	FD_SET ReadSet;
	FD_SET WriteSet;
	while (TRUE) {
		FD_ZERO(&ReadSet);// 清空,初始化套接字状态
		FD_ZERO(&WriteSet);
		FD_SET(tcp_Listen, &ReadSet);// 添加
		for (int i = 0; i < SUMArray.size(); i++) {
			FD_SET(SUMArray.at(i)->NetWork.Socket, &ReadSet);
			FD_SET(SUMArray.at(i)->NetWork.Socket, &WriteSet);
		}
		if (select(0, &ReadSet, &WriteSet, NULL, NULL) == SOCKET_ERROR) {
			System_Info("致命错误:阻塞失败.",false);
			exit(0);
		}
		for (int i = 0; i < SUMArray.size(); i++) {
			SUM_Info* Socket_Info = SUMArray.at(i);
			if (FD_ISSET(Socket_Info->NetWork.Socket, &ReadSet)) {
				if (Socket_Info->NetWork.Socket == tcp_Listen) { // 新连接
					if ((AcceptSocket = accept(tcp_Listen, (struct sockaddr*)&clientAdder, &clientAddersize)) != INVALID_SOCKET) {
						u_long mode = 1;
						if (ioctlsocket(AcceptSocket, FIONBIO, &mode) == SOCKET_ERROR) {
							System_Info("致命错误:新连接设置非阻塞式失败.", false);
							exit(0);
						}
						NewClient(AcceptSocket, clientAdder);
					}
					else {
						System_Info("接收新连接失败.",false);
					}
				}
				else { // 接收消息
					memset(Socket_Info->ClientInfo.Info, 0, sizeof(Socket_Info->ClientInfo.Info));
					DWORD Flages = 0;
					if (WSARecv(Socket_Info->NetWork.Socket, &Socket_Info->ClientInfo.DataBuf, 1, &Socket_Info->ClientInfo.ClientSize, &Flages, NULL, NULL) == SOCKET_ERROR) {
						DelClient(i);
						continue;
					}
					else {
						if (Socket_Info->ClientInfo.ClientSize == 0) {
							DelClient(i);
							continue;
						}
						// 解析
						CLIENTINFO* ClientInfo = (CLIENTINFO*)&Socket_Info->ClientInfo.Info;
						// 确定转发给谁
						SUM_Info* GoalSocket = SUMArray.at(ClientInfo->Who.Num);
						strcpy(GoalSocket->SendInfo.SendInfo.Who.Name, Socket_Info->ID_Info.Name);
						GoalSocket->SendInfo.SendInfo.Who.Num = Socket_Info->ID_Info.Num;
						//转发
						memcpy(GoalSocket->ClientInfo.Info, Socket_Info->ClientInfo.Info, sizeof(Socket_Info->ClientInfo.Info));
						User_Info(&GoalSocket->ID_Info, &Socket_Info->ID_Info, &ClientInfo->Command, ClientInfo->Info);
						if (ClientInfo->Who.Num == 0) {
							DisposeSendToServerCommand(i);
							continue;
						}
					}
				}
			}
			else { // 可以发送消息
				if (FD_ISSET(Socket_Info->NetWork.Socket, &WriteSet)) {
					if (Socket_Info->SendInfo.IsSend == TRUE) {
						// 设置 WSASend 结构
						Socket_Info->SendInfo.DataBuf.len = Socket_Info->SendInfo.SendSize;
						Socket_Info->SendInfo.DataBuf.buf = (char*)&Socket_Info->SendInfo.SendInfo;
						int BUFF = WSASend(Socket_Info->NetWork.Socket, &Socket_Info->SendInfo.DataBuf, 1, &Socket_Info->SendInfo.SendSize, 0, NULL, NULL);
						Socket_Info->SendInfo.IsSend = FALSE;
						if (BUFF == SOCKET_ERROR) {
							printf("%d\n", WSAGetLastError());
							DelClient(i);
							continue;
						}
					}
				}
			}
		}
	}
}
