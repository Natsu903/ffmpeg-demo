#pragma once
/*****************************************************************//**
 * \file   XFormat.h
 * \brief  封装和解封装的基类
 * 
 * \author Administrator
 * \date   October 2024
 *********************************************************************/
#include <mutex>
#include "xtools.h"

struct AVFormatContext;
struct AVCodecParameters;
struct AVPacket;
struct AVCodecContext;
struct XRational
{
	int num;
	int den;
};
class XFormat
{
public:
	/**
	 * 复制参数 线程安全.
	 * 
	 * \param stream_index 对应c_->streams下标
	 * \param dst 输出参数
	 * \return 是否成功
	 */
	bool CopyPara(int stream_index,AVCodecParameters* dst);
	bool CopyPara(int stream_index, AVCodecContext* dst);

	//返回智能指针，复制视频参数
	std::shared_ptr<XPara> CopyVideoPara();

	/**
	 * 设置上下文，并且清理上次设置的值，如果传递NULL,相当于关闭上下文.
	 * 
	 * \param AVFormatContext* c
	 */
	void set_c(AVFormatContext* c);
	int audio_index() { return audio_index_; }
	int video_index() { return video_index_; }
	XRational video_time_base() { return video_time_base_; }
	XRational audio_time_base() { return audio_time_base_; }
	//根据timebase换算时间
	bool RescaleTime(AVPacket* pkt,long long offset_pts, XRational time_base);
	int video_codec_id() { return video_codec_id_; }

	//判断是否超时
	bool isTimeout()
	{
		if (NowMs() - last_time_ > timeout_ms_)//超时
		{
			last_time_ = NowMs();
			is_connected_ = false;
			return true;
		}
		return false;
	}

	//设定超时时间
	void set_timeout_ms(int ms);

	bool is_connected() { return is_connected_; }

protected:
	int timeout_ms_ = 0;			//超时时间（ms）
	long long last_time_ = 0;		//上次接受到数据的时间
	bool is_connected_ = false;		//是否连接成功
	AVFormatContext* c_=nullptr;	//封装解封装上下文
	std::mutex mux_;				//资源互斥
	int video_index_ = 0;			//video在stream中的索引
	int audio_index_ = 1;			//audio在stream中的索引
	XRational video_time_base_ = { 1,25 };
	XRational audio_time_base_ = { 1,9000 };
	int video_codec_id_ = 0;		//编码器id
};

