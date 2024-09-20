#pragma once
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#define DATA_BUFMAX 2048
#define DATA_BUFMIN 45
#define DATA_BUFMID 2000
#pragma warning(disable:4996)
// 导入头文件
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <vector>
using namespace std;
extern "C" {
	// 处理信息
	__declspec(dllexport) vector<char*>* ParseCommand(char* Type, char* Command, char* Parametric);
	__declspec(dllexport) BOOL FreeVectorArray();
}
