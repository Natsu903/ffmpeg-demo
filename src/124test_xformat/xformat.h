#pragma once

/**
 * 封装和解封装基类
 */ 

#include <mutex>
struct AVFormatContext;
struct AVCodecParameters;
struct AVPacket;
struct XRational
{
	int num;
	int den;
};

class XFormat
{
public:
	/**
	 * 复制参数（线程安全）.
	 * 
	 * \param stream_index 对应c_->streams 下标
	 * \param dst 输出参数
	 * \return 是否成功
	 */
	bool CopyPara(int stream_index, AVCodecParameters* dst);

	/**
	 * 设置上下文，并且清理上次设置的值，如果传递NULL，相当于关闭上下文.
	 * 
	 * \param c
	 */
	void set_c(AVFormatContext* c);

	int audio_index() { return audio_index_; }
	int video_index() { return video_index_; }
	XRational video_time_base(){ return video_time_base_; }
	XRational audio_time_base(){ return audio_time_base_; }

protected:
	AVFormatContext* c_;//封装解封装上下文
	std::mutex mux_;//c_资源互斥
	int video_index_ = 0;
	int audio_index_ = 1;
	XRational video_time_base_ = { 1,25 };
	XRational audio_time_base_ = { 1,9000 };
};

