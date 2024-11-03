#include <iostream>
#include <thread>
#include "xtools.h"
#include "xdemux_task.h"

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

//#define RTSP_URL "rtsp://127.0.0.1:8554/test"  //测试流
#define RTSP_URL "rtsp://admin:GZH&password@192.168.31.234:554/Streaming/Channels/101"	//监控流

int main(int argc, char* argv[])
{
	//TestThread tt;
	//tt.Start();
	//this_thread::sleep_for(3s);
	//tt.Stop();
	XDemuxTask det;
	while (true)
	{
		if (det.Open(RTSP_URL))
		{
			break;
		}
		MSleep(100);
		continue;
	}
	det.Open(RTSP_URL);
	det.Start();
	system("pause");
	return 0;
}
