#include "xencode.h"
#include <iostream>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
using namespace std;

AVCodecContext* XEncode::Create(int codec_id)
{
    //找到编码器
    auto codec = avcodec_find_encoder((AVCodecID)codec_id);
    if (!codec)
    {
        cerr << "avcodec_find_encoder failed!" << codec_id << endl;
        return nullptr;
    }
    //创建上下文
    auto c = avcodec_alloc_context3(codec);
    if (!c)
    {
        cerr << "avcodec_alloc_context3 failed!" << c << endl;
        return nullptr;
    }
    //设置默认值
    c->time_base = { 1,25 };
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    c->thread_count = 16;
    return c;
}

void XEncode::set_c(AVCodecContext* c)
{
    unique_lock<mutex>lock(mux_);
    if (c_)
    {
        avcodec_free_context(&c_);
    }
    this->c_ = c;
}

bool XEncode::SetOpt(const char* key, const char* val)
{
    unique_lock<mutex>lock(mux_);
    if (!c_)return false;
    auto re = av_opt_set(c_->priv_data, key, val, 0);
    if (re != 0)
    {
        cerr << "av_opt_set failed" << endl;
        printErr(re);
    }
    return true;
}

bool XEncode::SetOpt(const char* key, int val)
{
	unique_lock<mutex>lock(mux_);
	if (!c_)return false;
	auto re = av_opt_set_int(c_->priv_data, key, val, 0);
    if (re != 0)
	{
		cerr << "av_opt_set failed" << endl;
		printErr(re);
        return false;
	}
	return true;
}

bool XEncode::Open()
{
	unique_lock<mutex>lock(mux_);
	if (!c_)return false;
    auto re = avcodec_open2(c_, NULL, NULL);
	if (re != 0)
	{
		cerr << "avcodec_open2 failed" << endl;
		printErr(re);
		return false;
	}
	return true;
}

AVPacket* XEncode::Encode(const AVFrame* frame)
{
    if (!frame)return nullptr;
	unique_lock<mutex>lock(mux_);
	if (!c_)return nullptr;
    //发送到编码线程
    auto re = avcodec_send_frame(c_, frame);
    if (re != 0)return nullptr;
    auto pkt = av_packet_alloc();
    //接受编码线程数据
    re = avcodec_receive_packet(c_, pkt);
	if (re == 0)
	{
		return pkt;
	}
    av_packet_free(&pkt);
    if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
    {
        return nullptr;
    }
    if (re < 0)
    {
        printErr(re);
    }
	return nullptr;
}

AVFrame* XCodec::CreateFrame()
{
	unique_lock<mutex>lock(mux_);
	if (!c_)return nullptr;
    auto frame = av_frame_alloc();
    frame->width = c_->width;
    frame->height = c_->height;
    frame->format = c_->pix_fmt;
    auto re = av_frame_get_buffer(frame, 0);
    if (re != 0)
    {
        av_frame_free(&frame);
        printErr(re);
        return nullptr;
    }
    return frame;
}

std::vector<AVPacket*> XEncode::End()
{
    std::vector<AVPacket*> res;
    unique_lock<mutex>lock(mux_);
    if (!c_)return res;
    auto re = avcodec_send_frame(c_, NULL);//发送NULL，获取缓冲
    if (re != 0)return res;
    while (re >= 0)
    {
        auto pkt = av_packet_alloc();
        re = avcodec_receive_packet(c_,pkt);
        if (re != 0)
        {
            av_packet_free(&pkt);
            break;

        }
        res.push_back(pkt);
    }
    return res;
}
