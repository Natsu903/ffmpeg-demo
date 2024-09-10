#include <iostream>
#include <fstream>
#include <string>

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
	//1.分割h264 存入AVPacket
	string filename = "test.h264";
	ifstream ifs(filename, ios::binary);
	if (!ifs) return -1;
	unsigned char inbuf[4096] = { 0 };
	AVCodecID codec_id = AV_CODEC_ID_H264;

	//找解码器
	auto codec = avcodec_find_decoder(codec_id);

	//创建上下文
	auto c = avcodec_alloc_context3(codec);

	//打开上下文
	avcodec_open2(c, NULL, NULL);

	//分割上下文
	auto parser = av_parser_init(codec_id);
	auto pkt = av_packet_alloc();
	auto frame = av_frame_alloc();

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
				//cout << pkt->size << " " << flush;
				//发送packet到解码线程
				ret = avcodec_send_packet(c, pkt);
				if (ret < 0)break;
				//获取多帧解码数据
				while (ret>=0)
				{
					//每次会调用av_frame_unref
					ret = avcodec_receive_frame(c, frame);
					if (ret < 0)break;
					cout << frame->format << " " << flush;
				}
			}
		}
	}
	//取出缓存数据
	int ret = avcodec_send_packet(c, NULL);
	while (ret>=0)
	{
		ret = avcodec_receive_frame(c, frame);
		if (ret < 0)break;
		cout << frame->format << "--" << flush;
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	return 0;
}
