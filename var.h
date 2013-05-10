#ifndef CONST_H__
#define CONST_H__


#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
using std::string;

const int CNTL_PORT=20;
const int DATA_PORT=21;
const int TEST_PORT=12345;


bool startsWith(string& s,const char* pat);
int toInt(string& s);

string execShellCmd(const char* cmd,int* res_code);

#endif
