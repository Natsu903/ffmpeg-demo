#include "xdemux.h"
#include <iostream>
#include "xtools.h"
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;

#define BERR(err) if(err!=0){PrintErr;return 0;}

AVFormatContext* XDemux::Open(const char* url)
{
    AVFormatContext* c = nullptr;
    AVDictionary* opts = nullptr;
    //av_dict_set(&opts, "rtsp transport", "tcp", 0);//传输媒体流为tcp，默认为utp
    av_dict_set(&opts, "stimeout", "1000", 0);//连接超时1s
    auto re = avformat_open_input(&c, url, nullptr, &opts);
	BERR(re);
    if (opts) av_dict_free(&opts);
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
    last_time_ = NowMs();
    return true;
}

bool XDemux::Seek(long long pts, int stream_index)
{
	unique_lock<mutex> lock(mux_);
	if (!c_) return false;
	auto re = av_seek_frame(c_, stream_index, pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);//向后关键帧
	BERR(re);
	return true;
}
