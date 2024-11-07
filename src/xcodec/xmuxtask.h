#pragma once
#include "xtools.h"
#include "xmux.h"
class XCODEC_API XMuxTask : public XThread
{
public:
	void Main() override;
	/**
	 * 打开封装文件.
	 * 
	 * \param url 输出地址
	 * \param video_para 视频参数
	 * \param video_time_base 视频时间基数
	 * \param audio_para 音频参数
	 * \param audio_time_base 音频时间基数
	 * \return 
	 */
	bool Open(const char* url, 
		AVCodecParameters* video_para = nullptr, AVRational* video_time_base = nullptr,
		AVCodecParameters* audio_para = nullptr, AVRational* audio_time_base = nullptr);

	void Do(AVPacket* pkt) override;

private:
	XMux xmux_;
	XAVPacketList pkts_;
	std::mutex mux_;
};

