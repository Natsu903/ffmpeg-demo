#include <iostream>
#include <thread>
using namespace std;
extern "C"
{
#include <libavformat/avformat.h>
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

#define CERR(err) if(err!=0){PrintErr(err);getchar();return -1;}

int main(int argc, char* argv[])
{
	//打开媒体文件
	const char* url = "v1080.mp4";
	//解封装输入上下文
	AVFormatContext* ic = nullptr;
	auto re = avformat_open_input(&ic, url,
		nullptr,//封装器格式，自动探测
		nullptr);//参数设置，rtsp需要设置
	CERR(re);
	//获取媒体信息
	re = avformat_find_stream_info(ic, nullptr);
	CERR(re);
	//打印封装信息
	av_dump_format(ic, 0, url,
		0);//0表示打开的是输入，1表示输出

	AVStream* as = nullptr;//音频流 
	AVStream* vs = nullptr;//视频流 
	for (int i = 0; i < ic->nb_streams; i++)
	{
		//音频
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			as = ic->streams[i];
			cout << "-----audio-----" << endl;
			cout << "sample_rate" << as->codecpar->sample_rate << endl;
			cout << "---------------" << endl;
		}
		else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vs = ic->streams[i];
			cout << "=====video=====" << endl;
			cout << "width:" << vs->codecpar->width << endl;
			cout << "height:" << vs->codecpar->height << endl;
			cout << "===============" << endl;
		}
	}

	AVPacket pkt;
	while (true)
	{
		re = av_read_frame(ic, &pkt);
		CERR(re);
		if (vs && pkt.stream_index == vs->index)
		{
			cout << "video:";
		}
		else if (as && pkt.stream_index == as->index)
		{
			cout << "audio:";
		}
		cout << pkt.pts << ":" << pkt.dts << ":" << pkt.size << endl;
		av_packet_unref(&pkt);
		this_thread::sleep_for(100ms);
	}


	avformat_close_input(&ic);

	system("pause");
	return 0;
}
