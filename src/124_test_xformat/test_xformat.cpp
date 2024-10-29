#include <iostream>
#include <thread>
#include "XDemux.h"
#include "XMux.h"
using namespace std;
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")


int main(int argc, char* argv[])
{
	string usage = "124_test_xformat 输入文件 输出文件 开始时间，结束时间（秒）\n";
	usage += "124_test_xformat v1080.mp4 out.mp4 10 20";
	//打开媒体文件 
	const char* url = "v1080.mp4";

	/*解封装*/ 

	//解封装输入上下文 
	XDemux demux;
	auto demux_c = demux.Open(url);

	demux.set_c(demux_c);

	/*封装*/ 
	const char* out_url = "out.mp4";

	XMux mux;
	auto mux_c = mux.Open(out_url);
	mux.set_c(mux_c);
	auto mvs = mux_c->streams[mux.video_index()];//视频流信息
	auto mas = mux_c->streams[mux.audio_index()];//视频流信息

	//有视频
	if (demux.video_index() >= 0)
	{
		mvs->time_base.num = demux.video_time_base().num;
		mvs->time_base.den = demux.video_time_base().den;
		//复制视频参数
		demux.CopyPara(demux.video_index(), mvs->codecpar);
	}
	//有音频 
	if (demux.audio_index() >= 0)
	{
		mas->time_base.num = demux.audio_time_base().num;
		mas->time_base.den = demux.audio_time_base().den;
		//复制音频参数 
		demux.CopyPara(demux.audio_index(), mas->codecpar);
	}
	mux.WriteHead();

	//截取10-20秒间的音视频
	double begin_sec = 10;	//截取开始时间
	double end_sec = 20;	//截取结束时间
	long long begin_audio_pts = 0;
	long long begin_pts = 0;
	long long end_pts = 0;
	//换算成输入ic的pts
	//if (vs && vs->time_base.num > 0)
	//{
	//	double t = (double)vs->time_base.den / (double)vs->time_base.num;
	//	begin_pts = begin_sec * t;
	//	end_pts = end_sec * t;
	//}
	//if (as&&as->time_base.num>0)
	//	begin_audio_pts = begin_sec * ((double)as->time_base.den / (double)as->time_base.num);

	//Seek输入媒体文件,移动到第十秒关键帧
	//if (vs)
	//{
	//	re = av_seek_frame(ic, vs->index, begin_pts, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);//向后关键帧
	//}
	//CERR(re);


	AVPacket pkt;
	while (true)
	{
		//re = av_read_frame(ic, &pkt);
		//if (re != 0) break;
		//CERR(re);
		if(!demux.Read(&pkt)) break;

		////重新计算pts、dts、duration
		//AVStream* in_stream = ic->streams[pkt.stream_index];
		//AVStream* out_stream = nullptr;
		//long long offset_pts;//偏移pts，用于截断的开头pts运算

		//if (vs && pkt.stream_index == vs->index)
		//{
		//	cout << "video:";
		//	//超过20s退出
		//	if (pkt.pts > end_pts)
		//	{
		//		av_packet_unref(&pkt);
		//		break;
		//	}
		//	out_stream = ec->streams[0];
		//	offset_pts = begin_pts;
		//}
		//else if (as && pkt.stream_index == as->index)
		//{
		//	cout << "audio:";
		//	out_stream = ec->streams[1];
		//	offset_pts = begin_audio_pts;
		//}
		//cout << pkt.pts << ":" << pkt.dts << ":" << pkt.size << endl;
		//
		//pkt.pts = av_rescale_q_rnd(pkt.pts- offset_pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.dts = av_rescale_q_rnd(pkt.dts- offset_pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.duration=av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		//写入音视频帧,清理pkt 
		mux.Write(&pkt);

	}
	//写入结尾
	mux.WriteEnd();
	demux.set_c(nullptr);
	mux.set_c(nullptr);
	return 0;
}
