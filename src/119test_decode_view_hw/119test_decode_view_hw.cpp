#include <iostream>
#include <fstream>
#include <string>
#include "xvideoview.h"

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

	//找解码器
	auto codec = avcodec_find_decoder(codec_id);

	//创建上下文
	auto c = avcodec_alloc_context3(codec);
	c->thread_count = 16;

	//硬件加速DXVA2
	auto hw_type = AV_HWDEVICE_TYPE_DXVA2;

	/**
	 * 打印所有支持的硬件加速方式.
	 */
	for (int i = 0;; i++)
	{
		auto config = avcodec_get_hw_config(codec, i);
		if (!config)break;
		if (config->device_type)
		{
			cout << av_hwdevice_get_type_name(config->device_type) << endl;
		}
	}
	//初始化硬件加速上下文
	AVBufferRef* hw_ctx = nullptr;
	av_hwdevice_ctx_create(&hw_ctx, hw_type, NULL, NULL, 0);
	c->hw_device_ctx = av_buffer_ref(hw_ctx);

	//打开上下文
	avcodec_open2(c, NULL, NULL);

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
				//发送packet到解码线程
				ret = avcodec_send_packet(c, pkt);
				if (ret < 0)break;
				//获取多帧解码数据
				while (ret >= 0)
				{
					//每次会调用av_frame_unref
					ret = avcodec_receive_frame(c, frame);
					if (ret < 0)break;
					auto pframe = frame;//为了同时支持硬解码和软解码
					if (c->hw_device_ctx)//硬解码
					{
						//硬解码转换,显存到内存
						av_hwframe_transfer_data(hw_frame,frame,0);
						pframe = hw_frame;
					}
					AV_PIX_FMT_DXVA2_VLD;
					cout << pframe->format << " " << flush;
					if (!is_Init_window)
					{
						is_Init_window = true;
						view->Init(pframe->width, pframe->height, (XVideoView::Format)pframe->format);
					}
					view->DrawFrame(pframe);
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
	//取出缓存数据
	int ret = avcodec_send_packet(c, NULL);
	while (ret>=0)
	{
		ret = avcodec_receive_frame(c, frame);
		if (ret < 0)break;
		//cout << frame->format << "--" << flush;
		
	}

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	return 0;
}
