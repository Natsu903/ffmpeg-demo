#include <iostream>
#include <thread>
#include "xdemux.h"
#include "xmux.h"
#include "xdecode.h"
using namespace std;
extern "C" { //指定函数是c语言函数，函数名不包含重载标注
//引用ffmpeg头文件
#include <libavformat/avformat.h>
}
//预处理指令导入库
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
void PrintErr(int err)
{
    char buf[1024] = { 0 };
    av_strerror(err, buf, sizeof(buf) - 1);
    cerr << endl;
}
#define CERR(err) if(err!=0){ PrintErr(err);getchar();return -1;}

int main(int argc, char* argv[])
{
    
    /// 输入参数处理
    //使用说明
    string useage = "124_test_xformat 输入文件 输出文件 开始时间（秒） 结束时间（秒）\n";
    useage += "124_test_xformat v1080.mp4 test_out.mp4 10 20";
    cout << useage << endl;

    if (argc < 3)
    {
        return -1;
    }

    string in_file = argv[1];
    string out_file = argv[2];
    ////////////////////////////////////////////////////////////////////////////////////
    /// 截取10 ~ 20 秒之间的音频视频 取多不取少
    // 假定 9 11秒有关键帧 我们取第9秒
    int begin_sec = 0;    //截取开始时间
    int end_sec = 0;      //截取结束时间
    if (argc > 3)
        begin_sec = atoi(argv[3]);
    if (argc > 4)
        end_sec = atoi(argv[4]);


    ///////////////////////////////////////////////////////////////////////////






    //打开媒体文件
    const char* url = "v1080.mp4";
    ////////////////////////////////////////////////////////////////////////////////////
    /// 解封装
    //解封装输入上下文

    XDemux demux;
    auto demux_c = demux.Open(in_file.c_str());

    demux.set_c(demux_c);

    ////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////
    /// 封装
    //编码器上下文
    const char* out_url = "test_mux.mp4";

    XMux mux;
    auto mux_c = mux.Open(out_file.c_str());
    mux.set_c(mux_c);
    auto mvs = mux_c->streams[mux.video_index()]; //视频流信息
    auto mas = mux_c->streams[mux.audio_index()]; //视频流信息

    //有视频
    if (demux.video_index() >= 0)
    {
        mvs->time_base.num = demux.video_time_base().num;
        mvs->time_base.den = demux.video_time_base().den;

        //复制视频参数
        demux.CopyPara(demux.video_index(), mvs->codecpar);
        
    }
    //有音频
    if (demux.audio_index() >= 0)
    {
        mas->time_base.num = demux.audio_time_base().num;
        mas->time_base.den = demux.audio_time_base().den;
        //复制音频参数
        demux.CopyPara(demux.audio_index(), mas->codecpar);
    }

    mux.WriteHead();
   
    ////////////////////////////////////////////////////////////////////////////////////


    long long video_begin_pts = 0;
    long long audio_begin_pts = 0;  //音频的开始时间
    long long video_end_pts = 0;
    //开始截断秒数 算出输入视频的pts
    if (begin_sec > 0)
    {
        //计算视频的开始和结束播放pts 
        if (demux.video_index() >= 0 && demux.video_time_base().num>0)
        {
            double t = (double)demux.video_time_base().den / (double)demux.video_time_base().num;
            video_begin_pts = t * begin_sec;
            video_end_pts = t * end_sec;
            demux.Seek(video_begin_pts, demux.video_index()); //移动到开始帧
        }

        //计算音频的开始播放pts
        if (demux.audio_index() >= 0 && demux.audio_time_base().num > 0)
        {
            double t = (double)demux.audio_time_base().den / (double)demux.audio_time_base().num;
            audio_begin_pts = t * begin_sec;
        }

    }

    /**
     * 解码初始化.
     */
    XDecode decode;
    auto decode_c = decode.Create(demux.video_codec_id(),false);
    
    //设定视频解码器参数
    demux.CopyPara(demux.video_index(), decode_c);
    decode.set_c(decode_c);
    decode.Open();
    auto frame = decode.CreateFrame();//解码后的存储

    
    int audio_count = 0;
    int video_count = 0;
    double total_sec = 0;
    AVPacket pkt;
    for (;;)
    {
        if (!demux.Read(&pkt))
        {
            break;
        }

        // 视频 时间大于结束时间
        if (video_end_pts > 0 
            && pkt.stream_index == demux.video_index()
            && pkt.pts > video_end_pts)
        {
            av_packet_unref(&pkt);
            break;
        }

        if (pkt.stream_index == demux.video_index())
        {
            //解码视频
            if (decode.Send(&pkt))
            {
                while (decode.Recv(frame));
                {
                    cout<<"."<<flush;
                }
            }
            mux.RescaleTime(&pkt, video_begin_pts,demux.video_time_base());

            video_count++;
            if(demux.video_time_base().den>0)
                total_sec += pkt.duration * ((double)demux.video_time_base().num / (double)demux.video_time_base().den);
        }
        else if (pkt.stream_index == demux.audio_index())
        {
            mux.RescaleTime(&pkt, audio_begin_pts, demux.audio_time_base());
            audio_count++;
        }

        //写入音视频帧 会清理pkt
        mux.Write(&pkt);
    }

    //写入结尾 包含文件偏移索引
    mux.WriteEnd();
    demux.set_c(nullptr);
    mux.set_c(nullptr);
    cout << "输出文件" << out_file << ":" << endl;
    cout << "视频帧:" << video_count << endl;
    cout << "音频帧:" << audio_count << endl;
    cout << "总时长:" << total_sec << endl;

    return 0;
}

