#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"
#include "xdecodetask.h"
#include "xvideo_view.h"
#include "xmuxtask.h"

using namespace std;

//#define RTSP_URL "rtsp://127.0.0.1:8554/test"  //测试流
#define RTSP_URL "rtsp://admin:GZH&password@192.168.31.234:554/Streaming/Channels/101"	//监控流

int main(int argc, char* argv[])
{
	XDemuxTask demux_task;
	while (true)
	{
		if (demux_task.Open(RTSP_URL))
		{
			break;
		}
		MSleep(100);
		continue;
	}
	auto vpara = demux_task.CopyVideoPara();
	auto apara = demux_task.CopyAudioPara();
	AVCodecParameters* video_para = nullptr;
	AVCodecParameters* audio_para = nullptr;
	AVRational* video_timebase = nullptr;
	AVRational* audio_timebase = nullptr;
	if (vpara)
	{
		video_para = vpara->para;
		video_timebase = vpara->time_base;
	}
	if (apara)
	{
		audio_para = apara->para;
		audio_timebase = apara->time_base;
	}
	
	XMuxTask mux_task;
	if (!mux_task.Open("rtsp_out1.mp4", video_para, video_timebase, audio_para, audio_timebase))
	{
		LOGERROR("mux_task.Open Failed");
		return -1;
	}
	demux_task.set_next(&mux_task);
	demux_task.Start();
	mux_task.Start();
	MSleep(5000);
	mux_task.Stop();

	if (!mux_task.Open("rtsp_out2.mp4", video_para, video_timebase, audio_para, audio_timebase))
	{
		LOGERROR("mux_task.Open Failed");
		return -1;
	}
	mux_task.Start();
	MSleep(5000);
	mux_task.Stop();

	getchar();
	//system("pause");
	return 0;
}
