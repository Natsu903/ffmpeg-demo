#include <iostream>
#include <fstream>
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

	// 1.找到编码器
	auto codec = avcodec_find_encoder(codec_id);
	if (!codec)
	{
		cerr << "codec can't find" << endl;
		return -1;
	}

	// 2.编码上下文
	auto c = avcodec_alloc_context3(codec);
	if (!c)
	{
		cerr << "avcodec_alloc_context3 failed" << endl;
		return -1;
	}

	// 3.设计上下文宽度
	c->width = 400;
	c->height = 300;

	// 帧时间戳的时间单位 pts*time_base = 播放时间(秒)
	c->time_base = {1, 25}; // 分数，1/25

	// 像素格式
	c->pix_fmt = AV_PIX_FMT_YUV420P; // 元数据格式，与编码算法相关
	c->thread_count = 16;			 // 编码线程数，可以调用系统借口获取cpu核心数量

	/**
	 * .
	 * ABR 平均比特率
	 */
	int br = 400000;
	//c->bit_rate = br;

	/**
	 * .
	 * CQP 恒定质量
	 * H.264的QP范围从0到51
	 * x264默认23 效果较好18
	 * x265默认28 效果较好25
	 */
	//av_opt_set_int(c->priv_data, "qp", 0,0);

	/**
	 * .
	 * CBR 恒定比特率 
	 */
	//c->rc_min_rate = br;
	//c->rc_max_rate = br;
	//c->rc_buffer_size = br;
	//c->bit_rate = br;
	//av_opt_set(c->priv_data, "nal-hrd", "cbr", 0);

	/**
	 * .
	 * 恒定速率因子(CRF)
	 */
	av_opt_set_int(c->priv_data, "crf", 23, 0);

	/**
	 * .
	 * 约束编码(VBV)
	 */
	c->rc_max_rate = br;
	c->rc_buffer_size = br * 2;

	////预设编码器参数
	//c->max_b_frames = 0;
	//int opt_re = av_opt_set(c->priv_data, "preset", "ultrafast", 0);//最快速度
	//if (opt_re != 0)
	//{
	//	cout << "preset failed" << endl;
	//}
	//opt_re = av_opt_set(c->priv_data, "tune", "zerolatency", 0);//零延迟 h265不支持b_frame
	//if (opt_re != 0)
	//{
	//	cout << "preset failed" << endl;
	//}

	// 4.打开编码上下文

	int re = avcodec_open2(c, codec, NULL);
	if (re != 0)
	{
		char buf[1024] = {0};
		av_strerror(re, buf, sizeof(buf) - 1);
		cerr << "avcodec_open2 failed!" << buf << endl;
		avcodec_free_context(&c);
		return -1;
	}
	cout << "avcodec_open2 success!" << endl;

	//创建AVFrame空间 未压缩数据
	auto frame = av_frame_alloc();
	frame->width = c->width;
	frame->height = c->height;
	frame->format = c->pix_fmt;
	re = av_frame_get_buffer(frame, 0);
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cerr << "avcodec_open2 failed!" << buf << endl;
		avcodec_free_context(&c);
		av_frame_free(&frame);
		return -1;
	}

	auto pkt = av_packet_alloc();

	//十秒视频 250帧
	for (int i = 0; i < 250; i++)
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

		//发送未压缩帧到线程中压缩
		re = avcodec_send_frame(c, frame);
		if (re != 0)
		{
			break;
		}
		while (re >= 0)//返回多帧
		{
			//接收压缩帧 一般前几次调用为空(缓冲，立刻返回，编码未完成)
			//编码是在独立的线程中
			//每次调用会重新分配pkt的空间
			re = avcodec_receive_packet(c, pkt);
			if (re == AVERROR(EAGAIN) || re == AVERROR_EOF) break;
			if (re < 0)
			{
				char buf[1024] = { 0 };
				av_strerror(re, buf, sizeof(buf) - 1);
				cerr << "avcodec_receive_packet failed!" << buf << endl;
				break;
			}
			cout << pkt->size << "  " << flush;
			ofs.write((char*)pkt->data, pkt->size);
			av_packet_unref(pkt);
		}
	}
	ofs.close();
	av_packet_free(&pkt);
	av_frame_free(&frame);
	// 释放上下文
	avcodec_free_context(&c);

	return 0;
}
