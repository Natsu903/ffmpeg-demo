#include <iostream>
#include <fstream>
#include <string>
#include "xvideoview.h"
#include "xdecode.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

using namespace std;

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")


int main(int argc, char* argv[])
{
	auto view = XVideoView::Create();

	//1.分割h264 存入AVPacket
	string filename = "test.h264";
	ifstream ifs(filename, ios::binary);
	if (!ifs) return -1;
	unsigned char inbuf[4096] = { 0 };
	AVCodecID codec_id = AV_CODEC_ID_H264;

	XDecode de;
	//根据编码器创建上下文
	auto c = de.Create(codec_id, false);
	de.set_c(c);
	de.InitHW();//硬件加速
	de.Open();

	//分割上下文
	auto parser = av_parser_init(codec_id);
	auto pkt = av_packet_alloc();
	auto frame = av_frame_alloc();
	auto hw_frame = av_frame_alloc();//硬解码转换
	auto begin = NowMs();
	int count = 0;
	bool is_Init_window = false;

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		int data_size = ifs.gcount();
		if (data_size <= 0)break;
		auto data = inbuf;
		while (data_size > 0)
		{
			//通过0001截断输出到AVPacket返回帧大小
			int ret = av_parser_parse2(parser, c,
				&pkt->data, &pkt->size,//输出
				data, data_size,//输入
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
			);
			data += ret;
			data_size -= ret;//已处理
			if (pkt->size)
			{
				cout << pkt->size << " " << flush;
				//获取多帧解码数据
				if (!de.Send(pkt))break;
				while (de.Recv(frame))
				{
					if (!is_Init_window)
					{
						is_Init_window = true;
						view->Init(frame->width, frame->height, (XVideoView::Format)frame->format);
					}
					view->DrawFrame(frame);
					count++;
					auto cur = NowMs();
					if (cur - begin >= 100)
					{
						cout << "\nfps" << count*10 << endl;
						count = 0;
						begin = cur;
					}
				}
			}
		}
	}
	auto frames = de.End();
	for (auto f : frames)
	{
		view->DrawFrame(f);
		av_frame_free(&f);
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	return 0;
}
