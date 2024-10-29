#include "XFormat.h"
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

using namespace std;

bool XFormat::CopyPara(int stream_index, AVCodecParameters* dst)
{
	unique_lock<mutex> mux_;
	if (!c_) return false;
	if (stream_index<0 || stream_index>c_->nb_streams) return false;
	auto re = avcodec_parameters_copy(dst, c_->streams[stream_index]->codecpar);
	if (re < 0) return false;
	return true;
}

void XFormat::set_c(AVFormatContext* c)
{
	unique_lock<mutex> lock(mux_);
	if (c_)//清理原值
	{
		if (c_->oformat)//输出上下文
		{
			if (c_->pb)
			{
				avio_closep(&c_->pb);
			}
			avformat_free_context(c_);
		}
		else if (c_->iformat)
		{
			avformat_close_input(&c_);
		}
		else
		{
			avformat_free_context(c_);
		}
	}
	c_ = c;
	if (!c_)return;
	//用于区分是否有音频或视频流
	audio_index_ = -1;
	video_index_ = -1;
	//区分音视频索引
	for (int i = 0; i < c->nb_streams; i++)
	{
		if (c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_index_ = i;
			audio_time_base_.den = c_->streams[i]->time_base.den;
			audio_time_base_.num = c_->streams[i]->time_base.num;
		}
		else if (c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_index_ = i;
			video_time_base_.den = c_->streams[i]->time_base.den;
			video_time_base_.num = c_->streams[i]->time_base.num;
		}
	}
}
