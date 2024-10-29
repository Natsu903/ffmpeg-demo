#pragma once
/*****************************************************************//**
 * \file   XMux.h
 * \brief  媒体封装
 * 
 * \author Administrator
 * \date   October 2024
 *********************************************************************/
#include "XFormat.h"
#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;

static void PrintErr(int err)
{
	char buf[1024] = { 0 };
	av_strerror(err, buf, sizeof(buf) - 1);
	cerr << buf << endl;
}

#define BERR(err) if(err!=0){PrintErr;return 0;}

class XMux :public XFormat
{
public:
	/**
	 * 打开封装.
	 * 
	 * \param url 
	 * \return 
	 */
	static AVFormatContext* Open(const char* url);

	bool WriteHead();

	bool Write(AVPacket* pkt);

	bool WriteEnd();
};

