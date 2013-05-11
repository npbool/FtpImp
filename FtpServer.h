#ifndef FTPSERVER_H
#define FTPSERVER_H

#include "mysocket.h"
#include <map>
using std::map;
#include "var.h"
class FtpServer
{
private:
    TcpServer server;
    TcpSocket ctrlSock; //使用Fork多进程模型，每个FtpServer只服务一个client
    TcpServer* tmpServer;

    static const int CMD_BUF_LEN=256;
    char cmd_buf[CMD_BUF_LEN];

    bool binaryMode;
    bool passive;
    bool login;
    string pwd;
    string root_dir;

    int cmd_argc;
    string cmd_args[3];
    map<string,string> accounts;
    void parse(string cmd);
public:
    FtpServer(){
        login = false;
        passive = false;
        root_dir = "/home/npbool/ftp";
        pwd = "/";
        tmpServer=0;
        binaryMode=true;
        chdir(root_dir.c_str());

        accounts["npbool"]="111";
        accounts["iwnking"]="222";
    }
    void run(int port);
    int processLogin();
};

#endif // FTPSERVER_H
