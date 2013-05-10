#include "var.h"
bool startsWith(string& s,const char* pat){
    if(strlen(pat)>s.length())
        return false;
    int index=0;
    while(*pat){
        if(s[index++]!=*pat)
            return false;
        ++pat;
    }
    return true;
}
int toInt(const string& s){
    return atoi(s.c_str());
}

string execShellCmd(const char* cmd,int* res_code){
    string res;
    const int BUF_SIZE = 128;
    static char buf[BUF_SIZE];

    FILE* ptr;
    if((ptr=popen(cmd,"r"))!=NULL){
        while(fgets(buf,BUF_SIZE,ptr)!=NULL){
            res+=buf;
        }
        pclose(ptr);
        *res_code = 0;
    } else {
        perror("can't open pipe!");
        *res_code = -1;
    }
    return res;
}
