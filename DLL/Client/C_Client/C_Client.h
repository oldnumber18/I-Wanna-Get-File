#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <stdarg.h>
using namespace std;
#define IP_PORT 9999
#define IP_ADDR "127.0.0.1"
#define DATA_BUFMAX 4096
typedef struct _WHO {
    int Num;
    char Name[20];
}WHO;
typedef struct _COMMAND {
    char Type[2];
    char Command[10];
}COMMAND;
// _A_SOCKET 
typedef struct __A_SOCKET {
    int Num;
    char ClientSocketInfo[1024];
}A_SOCKET;

typedef struct _CLIENT { // JIEXI Structure
    WHO Who;
    COMMAND Command;
    char Info[];
}CLIENTINFO;

typedef struct _SENDINFO { // SendInfo Structure
    WHO Who;
    COMMAND Command;
    char Info[];
}SENDINFO;
