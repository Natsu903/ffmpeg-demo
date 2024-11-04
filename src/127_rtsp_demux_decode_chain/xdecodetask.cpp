#include "xdecodetask.h"
#include <iostream>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

using namespace std;

bool XDecodeTask::Open(AVCodecParameters* para)
{
	if (!para)
	{
		LOGERROR("para is null");
		return false;
	}
	unique_lock<mutex> lock(mux_);
	auto c = decode_.Create(para->codec_id,false);
	if (!c)
	{
		LOGERROR("decode_.Create failed");
		return false;
	}
	//复制视频参数
	avcodec_parameters_to_context(c, para);
	decode_.set_c(c);
	if (!decode_.Open())
	{
		LOGERROR("decode_.Open() failed");
		return false;
	}
	LOGINFO("Open codec success!");
	return true;
}

void XDecodeTask::Do(AVPacket* pkt)
{
	cout << "#" << flush;
	if (!pkt||pkt->stream_index != 0)//判断是不是视频流
	{
		return;
	}
	pkt_list_.Push(pkt);
}

void XDecodeTask::Main()
{
	{
		unique_lock<mutex> lock(mux_);
		if (!frame_) frame_ = av_frame_alloc();
	}
	while (!is_exit_)
	{
		auto pkt = pkt_list_.Pop();
		if (!pkt)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		//发送到解码线程
		bool re = decode_.Send(pkt);
		av_packet_free(&pkt);
		if (!re)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		{
			unique_lock<mutex> lock(mux_);
			if (decode_.Recv(frame_))
			{
				cout << "@" << flush;
			}
		}
		this_thread::sleep_for(1ms);
	}
	{
		unique_lock<mutex> lock(mux_);
		if (frame_) av_frame_free(&frame_);
	}
}
