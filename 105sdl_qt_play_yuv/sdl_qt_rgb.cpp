#include "sdl_qt_rgb.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <QMessageBox>
#pragma comment(lib,"SDL2.lib")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

using namespace std;

static SDL_Window* sdl_win = nullptr;
static SDL_Renderer* sdl_renderer = nullptr;
static SDL_Texture* sdl_texture = nullptr;
static int sdl_w = 0;
static int sdl_h = 0;
static unsigned char* yuv = nullptr;
static int pix_size = 2;
static ifstream yuv_file;


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
    SDL_Init(SDL_INIT_VIDEO);
    sdl_win = SDL_CreateWindowFrom((void*)ui.label->winId());
    sdl_renderer = SDL_CreateRenderer(sdl_win, -1,SDL_RENDERER_ACCELERATED);
	//修改材质支持yuv
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, sdl_w, sdl_h);
	yuv = new unsigned char[sdl_w * sdl_h * pix_size];
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb()
{}

void sdl_qt_rgb::timerEvent(QTimerEvent * ev)
{
	yuv_file.read((char*)yuv, sdl_w * sdl_h * 1.5);
	//调整为一行y的字节数
	SDL_UpdateTexture(sdl_texture, NULL, yuv, sdl_w);
	SDL_RenderClear(sdl_renderer);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = sdl_w;
	rect.h = sdl_h;
	SDL_RenderCopy(sdl_renderer,sdl_texture,NULL,&rect);
	SDL_RenderPresent(sdl_renderer);
}
