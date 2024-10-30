﻿#include <iostream>
#include <thread>
#include "XDemux.h"
#include "XMux.h"
#include "xdecode.h"
#include "xencode.h"

using namespace std;
extern "C"
{
#include <libavformat/avformat.h>
#include "libavcodec/avcodec.h"
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")


int main(int argc, char* argv[])
{
	/*输入参数处理*/
	string usage = "124_test_xformat 输入文件 输出文件 开始时间，结束时间（秒）视频宽度 视频高度\n";
	usage += "124_test_xformat v1080.mp4 out.mp4 10 20 400 300";
	std::cout << usage << endl;
	if (argc < 3) return -1;
	string in_file = argv[1];
	string out_file = argv[2];

	//截取10-20秒间的音视频
	int begin_sec = 0;	//截取开始时间
	int end_sec = 0;	//截取结束时间
	if (argc > 3)
	{
		begin_sec = atoi(argv[3]);
	}
	if (argc > 4)
	{
		end_sec = atoi(argv[4]);
	}

	int video_width = 0;
	int video_height = 0;
	if (argc > 6)
	{
		video_width = atoi(argv[5]);
		video_height = atoi(argv[6]);
	}

	/*解封装*/ 
	//解封装输入上下文 
	XDemux demux;
	auto demux_c = demux.Open(in_file.c_str());
	demux.set_c(demux_c);


	long long video_begin_pts = 0;
	long long audio_begin_pts = 0;
	long long video_end_pts = 0;

	//开始截断秒数
	//if (begin_sec > 0)
	{
		if (demux.video_index() >= 0 && demux.video_time_base().num > 0)
		{
			double t = (double)demux.video_time_base().den / demux.video_time_base().num;
			video_begin_pts = t * begin_sec;
			video_end_pts = t * end_sec;
			demux.Seek(video_begin_pts, demux.video_index());
		}
		if (demux.audio_index() >= 0 && demux.audio_time_base().num > 0)
		{
			double t = (double)demux.audio_time_base().den / demux.video_time_base().num;
			audio_begin_pts = t * begin_sec;
		}
	}
	/*解码初始化*/
	XDecode decode;
	auto decode_c = decode.Create(demux.video_codec_id(), false);
	//设置视频解码器参数
	demux.CopyPara(demux.video_index(), decode_c);

	decode.set_c(decode_c);
	decode.Open();
	auto frame = decode.CreateFrame();//解码后存储

	if (demux.video_index() >= 0)
	{
		if (video_width <= 0)
			video_width = demux_c->streams[demux.video_index()]->codecpar->width;
		if (video_height <= 0)
			video_height = demux_c->streams[demux.video_index()]->codecpar->height;
	}
	XEncode encode;
	auto encode_c = encode.Create(AV_CODEC_ID_H265, true);
	encode_c->pix_fmt = AV_PIX_FMT_YUV420P;
	encode_c->width = video_width;
	encode_c->height = video_height;
	encode.set_c(encode_c);
	encode.Open();

	/*封装*/ 
	XMux mux;
	auto mux_c = mux.Open(out_file.c_str());
	mux.set_c(mux_c);
	auto mvs = mux_c->streams[mux.video_index()];//视频流信息
	auto mas = mux_c->streams[mux.audio_index()];//视频流信息

	//有视频
	if (demux.video_index() >= 0)
	{
		mvs->time_base.num = demux.video_time_base().num;
		mvs->time_base.den = demux.video_time_base().den;
		//复制视频参数
		//demux.CopyPara(demux.video_index(), mvs->codecpar);
		//复制编码器格式
		avcodec_parameters_from_context(mvs->codecpar, encode_c);
	}
	//有音频 
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num;
		mas->time_base.den = demux.audio_time_base().den;
		//复制音频参数 
		demux.CopyPara(demux.audio_index(), mas->codecpar);
	}
	//写入head会改变timebase
	mux.WriteHead();

	int audio_count = 0;
	int video_count = 0;
	double total_sec = 0;
	AVPacket pkt;
	while (true)
	{
		if (!demux.Read(&pkt)) break;
		//视频 时间大于结束时间
		if (video_end_pts > 0 && pkt.stream_index==demux.video_index()&& pkt.pts > video_end_pts)
		{
			av_packet_unref(&pkt);
			break;
		}
		if (pkt.stream_index == demux.video_index())
		{
			mux.RescaleTime(&pkt, video_begin_pts, demux.video_time_base());
			
			if (decode.Send(&pkt))
			{
				while (decode.Recv(frame))
				{
					auto epkt = encode.Encode(frame);
					if (epkt)
					{
						//写入视频帧,清理pkt 
						mux.Write(epkt);
						av_packet_free(&epkt);
					}
				}
			}
			video_count++;
			if (demux.video_time_base().den > 0)
			{
				total_sec += pkt.duration * ((double)demux.video_time_base().num / demux.video_time_base().den);
			}
			av_packet_unref(&pkt);
		}
		else if (pkt.stream_index == demux.audio_index())
		{
			mux.RescaleTime(&pkt, audio_begin_pts, demux.audio_time_base());
			audio_count++;
			//写入音频帧,清理pkt 
			mux.Write(&pkt);
		}
		else
		{
			av_packet_unref(&pkt);
		}
		
	}

	//写入结尾
	mux.WriteEnd();
	demux.set_c(nullptr);
	mux.set_c(nullptr);
	encode.set_c(nullptr);
	std::cout << "输出文件" << out_file << ":" << endl;
	std::cout << "视频帧：" << video_count << endl;
	std::cout << "音频帧：" << audio_count << endl;
	std::cout << "总时长：" << total_sec << endl;
	return 0;
}
