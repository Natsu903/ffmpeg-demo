#include "XDemux.h"
#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;

static void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

#define BERR(err) if(err!=0){PrintErr;return 0;}

AVFormatContext* XDemux::Open(const char* url)
{
    AVFormatContext* c = nullptr;
    auto re = avformat_open_input(&c, url, nullptr, nullptr);
    BERR(re);
    //获取媒体信息
    re = avformat_find_stream_info(c, nullptr);
    BERR(re);
    //打印输入封装信息
    av_dump_format(c, 0, url, 0);

    return c;
}

bool XDemux::Read(AVPacket* pkt)
{
    unique_lock<mutex> lock(mux_);
    if (!c_) return false;
    auto re = av_read_frame(c_, pkt);
    BERR(re);
    return true;
}
