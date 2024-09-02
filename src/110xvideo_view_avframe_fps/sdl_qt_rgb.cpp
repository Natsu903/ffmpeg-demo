#include "sdl_qt_rgb.h"
#include <fstream>
#include <iostream>
#include <QMessageBox>
#include <QSpinBox>
#include "xvideoview.h"
#include <thread>
#include <sstream>
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
static long long file_size = 0;
static QLabel* view_fps = nullptr;//显示fps控件
static QSpinBox* set_fps=nullptr;//设置fps控件
int fps = 25;//播放帧率

void sdl_qt_rgb::timerEvent(QTimerEvent* ev)
{
	//yuv_file.read((char*)yuv, sdl_w * sdl_h * 1.5);
	yuv_file.read((char*)frame->data[0], sdl_w * sdl_h);//Y
	yuv_file.read((char*)frame->data[1], sdl_w * sdl_h / 4);//U
	yuv_file.read((char*)frame->data[2], sdl_w * sdl_h / 4);//V
	yuv_file.gcount();

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

void sdl_qt_rgb::View()
{
	yuv_file.read((char*)frame->data[0], sdl_w * sdl_h);//Y
	yuv_file.read((char*)frame->data[1], sdl_w * sdl_h / 4);//U
	yuv_file.read((char*)frame->data[2], sdl_w * sdl_h / 4);//V
	//读到文件结尾回到开头
	if (yuv_file.tellg() == file_size)
	{
		yuv_file.seekg(0, ios::beg);
	}
	if (view->IsExit())
	{
		view->Close();
		exit(0);
	}
	view->DrawFrame(frame);
	stringstream ss;
	ss << "fps:" << view->render_fps();
	view_fps->setText(ss.str().c_str());
	fps = set_fps->value();//拿到播放帧率

}

void sdl_qt_rgb::Main()
{
	while (!is_exit_)
	{
		ViewS();
		if (fps > 0)
		{
			MSleep(1000 / fps);
		}
		else
			MSleep(10);
	}
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
	yuv_file.seekg(0, ios::end);
	file_size = yuv_file.tellg();
	yuv_file.seekg(0, ios::beg);

    ui.setupUi(this);

	connect(this, SIGNAL(ViewS()), this, SLOT(View()));

	//显示fps控件
	view_fps = new QLabel(this);
	view_fps->setText("100fps");

	//设置fps控件
	set_fps = new QSpinBox(this);
	set_fps->move(200, 0);
	set_fps->setValue(25);
	set_fps->setRange(1, 300);

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
    //startTimer(10);
	th_ = std::thread(&sdl_qt_rgb::Main,this);
}

