#include "var.h"
#include <iostream> 
using namespace std;
 int main(){
     int c;
     string res = execShellCmd("ls -al",&c);
     cout<<res<<endl;
 
 }

