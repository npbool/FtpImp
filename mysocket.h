#ifndef MYSOCKET_H_INCLUDED
#define MYSOCKET_H_INCLUDED

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;
struct TcpSocket{
    int sockfd;
    struct sockaddr_in servaddr;
    int port;

    TcpSocket(){
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd==-1){
            perror("can't create socket\n");
        }
    }
    TcpSocket(int _sockfd,struct sockaddr_in _addr){
        sockfd = _sockfd;
        servaddr = _addr;
    }
    int connectToServ(const char* ip,int port){
        memset(&servaddr,0,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = inet_addr(ip);
        if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1){
            perror("can't connect to server!\n");
            return -1;
        }
        return 0;
    }

    int send(const void* buf,int len){
        return ::send(sockfd,buf,len,0);
    }
    int recv(void* buf,int len){
        return ::recv(sockfd,buf,len,0);
    }
    int sendString(const char* str){
        return send(str,strlen(str));
    }
    string readLine(int* res_code){
        string res;
        const int BUF_SIZE=128;
        static char buf[BUF_SIZE+1];

        while(1){
            memset(buf,0,sizeof(buf));
            int len = recv(buf,BUF_SIZE);
            if(len>0){
                res += string(buf);
                //cout<<"part:"<<res<<endl;
                if(strstr(buf,"\r\n")!=NULL){
                    *res_code=0;
                    //printf("Response:%s\n",res.c_str());
                    return res;
                }
            } else {
                *res_code = -1;
                return res;
            }
        }
    }
    int close(){
        return ::close(sockfd);
    }
};


struct TcpServer : public TcpSocket{

    int bindAndListen(int port){
        memset(&servaddr,0,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);
        if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))!=0){
            perror("bind failed\n");
            return -1;
        }
        if(listen(sockfd,5)!=0){
            perror("listen failed\n");
            return -1;
        }
        //printf("server ip:%s",inet_ntoa(servaddr.sin_addr));
        return 0;
    }
    TcpSocket getOneConn(){
        while(true){
            int connfd;
            struct sockaddr_in connaddr;
            socklen_t addrsz=sizeof(connaddr);
            connfd = accept(sockfd,(sockaddr*)&connaddr,&addrsz);
            if(connfd==-1){
                perror("invalid connection\n");

                continue;
            }

            printf("new connection\n");
            return TcpSocket(connfd,connaddr);
        }
    }
    TcpSocket getConn(){
        while(true){
            int connfd;
            struct sockaddr_in connaddr;
            socklen_t addrsz=sizeof(connaddr);
            connfd = accept(sockfd,(sockaddr*)&connaddr,&addrsz);
            if(connfd==-1){
                perror("invalid connection\n");

                continue;
            }


            if(fork()==0){
                //in child
                printf("new connection\n");
                return TcpSocket(connfd,connaddr);
            }
            //else
            //in parent blocked in accept again!
        }
    }
};

#endif // MYSOCKET_H_INCLUDED
