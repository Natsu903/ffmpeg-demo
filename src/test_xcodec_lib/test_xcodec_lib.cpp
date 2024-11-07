#include <iostream>
#include "xdemux_task.h"
using namespace std;

int main(int argc ,char* argv[])
{
	cout << "xcodec lib test" << endl;
	XDemuxTask demux_task;
	demux_task.Open("v1080.mp4");
	demux_task.Start();
	getchar();
	return 0;
}
