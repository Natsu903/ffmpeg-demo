﻿#include <iostream>
#include <thread>
using namespace std;
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}
#define CERR(err) if(err!=0){PrintErr(err);return -1;}

int main(int argc, char* argv[])
{
	//打开媒体文件
	const char* url = "v1080.mp4";
	//解封装上下文
	AVFormatContext* ic = nullptr;
	auto re = avformat_open_input(&ic, url,
		NULL,						//封装器格式，NULL表示自动探测(根据后缀名或文件头)
		NULL						//参数设置，rtsp需要设置
	);
	CERR(re);

	//获取媒体信息(无头部格式)
	re = avformat_find_stream_info(ic, NULL);
	CERR(re);
	//打印封装信息
	av_dump_format(ic, 0, url,
		0						//0表示输入，1表示输出
	);
	AVStream* as = nullptr;
	AVStream* vs = nullptr;
	for (int i = 0; i < ic->nb_streams; i++)
	{
		//音频
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			as = ic->streams[i];
			cout << "===音频===" << endl;
			cout <<"sample_rate:" << as->codecpar->sample_rate << endl;
		}
		else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vs = ic->streams[i];
			cout << "===视频===" << endl;
			cout << "width:" << vs->codecpar->width << endl;
			cout << "height:" << vs->codecpar->height << endl;
		}
	}
	AVPacket pkt;
	for (;;)
	{
		re = av_read_frame(ic, &pkt);
		CERR(re);
		if (vs && pkt.stream_index == vs->index)
		{
			cout << "视频：";
		}
		else if (as && pkt.stream_index == as->index)
		{
			cout << "音频：";
		}
		cout << pkt.pts << ":" << pkt.dts << ":" << pkt.size << endl;
		av_packet_unref(&pkt);
		this_thread::sleep_for(100ms);
	}

	avformat_close_input(&ic);
	return 0;
}
