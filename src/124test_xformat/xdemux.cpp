#include "xdemux.h"
#include <iostream>
#include <thread>
using namespace std;
extern "C" { //指定函数是c语言函数，函数名不包含重载标注
	//引用ffmpeg头文件
#include <libavformat/avformat.h>
}
//预处理指令导入库
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")

static void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}
#define BERR(err) if(err!=0){PrintErr(err);return 0;}


AVFormatContext* XDemux::Open(const char* url)
{
    AVFormatContext* c = nullptr;
	//打开封装上下文
    auto re = avformat_open_input(&c, url, nullptr, nullptr);
    BERR(re);
	//获取媒体信息
	re = avformat_find_stream_info(c, nullptr);
	BERR(re);
	//打印输入封装信息
	av_dump_format(c, 0, url,0);
    return c;
}

bool XDemux::Read(AVPacket* pkt)
{
	unique_lock<mutex> lock(mux_);
	if (!c_)return false;
	auto re = av_read_frame(c_, pkt);
	BERR(re);
	return true;
}
