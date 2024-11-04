﻿#pragma once
#include "xtools.h"
#include "xdecode.h"

class XDecodeTask:public XThread
{
public:
	/**
	 * 打开解码器.
	 * 
	 * \param para
	 * \return 
	 */
	bool Open(AVCodecParameters* para);
	//责任链处理函数
	void Do(AVPacket* pkt) override;
	//线程主函数
	void Main() override;

private:
	std::mutex mux_;
	XDecode decode_;
	XAVPacketList pkt_list_;
	AVFrame* frame_ = nullptr;//解码后存储
};

