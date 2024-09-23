#include "xcodec.h"
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

void printErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

AVCodecContext* XCodec::Create(int codec_id,bool is_encode)
{
	//找到编码器
	const AVCodec* codec=nullptr;
	if (is_encode)
	{
		codec = avcodec_find_encoder((AVCodecID)codec_id);
	}
	else
	{
		codec = avcodec_find_decoder((AVCodecID)codec_id);
	}
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

void XCodec::set_c(AVCodecContext* c)
{
	unique_lock<mutex>lock(mux_);
	if (c_)
	{
		avcodec_free_context(&c_);
	}
	this->c_ = c;
}

bool XCodec::SetOpt(const char* key, const char* val)
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

bool XCodec::SetOpt(const char* key, int val)
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

bool XCodec::Open()
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