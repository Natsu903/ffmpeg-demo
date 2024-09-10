#include <iostream>
#include <fstream>
#include "xencode.h"
using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib,"avutil.lib")

int main(int argc, char *argv[])
{
	string filename = "400_300_25_preset";
	AVCodecID codec_id = AV_CODEC_ID_H264;
	if (argc > 1)
	{
		string codec = argv[1];
		if (codec == "h265" || codec == "hevc")
		{
			codec_id = AV_CODEC_ID_HEVC;
		}
	}
	if (codec_id == AV_CODEC_ID_H264)
	{
		filename += ".h264";
	}
	else if (codec_id == AV_CODEC_ID_HEVC)
	{
		filename += ".h265";
	}
	ofstream ofs;
	ofs.open(filename, ios::binary);
	
	XEncode en;
	auto c = en.Create(codec_id);
	c->width = 400;
	c->height = 300;
	en.set_c(c);
	en.SetOpt("crf", 18);
	en.Open();
	auto frame = en.CreateFrame();
	//十秒视频 250帧
	for (int i = 0; i < 500; i++)
	{
		//生成AVFrame数据 每帧数据不同
		//Y
		for (int y = 0; y < c->height; y++)
		{
			for (int x = 0; x < c->width; x++)
			{
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}
		//U,V
		for (int y = 0; y < c->height / 2; y++)
		{
			for (int x = 0; x < c->width / 2; x++)
			{
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}
		frame->pts = i;//显示时间
		auto pkt = en.Encode(frame);
		if (pkt)
		{
			ofs.write((char*)pkt->data, pkt->size);
			av_packet_free(&pkt);
		}
	}
	auto pkts = en.End();
	for (auto pkt : pkts)
	{
		ofs.write((char*)pkt->data, pkt->size);
		av_packet_free(&pkt);
	}
	ofs.close();
	en.set_c(nullptr);
	return 0;
}
