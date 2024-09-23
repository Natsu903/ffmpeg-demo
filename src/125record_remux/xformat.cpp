#include "xformat.h"
#include <iostream>
#include <thread>
using namespace std;
extern "C" { //指定函数是c语言函数，函数名不包含重载标注
//引用ffmpeg头文件
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
//预处理指令导入库
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;
void XFormat::set_c(AVFormatContext* c)
{
    unique_lock<mutex> lock(mux_);
    if (c_) //清理原值
    {
        if (c_->oformat) //输出上下文
        {
            if (c_->pb)
                avio_closep(&c_->pb);
            avformat_free_context(c_);
        }
        else if (c_->iformat)  //输入上下文
        {
            avformat_close_input(&c_);
        }
        else
        {
            avformat_free_context(c_);
        }
    }
    c_ = c;
    if (!c_)return;
    //用于区分是否有音频或者视频流
    audio_index_ = -1; 
    video_index_ = -1;
    //区分音视频stream 索引
    for (int i = 0; i < c->nb_streams; i++)
    {
        //音频
        if (c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_index_ = i;
            audio_time_base_.den = c->streams[i]->time_base.den;
            audio_time_base_.num = c->streams[i]->time_base.num;
        }
        else if (c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_index_ = i;
            video_time_base_.den = c->streams[i]->time_base.den;
            video_time_base_.num = c->streams[i]->time_base.num;
            video_codec_id_ = c_->streams[i]->codecpar->codec_id;
        }
    }
}


/// <summary>
/// 复制参数 线程安全
/// </summary>
/// <param name="stream_index">对应c_->streams 下标</param>
/// <param name="dst">输出参数</param>
/// <returns>是否成功</returns>
bool XFormat::CopyPara(int stream_index, AVCodecParameters* dst)
{
    unique_lock<mutex> lock(mux_);
    if (!c_)
    {
        return false;
    }
    if (stream_index<0 || stream_index>c_->nb_streams)
        return false;
    auto re = avcodec_parameters_copy(dst, c_->streams[stream_index]->codecpar);
    if (re < 0)
    {
        return false;
    }
    return true;
}

bool XFormat::CopyPara(int stream_index, AVCodecContext* dts)
{
	unique_lock<mutex> lock(mux_);
	if (!c_)
	{
		return false;
	}
	if (stream_index<0 || stream_index>c_->nb_streams)
		return false;
	auto re = avcodec_parameters_to_context(dts, c_->streams[stream_index]->codecpar);
	if (re < 0)
	{
		return false;
	}
	return true;
}

bool XFormat::RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base)
{
    unique_lock<mutex> lock(mux_);
    if (!c_)return false;        
    auto out_stream = c_->streams[pkt->stream_index];
    AVRational in_time_base;
    in_time_base.num = time_base.num;
    in_time_base.den = time_base.den;
    pkt->pts = av_rescale_q_rnd(pkt->pts- offset_pts, in_time_base,
            out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX)
        );
    pkt->dts = av_rescale_q_rnd(pkt->dts- offset_pts, in_time_base,
        out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX)
    );
    pkt->duration = av_rescale_q(pkt->duration, in_time_base, out_stream->time_base);
    pkt->pos = -1;

    return true;
}