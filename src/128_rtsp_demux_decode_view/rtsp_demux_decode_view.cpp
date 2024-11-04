#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"
#include "xdecodetask.h"
#include "xvideo_view.h"

using namespace std;

//class TestThread :public XThread
//{
//	void Main() override
//	{
//		LOGDEBUG("TestThread Main() begin");
//		while (!is_exit_)
//		{
//			this_thread::sleep_for(1ms);
//		}
//		LOGDEBUG("TestThread Main() end");
//	}
//};

#define RTSP_URL "rtsp://127.0.0.1:8554/test"  //测试流
//#define RTSP_URL "rtsp://admin:GZH&password@192.168.31.234:554/Streaming/Channels/101"	//监控流

int main(int argc, char* argv[])
{
	//TestThread tt;
	//tt.Start();
	//this_thread::sleep_for(3s);
	//tt.Stop();
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
	auto para = demux_task.CopyVideoPara();
	//初始化渲染
	auto view = XVideoView::Create();
	view->Init(para->para);

	XDecodeTask decode_task;
	if (!decode_task.Open(para->para))
	{
		LOGERROR("open decode failed");
	}
	else
	{
		demux_task.set_next(&decode_task);
		demux_task.Start();
		decode_task.Start();
	}

	while (true)
	{
		auto f = decode_task.GetFrame();
		if (!f)
		{
			MSleep(1);
			continue;
		}
		view->DrawFrame(f);
		XFreeFrame(&f);
	}

	getchar();
	//system("pause");
	return 0;
}
