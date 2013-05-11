#include "FtpClient.h"

string FtpClient::readResponse(int* res_code){
    return sock.readLine(res_code);
}

int FtpClient::sendCommand(const char* cmd){
    printf("Command:%s",cmd);
    return sock.send(cmd,strlen(cmd));
}

int FtpClient::connectToHost(const char* ip,int port,const char* uname,const char* passwd){
    string response;
    int res_code;
    if(sock.connectToServ(ip,port)<0){
        perror("Can't connect to ftp server\n");
        return -1;
    }

    response = readResponse(&res_code);

    sprintf(cmd_buf,"USER %s\r\n",uname);
    sendCommand(cmd_buf);

    response = readResponse(&res_code);
    if(response[0] != '3'){
        return -1;
    }

    sprintf(cmd_buf,"PASS %s\r\n",passwd);
    sendCommand(cmd_buf);
    response = readResponse(&res_code);
    if(response[0] != '2' ){
        printf("Login failed\n");
        return -1;
    }

    return 0;
}

const char* helpStr = "Command provided:"
"get\n"
"put\n"
"pwd\n"
"dir\n"
"cd\n"
"?"
"quit";



int FtpClient::processInput(){
    string line;
    cout<<"FTP< ";
    while(getline(cin,line)){
        string res;
        int res_code;
        parse(line);
        if(cmd_args[0]=="pwd"){
            sendCommand("PWD\r\n");
            res = readResponse(&res_code);
            cout<<res;
        } else if(cmd_args[0]=="dir"){
            TcpSocket tmpSock = enterPassiveMode();
            sendCommand("LIST\r\n");
            res = readResponse(&res_code);
            cout<<res<<endl;
            while(true){
                res = tmpSock.readLine(&res_code);
                cout<<res;
                if(res_code<0)
                    break;
            }
            res = readResponse(&res_code);
            cout<<endl<<res;
        } else if(cmd_args[0]=="cd"){
            sprintf(cmd_buf,"CWD %s\r\n",cmd_args[1].c_str());
            sendCommand(cmd_buf);
            res = readResponse(&res_code);
            cout<<res;
        } else if(cmd_args[0]=="delete"){
            sprintf(cmd_buf,"DELE %s\r\n",cmd_args[1].c_str());
            sendCommand(cmd_buf);
            cout<<readResponse(&res_code);
        } else if(cmd_args[0]=="mkdir"){
            sprintf(cmd_buf,"MKD %s\r\n",cmd_args[1].c_str());
            sendCommand(cmd_buf);
            cout<<readResponse(&res_code);
        } else if(cmd_args[0]=="rename"){
            sprintf(cmd_buf,"RNFR %s\r\n",cmd_args[1].c_str());
            sendCommand(cmd_buf);
            res = readResponse(&res_code);
            cout<<res;
            if(res[0]=='3'){
                sprintf(cmd_buf,"RNTO %s\r\n",cmd_args[2].c_str());
                sendCommand(cmd_buf);
                cout<<readResponse(&res_code);
            }
        } else if(cmd_args[0]=="get"){
            TcpSocket tmpSocket = enterPassiveMode();

            const char* file_name = cmd_args[1].c_str();
            sprintf(cmd_buf,"RETR %s\r\n",file_name);
            sendCommand(cmd_buf);
            res = readResponse(&res_code);
            cout<<res;
            if(res[0]=='1'){
                printf("Receving file %s\n",file_name);
                int total_size = tmpSocket.recvFile(file_name);
                if(total_size>=0){
                    printf("Receive %d bytes\n",total_size);
                } else {
                    printf("Can't open file %s for writing!\n",file_name);
                }
                res = readResponse(&res_code);
                cout<<res;
            }
            tmpSocket.close();
        } else if(cmd_args[0]=="put"){
            TcpSocket tmpSocket = enterPassiveMode();

            const char* file_name = cmd_args[1].c_str();
            sprintf(cmd_buf,"STOR %s\r\n",file_name);
            sendCommand(cmd_buf);
            res = readResponse(&res_code);
            cout<<res;
            if(res[0]=='1'){
                int total_size = tmpSocket.sendFile(file_name); 
                if(total_size>=0){
                    printf("Transfer %d bytes\n",total_size);
                } else {
                    printf("Can't open file %s for reading!\n",file_name);
                }
                tmpSocket.close();

                res = readResponse(&res_code);
                cout<<res;
            }
        } else if(cmd_args[0]=="quit"){
            sock.sendString("QUIT\r\n");
            sock.close();
            exit(0);
        } else if(cmd_args[0]=="?"){
            printf(helpStr);
        } else {
            cout<<"wrong command"<<endl;
        }

        cout<<endl<<"FTP< ";
    }
    return 0;
}
TcpSocket FtpClient::enterPassiveMode(){
    sendCommand("PASV\r\n");
    string ip;
    int port;
    string res;
    int res_code;
    res = readResponse(&res_code);
    cout<<res;
    if(res_code!=-1 && res[0]=='2'){
        int st = res.find('(')+1;
        int ed = st;
        int dotMeeted=0;
        while(dotMeeted<4){
            ++ed;
            if(res[ed]==','){
                res[ed]='.';
                ++dotMeeted;
            }
        }
        ++ed;
        ip = res.substr(st,ed-st-1);
        port = 0;
        while(res[ed]!=','){
            port = (port) * 10 + res[ed]-'0';
            ++ed;
        }
        ++ed;
        int lowpart = 0;
        while(res[ed]!=')'){
            lowpart = lowpart * 10 + res[ed]-'0';
            ++ed;
        }
        port = (port<<8)+lowpart;
        printf("passive ip:%s port:%d\n",ip.c_str(),port);

        TcpSocket sock;
        sock.connectToServ(ip.c_str(),port);
        return sock;
    }
}
int main(int argc,char* argv[]){
    if(argc<4){
        printf("usage: exe [port] [username] [password]\n");
        return 0;
    }
    FtpClient c;
    if(c.connectToHost("127.0.0.1",atoi(argv[1]),argv[2],argv[3])<0){
        return 0;
    }
    c.processInput();
}
