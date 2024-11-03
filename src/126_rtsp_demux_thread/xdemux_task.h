#pragma once
#include "xtools.h"
#include "XDemux.h"
class XDemuxTask :public XThread
{
public:
	void Main() override;

	/**
	 * 打开解封装.
	 * 
	 * \param url rtsp地址
	 * \param timeout_ms 超时时间，单位ms
	 * \return 
	 */
	bool Open(std::string url,int timeout_ms=1000);

private:
	XDemux demux_;
	std::string url_;
	int timeout_ms_ = 0;//超时时间
};

