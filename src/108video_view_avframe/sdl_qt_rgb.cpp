#include "sdl_qt_rgb.h"
#include <fstream>
#include <iostream>
#include <QMessageBox>
#include "xvideoview.h"
extern "C"
{
#include <libavcodec/avcodec.h>
}

#pragma comment(lib,"avutil.lib")

using namespace std;

static int sdl_w = 0;
static int sdl_h = 0;
static int pix_size = 2;
static ifstream yuv_file;
static XVideoView* view=nullptr;
static AVFrame* frame = nullptr;

void sdl_qt_rgb::timerEvent(QTimerEvent* ev)
{
	//yuv_file.read((char*)yuv, sdl_w * sdl_h * 1.5);
	yuv_file.read((char*)frame->data[0], sdl_w * sdl_h);//Y
	yuv_file.read((char*)frame->data[1], sdl_w * sdl_h/4);//U
	yuv_file.read((char*)frame->data[2], sdl_w * sdl_h/4);//V
	if (view->IsExit())
	{
		view->Close();
		exit(0);
	}
	view->DrawFrame(frame);
	//view->Draw(yuv);
}

void sdl_qt_rgb::resizeEvent(QResizeEvent* ev)
{
	ui.label->resize(size());
	ui.label->move(0, 0);
	//view->Scale(width(), height());
}


sdl_qt_rgb::sdl_qt_rgb(QWidget *parent)
    : QWidget(parent)
{
	//二进制方式打开yuv文件
	yuv_file.open("400_300_25.yuv", ios::binary);
	if (!yuv_file)
	{
		QMessageBox::information(this, "", "open yuv file failed");
		return;
	}
    ui.setupUi(this);
	sdl_w = 400;
	sdl_h = 300;
	ui.label->resize(sdl_w, sdl_h);
	view = XVideoView::Create();
	view->Init(sdl_w, sdl_h, XVideoView::YUV420P);
	view->Close();
	view->Init(sdl_w, sdl_h, XVideoView::YUV420P, (void*)ui.label->winId());

	//生成frame对象
	frame = av_frame_alloc();
	frame->width = sdl_w;
	frame->height = sdl_h;
	frame->format = AV_PIX_FMT_YUV420P;
	frame->linesize[0] = sdl_w;
	frame->linesize[1] = sdl_w/2;
	frame->linesize[2] = sdl_w/2;
	//生成图像空间默认32字节对齐
	auto re = av_frame_get_buffer(frame, 0);
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf));
		std::cerr << buf << std::endl;
	}
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb()
{}

