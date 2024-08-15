#include "sdl_qt_rgb.h"
#include <fstream>
#include <QMessageBox>
#include "xvideoview.h"

using namespace std;

static int sdl_w = 0;
static int sdl_h = 0;
static unsigned char* yuv = nullptr;
static int pix_size = 2;
static ifstream yuv_file;
static XVideoView* view=nullptr;

void sdl_qt_rgb::timerEvent(QTimerEvent* ev)
{
	yuv_file.read((char*)yuv, sdl_w * sdl_h * 1.5);
	view->Draw(yuv);
}

void sdl_qt_rgb::resizeEvent(QResizeEvent* ev)
{
	ui.label->resize(size());
	ui.label->move(0, 0);
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
	view->Init(sdl_w, sdl_h, XVideoView::YUV420P,(void*)ui.label->winId());

	yuv = new unsigned char[sdl_w * sdl_h * pix_size];
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb()
{}

