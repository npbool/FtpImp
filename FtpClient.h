#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "mysocket.h"
#include "var.h"
class FtpClient{
private:
    TcpSocket sock;

    string pwd;

    static const int CMD_BUF_LEN = 128;
    char cmd_buf[CMD_BUF_LEN];

    string readResponse(int* res_code); //single line
    int sendCommand(const char* cmd);

    string cmd_args[3];
    int cmd_argc;
    void parse(string cmd){
        cmd_argc = 0;
        int st = 0;
        for(int i=0;i<cmd.length();++i){
            if(cmd[i]==' ' || cmd[i]=='\n'){
                cmd_args[cmd_argc++] = cmd.substr(st,i-st);
                st = i+1;
                if(cmd_argc==3) return;
            }
        }
        if(cmd[cmd.length()-1] != '\n'){
            cmd_args[cmd_argc++] = cmd.substr(st,cmd.length()-st);
        }
    }

    TcpSocket enterPassiveMode();

public:
    int connectToHost(const char* ip,int port,const char* username,const char* password);
    int processInput();
};

#endif // FTPCLIENT_H
