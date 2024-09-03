#include "sdl_qt_rgb.h"
#include <fstream>
#include <iostream>
#include <QMessageBox>
#include <QSpinBox>
#include "xvideoview.h"
#include <thread>
#include <sstream>
#include <QFileDialog>
#include <vector>

#pragma comment(lib,"avutil.lib")

using namespace std;

static std::vector<XVideoView*> views;

void sdl_qt_rgb::timerEvent(QTimerEvent* ev)
{

}

void sdl_qt_rgb::resizeEvent(QResizeEvent* ev)
{

}

void sdl_qt_rgb::View()
{

}

void sdl_qt_rgb::Main()
{
	while (!is_exit_)
	{
		ViewS();
		MSleep(1);
	}
}

void sdl_qt_rgb::Open1()
{
	Open(0);
}

void sdl_qt_rgb::Open2()
{
	Open(1);
}

void sdl_qt_rgb::Open(int i)
{
	QFileDialog fd;
	auto filename = fd.getOpenFileName();
	if (filename.isEmpty())return;
	cout << filename.toLocal8Bit().data() << endl;
	//打开文件
	if (views[i]->Open(filename.toStdString()))
	{
		return;
	}
	int w = 0;
	int h = 0;
	QString pix = 0;//像素格式
	if (i == 0)
	{
		w = ui.width1->value();
		h = ui.height1->value();
		pix = ui.pix1->currentText();
	}
	else
	{
		w = ui.width2->value();
		h = ui.height2->value();
		pix = ui.pix2->currentText();
	}
	XVideoView::Format fmt = XVideoView::YUV420P;
	if (pix == "YUV420P") {}
	else if (pix == "RGBA")
	{
		fmt = XVideoView::RGBA;
	}
	else if (pix == "ARGB")
	{
		fmt = XVideoView::ARGB;
	}
	else if (pix == "BGRA")
	{
		fmt = XVideoView::BGRA;
	}

	//初始化窗口和材质
	views[i]->Init(w, h, fmt);
}


sdl_qt_rgb::sdl_qt_rgb(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	connect(this, SIGNAL(ViewS()), this, SLOT(View()));

	views.push_back(XVideoView::Create());
	views.push_back(XVideoView::Create());
	views[0]->set_win_id((void*)ui.video1->winId());
	views[0]->set_win_id((void*)ui.video2->winId());


    //startTimer(10);
	th_ = std::thread(&sdl_qt_rgb::Main,this);
}

