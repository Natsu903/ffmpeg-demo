#pragma once
/*****************************************************************//**
 * \file   XMux.h
 * \brief  媒体封装
 * 
 * \author Administrator
 * \date   October 2024
 *********************************************************************/
#include "xformat.h"
#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;

#define BERR(err) if(err!=0){PrintErr;return 0;}

class XCODEC_API XMux :public XFormat
{
public:
	/**
	 * 打开封装.
	 * 
	 * \param url 
	 * \return 
	 */
	static AVFormatContext* Open(const char* url,
		AVCodecParameters* video_para = nullptr, 
		AVCodecParameters* audio_para = nullptr);

	bool WriteHead();

	bool Write(AVPacket* pkt);

	bool WriteEnd();
	void set_Src_Video_Timebase(AVRational* tb);
	void set_Src_Audio_Timebase(AVRational* tb);
	~XMux();

private:
	AVRational* src_video_timebase_ = nullptr;
	AVRational* src_audio_timebase_ = nullptr;

	long long begin_video_pts_ = -1;//原视频开始时间
	long long begin_audio_pts_ = -1;//原音频开始时间
};

