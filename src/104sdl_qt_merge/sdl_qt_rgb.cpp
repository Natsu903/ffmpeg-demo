#include "sdl_qt_rgb.h"
#include <SDL2/SDL.h>
#include <QMessageBox>
#pragma comment(lib,"SDL2.lib")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

static SDL_Window* sdl_win = nullptr;
static SDL_Renderer* sdl_renderer = nullptr;
static SDL_Texture* sdl_texture = nullptr;
static int sdl_w = 0;
static int sdl_h = 0;
static unsigned char* rgb = nullptr;
static int pix_size = 4;


sdl_qt_rgb::sdl_qt_rgb(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    sdl_w = ui.label->width();
    sdl_h = ui.label->height();
    SDL_Init(SDL_INIT_VIDEO);
    sdl_win = SDL_CreateWindowFrom((void*)ui.label->winId());
    sdl_renderer = SDL_CreateRenderer(sdl_win, -1,SDL_RENDERER_ACCELERATED);
	//读取两幅图像信息，修改新图像的大小
	QImage img1(":/png/1.png");
	QImage img2(":/png/2.png");
	if (img1.isNull() || img2.isNull())
	{
		QMessageBox::information(this, "", "open iamge fail");
		return;
	}
	int out_w = img1.width() + img2.width();
	int out_h = (img1.height() > img2.height()) ? img1.height() : img2.height();
	sdl_w = out_w;
	sdl_h = out_h;
	resize(sdl_w, sdl_h);
	//修改label位置和大小与新图像一致
	ui.label->move(0, 0);
	ui.label->resize(sdl_w, sdl_h);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, sdl_w, sdl_h);
    rgb = new unsigned char[sdl_w * sdl_h * pix_size];
	//默认设置为透明
	memset(rgb, 0, sdl_w * sdl_h * pix_size);
	//合并两幅图像
	QImage low_height_img = (img1.height() < img2.height()) ? img1 : img2;
	QImage up_height_img = (img1.height() < img2.height()) ? img2 : img1;
	for (int i = 0; i < sdl_h; i++)
	{
		int b = i * sdl_w * pix_size;
		if (i < low_height_img.height())
		{
			memcpy(rgb + b, low_height_img.scanLine(i), low_height_img.width() * pix_size);
		}
		b += low_height_img.width() * pix_size;
		if (i < up_height_img.height())
		{
			memcpy(rgb + b, up_height_img.scanLine(i), up_height_img.width() * pix_size);
		}
	}
	QImage out(rgb, sdl_w, sdl_h, QImage::Format_ARGB32);
	out.save("out.png");
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb()
{}

void sdl_qt_rgb::timerEvent(QTimerEvent * ev)
{
	SDL_UpdateTexture(sdl_texture, NULL, rgb, sdl_w * pix_size);
	SDL_RenderClear(sdl_renderer);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = sdl_w;
	rect.h = sdl_h;
	SDL_RenderCopy(sdl_renderer,sdl_texture,NULL,&rect);
	SDL_RenderPresent(sdl_renderer);
}
