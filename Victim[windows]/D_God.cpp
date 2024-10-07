// D_God.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include <windows.h>
int main()
{
	HMODULE hModule = LoadLibrary(TEXT("A:\\Project\\c\\DLL\\D_Server\\x64\\Release\\D_Server.dll"));// 加载动态库
}