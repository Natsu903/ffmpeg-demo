#include "xmux.h"

#include <iostream>
#include <thread>
using namespace std;
extern "C" { //指定函数是c语言函数，函数名不包含重载标注
//引用ffmpeg头文件
#include <libavformat/avformat.h>
}
static void PrintErr(int err)
{
    char buf[1024] = { 0 };
    av_strerror(err, buf, sizeof(buf) - 1);
    cerr << buf << endl;
}
#define BERR(err) if(err!= 0){PrintErr(err);return 0;}

//////////////////////////////////////////////////
//// 打开封装
AVFormatContext* XMux::Open(const char* url)
{
    AVFormatContext* c = nullptr;
    //创建上下文
    auto re = avformat_alloc_output_context2(&c, NULL, NULL, url);
    BERR(re);

    //添加视频音频流
    auto vs = avformat_new_stream(c, NULL);   //视频流
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    auto as = avformat_new_stream(c, NULL);   //音频流
    as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;


    //打开IO
    re = avio_open(&c->pb, url, AVIO_FLAG_WRITE);
    BERR(re);
    return c;
}
bool XMux::Write(AVPacket* pkt)
{
    unique_lock<mutex> lock(mux_);
    if (!c_)return false;
    //写入一帧数据，内部缓冲排序dts，通过pkt=null 可以写入缓冲
    auto re = av_interleaved_write_frame(c_,pkt);
    BERR(re);
    return true;
}

bool XMux::WriteEnd()
{
    unique_lock<mutex> lock(mux_);
    if (!c_)return false;
    av_interleaved_write_frame(c_, nullptr);//写入排序缓冲
    auto re = av_write_trailer(c_);
    BERR(re);
    return true;
}
bool XMux::WriteHead()
{
    unique_lock<mutex> lock(mux_);
    if (!c_)return false;
    auto re = avformat_write_header(c_, nullptr);
    BERR(re);

    //打印输出上下文
    av_dump_format(c_, 0, c_->url, 1);

    return true;
}