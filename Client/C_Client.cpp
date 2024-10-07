#include "C_Client.h"
vector<WHO> SocketArray;
// printf
char* GetTime() {
    time_t GetTime;
    time(&GetTime);
    static char Time[50] = { 0 };
    tm* p = localtime(&GetTime);
    sprintf(Time,"[%02d:%02d:%02d]",p->tm_hour,p->tm_min,p->tm_sec);
    return Time;
}
void SystemPrintLog(const char* Info,bool BUFF) {
    cout << "\r";
    if (BUFF) cout << "\033[34m" << GetTime() << "\033[32m" << "[INFO]" << "\033[0m" << "|SYSTEM-->" << Info << endl;
    else cout << "\033[34m" << GetTime() << "\033[31m" << "[ERROR]" << "\033[0m" << "|SYSTEM-->" << Info << endl; 
}
// dispose Command from Server
bool _A_SOCKET(vector<WHO>* NowServerSocketArray, const A_SOCKET* Info) {
    NowServerSocketArray->clear();
    WHO* GetSocketArray = (WHO*)Info->ClientSocketInfo;
    WHO Who;
    for (int i = 0; i < Info->Num; i++) {
        memcpy(&Who,GetSocketArray,sizeof(WHO));
        NowServerSocketArray->push_back(Who);
        GetSocketArray++;
    }
    return true;
}
int CPUInfo(CLIENTINFO* ClientInfo,int Socket) {
    //printf("[%d]%s:Type:%s Command:%s Info:%s\n",ClientInfo->Who.Num,ClientInfo->Who.Name,ClientInfo->Command.Type,ClientInfo->Command.Command,ClientInfo->Info);
    if ((!strcmp(ClientInfo->Command.Type, "A")) && ClientInfo->Who.Num == 0) {
        if (!strcmp(ClientInfo->Command.Command, "SOCKET")) {
            _A_SOCKET(&SocketArray, (A_SOCKET*)ClientInfo->Info);
            SystemPrintLog("Client Server Socket Information Synchronous Success:",true);
            int SumSocketNumber = SocketArray.size();
            for (int i = 0; i < SumSocketNumber; i++){
                printf("[%d]%s\n", SocketArray.at(i).Num, SocketArray.at(i).Name);
            }
            printf("Sum Client Socket Number:%d\n",SumSocketNumber);
            return true;
        }
    }
    else if (!strcmp(ClientInfo->Command.Type, "R")) {
        printf("%s%s-->this:%s",GetTime(),ClientInfo->Who.Name,ClientInfo->Info);
        return true;
    }
    return false;
}
int TellServerID(int Socket){
    SENDINFO SendInfo;
    SendInfo.Who.Num = 0;
    strcpy(SendInfo.Who.Name,"SERVER");
    strcpy(SendInfo.Command.Type, "A");
    strcpy(SendInfo.Command.Command,"SOCKET");
    int BUFF = send(Socket,(char*)&SendInfo,sizeof(SENDINFO) + strlen(SendInfo.Info),0);
    return BUFF;
}
// dispose Input and Send
std::tuple<int,int> SetSendGoal(char* Info,int InfoSize) {
    int ReturnInt = -1;
    int ReturnChar = -1;
    for (int i = 0 ; i < InfoSize;i++) {
        if (65 <= Info[i] && Info[i] <= 90) {
            ReturnChar = Info[i];
        }
        else if (48 <= Info[i] && Info[i] <= 57) {
            ReturnInt = Info[i] - 48;
            i++;
            while(48 <= Info[i] && Info[i] <= 57) {
                ReturnInt = ReturnInt*10 + Info[i] - 48;
                i++;
            }
            i--;
        }
        else {
            SystemPrintLog("Input have error",false);
        }
    }
    return std::make_tuple(ReturnInt,ReturnChar);
}
//disposed Input
std::string InputInformation(string ReturnArray, char* PrintfInfo) {
    ReturnArray.clear();
    struct termios orig_termios;
    int ch = 0;

    tcgetattr(STDIN_FILENO,&orig_termios);
	struct termios new_termios = orig_termios;
	new_termios.c_lflag &= ~(ICANON | ECHO);
	new_termios.c_cc[VTIME] = 0;
	new_termios.c_cc[VMIN] = 1;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    while (1) {
        ch = getchar();
        if (32 <= ch && ch <= 126) {
            ReturnArray.push_back(ch);
            cout << "\r" << PrintfInfo << ReturnArray;
        }
        else if (ch == 10) {
            if (ReturnArray.size() == 0) {
                SystemPrintLog("Can't not find space.",false);
                continue;
            }
            break;
        }
        else if (ch == 8 || ch == 127) {
            if (ReturnArray.size() != 0) {
                printf("\b ");
                ReturnArray.pop_back();
                cout << "\r" << PrintfInfo << ReturnArray;
            }
        }
        else {
            continue;
        }
    }
    cout << endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    return ReturnArray;
}
void InputAndSendInfo(int Socket) {// Send Goal
    int SendGoal = 0;
    char Type[2] = { "R" };
    string GetInputInfo;
    char PrintInfo1[100] = { 0 };
    int spacelocation = 0;
    while(1){
        if (SocketArray.size() != 0) break;
        else sleep(3);
    }
    while (1) {
        sprintf(PrintInfo1, "[%s]this-->[%d]%s:", Type, SocketArray.at(SendGoal).Num, SocketArray.at(SendGoal).Name);
        GetInputInfo = InputInformation(GetInputInfo, PrintInfo1);
        if ((spacelocation = GetInputInfo.find_first_of(' ')) != string::npos) {
            if (spacelocation < 10) {
                if (GetInputInfo.c_str()[0] == '/') {
                    if (!strncmp(GetInputInfo.c_str(), "/help", sizeof("/help")-1)) {
                       SystemPrintLog("Can't understand Code?",true);
                    }
                    else if (!strncmp(GetInputInfo.c_str(),"/set",sizeof("/set")-1)) {
                        std::tuple<int,int> result = SetSendGoal((char*)GetInputInfo.substr(spacelocation+1).c_str(), GetInputInfo.size() - sizeof("/set"));
                        int ReturnInt = std::get<0>(result);
                        if (ReturnInt != -1) {
                            if (0 <= ReturnInt && ReturnInt < SocketArray.size()) {
                                SendGoal = ReturnInt;
                                char Info[50] = { 0 };
                                sprintf(Info, "New Send Goal: [%d]%s",SendGoal,SocketArray.at(SendGoal).Name);
                                SystemPrintLog(Info, true);
                            }
                            else {
                                SystemPrintLog("Input Send Goal Number error",false);
                            }
                        }
                        int ReturnChar = std::get<1>(result);
                        if (ReturnChar != -1) {
                            sprintf(Type, "%c", ReturnChar);
                            char Info[50] = { 0 };
                            sprintf(Info, "New Send Type: %s", Type);
                            SystemPrintLog(Info,true);
                        }
                    }
                    else {
                        SystemPrintLog("Understand Command.",false);
                    }
                    continue;
                }
                else {
                    unsigned SendSize = sizeof(SENDINFO) + sizeof(char) * (GetInputInfo.size() - spacelocation);
                    SENDINFO* SendInfo = (SENDINFO*)malloc( SendSize);

                    strcpy(SendInfo->Command.Command, GetInputInfo.substr(0,spacelocation).c_str());
                    strcpy(SendInfo->Command.Type, Type); 
                    strcpy(SendInfo->Info, GetInputInfo.substr(spacelocation+1).c_str());
                    SendInfo->Who.Num = SendGoal;
                    strcpy(SendInfo->Who.Name,SocketArray.at(SendGoal).Name);
                    int BUFF = send(Socket, SendInfo, SendSize, 0);
                    free(SendInfo);
                    if (BUFF < 0) return;
                    else {
                        SystemPrintLog("Send Complate.", true);
                    }
                }
            }
            else {
                SystemPrintLog("please look your input space loction.",false);
            }
        }
        else {
            SystemPrintLog("can't not find space.",false);
        }
    }
    return;
}
int main() {
    int Socket;
    if ((Socket = socket(AF_INET, SOCK_STREAM, 6)) < 0) {
        SystemPrintLog("Create Socket Error", false);
        close(Socket);
        return -1;
    }
    struct sockaddr_in Sockaddr;
    memset(&Sockaddr,0,sizeof(Sockaddr));
    Sockaddr.sin_family = AF_INET;
    Sockaddr.sin_port = htons(IP_PORT);
    Sockaddr.sin_addr.s_addr = inet_addr("192.168.1.18");

    if (connect(Socket,(struct sockaddr*)&Sockaddr,sizeof(Sockaddr)) < 0){
        SystemPrintLog("Client to Server Failed.", false);
        close(Socket);
        return -1;
    }
    { // Send ID to Server
        SENDINFO SendInfo;
        SendInfo.Who.Num = 0;
        strcpy(SendInfo.Who.Name,"SERVER");
        strcpy(SendInfo.Command.Type, "A");
        strcpy(SendInfo.Command.Command,"SOCKET");
        int BUFF = send(Socket,(char*)&SendInfo,sizeof(SENDINFO),0);
        if (!(BUFF > 0)) {
            SystemPrintLog("Send Server Info Error", false);
            close(Socket);
            return -1;
        }
    }
    SystemPrintLog("Client Server Success.", true);
    
    //1.Input and Send Info 2.GetInfo
    std::thread GetInfoThread(InputAndSendInfo,Socket);
    GetInfoThread.detach();
    char GetInfo[DATA_BUFMAX] = { 0 };
    while (recv(Socket, GetInfo, sizeof(GetInfo), 0) > 0) {
        int BUFF = CPUInfo((CLIENTINFO*)GetInfo, Socket);
    }
    SystemPrintLog("Can't Read from Server Info,Client quit",false);
    close(Socket);
    return -1;
}
