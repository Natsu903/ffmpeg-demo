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

	/*解封装*/

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

	/*封装*/ 
	const char* out_url = "out.mp4";
	AVFormatContext* ec = nullptr;
	re = avformat_alloc_output_context2(&ec, nullptr, nullptr,
		out_url//根据文件名推测格式 
		);
	CERR(re);
	//添加视频、音频流 
	auto mvs = avformat_new_stream(ec, nullptr);//视频流
	auto mas = avformat_new_stream(ec, nullptr);//音频流

	//打开输入输出io 
	re = avio_open(&ec->pb, out_url, AVIO_FLAG_WRITE);
	CERR(re);

	//设置编码音视频流参数 
	if(vs)
	{
		mvs->time_base = vs->time_base;//时间基数与原视频一致 
		//从解封装复制参数
		avcodec_parameters_copy(mvs->codecpar, vs->codecpar);
	}
	if (as)
	{
		mas->time_base = as->time_base;
		//从解封装复制参数
		avcodec_parameters_copy(mas->codecpar, as->codecpar);
	}
	//写入文件头
	re = avformat_write_header(ec, nullptr);
	CERR(re);
	//打印输出上下文
	av_dump_format(ec, 0, out_url, 1);

	//截取10-20秒间的音视频
	double begin_sec = 10;	//截取开始时间
	double end_sec = 20;	//截取结束时间
	long long begin_audio_pts = 0;
	long long begin_pts = 0;
	long long end_pts = 0;
	//换算成输入ic的pts
	if (vs && vs->time_base.num > 0)
	{
		double t = (double)vs->time_base.den / (double)vs->time_base.num;
		begin_pts = begin_sec * t;
		end_pts = end_sec * t;
	}
	if (as&&as->time_base.num>0)
		begin_audio_pts = begin_sec * ((double)as->time_base.den / (double)as->time_base.num);

	//Seek输入媒体文件,移动到第十秒关键帧
	if (vs)
	{
		re = av_seek_frame(ic, vs->index, begin_pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);//向后关键帧
	}CERR(re);


	AVPacket pkt;
	while (true)
	{
		re = av_read_frame(ic, &pkt);
		if (re != 0) break;
		CERR(re);

		//重新计算pts、dts、duration
		AVStream* in_stream = ic->streams[pkt.stream_index];
		AVStream* out_stream = nullptr;
		long long offset_pts;//偏移pts，用于截断的开头pts运算

		if (vs && pkt.stream_index == vs->index)
		{
			cout << "video:";
			//超过20s退出
			if (pkt.pts > end_pts)
			{
				av_packet_unref(&pkt);
				break;
			}
			out_stream = ec->streams[0];
			offset_pts = begin_pts;
		}
		else if (as && pkt.stream_index == as->index)
		{
			cout << "audio:";
			out_stream = ec->streams[1];
			offset_pts = begin_audio_pts;
		}
		cout << pkt.pts << ":" << pkt.dts << ":" << pkt.size << endl;
		
		pkt.pts = av_rescale_q_rnd(pkt.pts- offset_pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts- offset_pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration=av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		//写入音视频帧,清理pkt 
		re = av_interleaved_write_frame(ec, &pkt);
		if (re != 0)
		{
			PrintErr(re);
		}
		av_packet_unref(&pkt);
		this_thread::sleep_for(100ms);
	}
	//写入结尾
	re = av_write_trailer(ec);
	if (re != 0)
	{
		PrintErr(re);
	}

	avformat_close_input(&ic);
	avio_closep(&ec->pb);

	avformat_free_context(ec);
	ec = nullptr;

	system("pause");
	return 0;
}
