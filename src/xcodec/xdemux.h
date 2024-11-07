#pragma once
#include "xformat.h"

class XCODEC_API XDemux : public XFormat
{
public:
	/**
	 * 打开解封装.
	 * 
	 * \param url 解封装地址，支持rtsp
	 * \return 失败返回nullptr
	 */
	static AVFormatContext* Open(const char* url);

	/**
	 * 读取一帧数据.
	 * 
	 * \param pkt 读取数据
	 * \return 是否成功
	 */
	bool Read(AVPacket* pkt);

	bool Seek(long long pts,int stream_index);
};

