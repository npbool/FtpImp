#include "FtpClient.h"


string FtpClient::readResponse(int* res_code){
    return sock.readLine(res_code);
}

int FtpClient::sendCommand(const char* cmd){
    printf("Command:%s\n",cmd);
    sock.send(cmd,strlen(cmd));
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
    while(cin.getline(line)){
        line.resize(line.size()-1);
        if(line=="pwd\n"){
            sendCommand("PWD\r\n");

        } else if(line=="dir\n"){
            string ip;
            int port;
            enterPassiveMode(&ip,&port);
            TcpSocket tmpSock;
            tmpSock.connectToServ(ip,port);
            sendCommand("LIST\r\n");

        } else if(startsWith("cd")){
            parse(line);
            sprintf(cmd_buf,"CWD %s\r\n",cmd_args[1]);
            sendCommand()
        }
    }
}
int FtpClient::enterPassiveMode(string* ip,int* port){
    sendCommand("PASV\r\n");
    string res;
    int res_code;
    res = readResponse(&res_code);
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
        *ip = res.substr(st,ed-st);
        *port = 0;
        ++ed;
        while(str[ed]!=','){
            *port = (*port) * 10 + str[ed]-'0';
            ++ed;
        }
        ++ed;
        *port <<= 8;
        while(str[ed]!=')'){
            *port = (*port) * 10 + str[ed]-'0';
        }
        printf("passive ip:%s port:%s\n",ip->c_str(),*port);
        return 0;
    } else {
        return -1;
    }
}
int main(){
    FtpClient c;
    c.connectToHost("127.0.0.1",TEST_PORT,"npbool","111");
    c.enterPassiveMode();
    c.processInput();
}
