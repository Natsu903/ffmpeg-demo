#include <iostream>
//指定函数是c语言，函数名不包含重载标注
extern "C" 
{
#include <libavcodec/avcodec.h>
}

using namespace std;

//预处理指令导入库
#pragma comment(lib,"avcodec.lib")

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;
    cout << avcodec_configuration() << endl;
    return 0;
}