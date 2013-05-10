#include "FtpServer.h"

void FtpServer::run(){
    server.bindAndListen(TEST_PORT);
    ctrlSock = server.getConn();
    if(processLogin()<0){
        printf("user login failed");
        ctrlSock.close();
        return;
    }
    string cmd;
    int res_code;
    while(1){
        cmd = ctrlSock.readLine(&res_code);
        if(res_code<0){
            perror("read command failed");
            ctrlSock.close();
            return;
        }
        parse(cmd);
        if(cmd_args[0]=="PWD"){
            sprintf(cmd_buf,"257 \"%s\"\r\n",pwd.c_str());
            ctrlSock.sendString(cmd_buf);
        } else if(cmd_args[0]=="CWD"){
            if(chdir(cmd_args[1].c_str())<0){
                ctrlSock.sendString("500 CWD Failed\r\n");
            } else {
                ctrlSock.sendString("250 Directory successfully changed.\r\n");
            }
        } else if(cmd_args[0]=="PASV"){
            passive = true;
            if(tmpServer!=NULL){
                tmpServer->close();
                delete tmpServer;
            }

            tmpServer = new TcpServer();
            unsigned int tmpPort = rand();
            while(tmpServer->bindAndListen(tmpPort)<0){
                tmpPort = rand();
            }
            sprintf(cmd_buf,"227 Entering Passive Mode(127,0,0,1,%d,%d)\r\n",tmpPort>>8,tmpPort&0xFF);
            ctrlSock.sendString(cmd_buf);
        } else if(cmd_args=="LIST"){
            TcpSocket tmpSocket = tmpServer->getOneConn();

            string output;
            if(cmd_argc==1){
                output = execShellCmd("ls -alh",&res_code);
            } else {
                if(argv[1][0]=='/') argv[1] = root_dir+argv[1];
                sprintf(cmd_buf,"ls -alh %s",argv[1].)
                output = execShellCmd(cmd_buf,&res_code);
            }

            if(res_code==-1){
                ctrlSock.sendString("500 Some error >_<\r\n");
            } else {
                ctrlSock.sendString("150 Here comes the directory listing.\r\n");
                tmpSocket.sendString(res.c_str);
                ctrlSock.sendString("226 Directory send OK.\r\n");
            }
            tmpSocket.close();
        } else if(cmd_args=="get"){
        } else if(cmd_args=="put"){
        } else {
            printf("not implemented\n");
            printf(cmd_args[0].c_str());
            ctrlSock.sendString("502 Command not implemented\r\n");
        }
    }

}

int FtpServer::processLogin(){
    string response;
    int res_code;

    ctrlSock.sendString("220 (npbool's ftp 3.1415926...)\r\n");
    response = ctrlSock.readLine(&res_code);
    parse(response);
    string username = cmd_args[1];
    cout<<"username:"<<username<<endl;

    ctrlSock.sendString("331 Please specify the password.\r\n");
    response = ctrlSock.readLine(&res_code);
    parse(response);
    string password = cmd_args[1];
    cout<<"password:"<<password<<endl;

    if(username=="npbool" && password=="111"){
        ctrlSock.sendString("230 Login successful.\r\n");
        login = true;
    }
    return 0;
}

void FtpServer::parse(string cmd){
    cmd_argc = 0;
    int st = 0;
    for(int i=0;i<cmd.size()-1;++i){
        if(cmd[i]==' ' || cmd[i]=='\r'){
            cmd_args[argi++] = cmd.substr(st,i-st);
            st = i+1;
            if(argi==3) return;
        }
    }
}

int main(){
    FtpServer s;
    s.run();
}
