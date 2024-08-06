#include <iostream>
//指定函数是c语言，函数名不包含重载标注
extern "C" 
{
#include <libavcodec/avcodec.h>
}

using namespace std;

//预处理指令导入库
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"SDL2.lib")

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;
    cout << avcodec_configuration() << endl;

    //创建frame对象
    auto frame1 = av_frame_alloc();

    //图像设定
    frame1->width = 400;
    frame1->height = 300;
    frame1->format = AV_PIX_FMT_ARGB;

    //分配空间 16字节对齐
    auto re = av_frame_get_buffer(frame1, 16);
    if (re!=0)
    {
        char buf[1024] = { 0 };
        av_strerror(re, buf, sizeof(buf));
        cout << buf << endl;
    }
    cout << "frame1->linesize[0]=" << frame1->linesize[0] << endl;
    if (frame1->buf[0])
    {
        cout << "frame1 ref cout=" << av_buffer_get_ref_count(frame1->buf[0]) << endl;
    }

    auto frame2 = av_frame_alloc();
    av_frame_ref(frame2, frame1);
    cout << "frame1 ref cout=" << av_buffer_get_ref_count(frame1->buf[0]) << endl;
    cout << "frame1 ref cout=" << av_buffer_get_ref_count(frame2->buf[0]) << endl;

    //释放frame对象空间，buf的引用减一
    av_frame_free(&frame2);
	cout << "av_frame_free(&frame1);" << endl;
	cout << "frame1 ref cout=" << av_buffer_get_ref_count(frame2->buf[0]) << endl;
    return 0;
}