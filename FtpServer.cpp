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
            ctrlSock.close();
            printf("Client quit. Exit. \n");
            exit(0);
        }
        cout<<"Client Command:"<<cmd;
        parse(cmd);
        if(cmd_args[0]=="PWD"){
            char pwd[256];
            sprintf(cmd_buf,"257 \"%s\"\r\n",getcwd(pwd,256));
            ctrlSock.sendString(cmd_buf);
        } else if(cmd_args[0]=="CWD"){
            if(chdir(cmd_args[1].c_str())<0){
                ctrlSock.sendString("500 CWD Failed\r\n");
            } else {
                ctrlSock.sendString("250 Directory successfully changed.\r\n");
            }
        } else if(cmd_args[0]=="PASV"){
            if(tmpServer!=NULL){
                tmpServer->close();
                delete tmpServer;
                tmpServer = 0;
            }

            tmpServer = new TcpServer();
            unsigned int tmpPort = rand()%55535+10000;
            while(tmpServer->bindAndListen(tmpPort)<0){
                tmpPort = rand()%55535+10000;
            }
            cout<<"PASSIVE mode: open port "<<tmpPort<<endl;
            sprintf(cmd_buf,"227 Entering Passive Mode(127,0,0,1,%d,%d)\r\n",tmpPort>>8,tmpPort&0xFF);
            ctrlSock.sendString(cmd_buf);
            passive = true;
        } else if(cmd_args[0]=="LIST"){
            TcpSocket tmpSocket = tmpServer->getOneConn();

            string output;
            if(cmd_argc==1){
                output = execShellCmd("ls -lh",&res_code);
            } else {
                if(cmd_args[1][0]=='/') cmd_args[1] = root_dir+cmd_args[1];
                sprintf(cmd_buf,"ls -alh %s",cmd_args[1].c_str());
                output = execShellCmd(cmd_buf,&res_code);
            }

            if(res_code==-1){
                ctrlSock.sendString("500 Some error >_<\r\n");
            } else {
                ctrlSock.sendString("150 Here comes the directory listing.\r\n");
                tmpSocket.sendString(output.c_str());

                tmpServer->close();
                delete tmpServer;
                tmpServer = 0;
                tmpSocket.close();

                ctrlSock.sendString("226 Directory send OK.\r\n");
            }

            passive = false;
        } else if(cmd_args[0]=="RETR"){
            const char* file_name = cmd_args[1].c_str();
            unsigned long file_size = -1;
            struct stat statinfo;
            if(stat(file_name,&statinfo)<0){
                ctrlSock.sendString("550 File not exist\r\n");
                return;
            }

            file_size = statinfo.st_size; 
            if(!passive){
            } else {
                sprintf(cmd_buf,"150 Opening BINARY mode data connection for %s (%ld bytes).\r\n",file_name,file_size);
                ctrlSock.sendString(cmd_buf);

                TcpSocket tmpSocket = tmpServer->getOneConn();
                const int READ_BUF_SIZE=1024;
                char buf[READ_BUF_SIZE];
                FILE* file = fopen(file_name,"r");
                if(file==NULL){
                    printf("Can't open file %s for reading!\n",file_name);
                }
                int read_size = 0;
                int total_size = 0;
                while((read_size=fread(buf,1,READ_BUF_SIZE,file))>0){
                    tmpSocket.send(buf,read_size);
                    total_size+=read_size;
                }
                printf("Transfer %d bytes\n",total_size);
                tmpServer->close();
                delete tmpServer;
                tmpServer = 0;
                tmpSocket.close();
                ctrlSock.sendString("226 Transfer complete.\r\n");

                passive = false;
            }

        } else if(cmd_args[0]=="STOR"){
            const char* file_name = cmd_args[1].c_str();
            if(passive){
                ctrlSock.sendString("150 Ok to send data.\r\n");

                TcpSocket tmpSocket = tmpServer->getOneConn();
                const int READ_BUF_LEN=1024;
                char buf[READ_BUF_LEN];
                FILE* file = fopen(file_name,"w");
                if(file==NULL){
                    printf("Can't open file %s for writing!\n",file_name);
                }
                int recv_len = 0;
                int total_size = 0;
                while((recv_len = tmpSocket.recv(buf,READ_BUF_LEN))>0){
                    fwrite(buf,1,recv_len,file);
                    total_size += recv_len;
                }
                fclose(file);
                tmpSocket.close();
                tmpServer->close();
                delete tmpServer;
                tmpServer = 0;
                passive = false;

                ctrlSock.sendString("226 Transfer complete.\r\n");
            }
        } else if(cmd_args[0]=="QUIT"){
            ctrlSock.close();
            printf("Client quit. Exit.\n");
            exit(0);
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
            cmd_args[cmd_argc++] = cmd.substr(st,i-st);
            st = i+1;
            if(cmd_argc==3) return;
        }
    }
}

int main(){
    FtpServer s;
    s.run();
}
