#include "xformat.h"
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

/**
* 复制参数（线程安全）.
*
* \param stream_index 对应c_->streams 下标
* \param dst 输出参数
* \return 是否成功
*/
bool XFormat::CopyPara(int stream_index, AVCodecParameters* dst)
{
	unique_lock<mutex> lock(mux_);
	if (!c_)
	{
		return false;
	}
	if (stream_index<0 || stream_index > c_->nb_streams)
		return false;
	auto re = avcodec_parameters_copy(dst, c_->streams[stream_index]->codecpar);
	if (re < 0)
	{
		return false;
	}
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
				avio_close(c_->pb);
			}
			avformat_free_context(c_);
		}
		else if(c_->iformat)//输入上下文
		{
			avformat_close_input(&c_);
		}
		else
		{
			avformat_free_context(c_);
		}
		c_ = c;
		if (!c_)return;

		//区分音视频stream索引
		for (int i = 0; i < c->nb_streams; i++)
		{
			//音频
			if (c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audio_index_ = i;
			}
			else if (c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				video_index_ = i;
			}
		}
	}
}
