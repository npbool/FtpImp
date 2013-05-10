#ifndef CONST_H__
#define CONST_H__


#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
using std::string;

const int CNTL_PORT=21;
const int DATA_PORT=20;
const int TEST_PORT=12345;


bool startsWith(string& s,const char* pat);
int toInt(string& s);

string execShellCmd(const char* cmd,int* res_code);

#endif
