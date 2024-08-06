#include "sdl_qt_rgb.h"
#include <SDL2/SDL.h>
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
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, sdl_w, sdl_h);
    rgb = new unsigned char[sdl_w * sdl_h * 4];
    startTimer(10);
}

sdl_qt_rgb::~sdl_qt_rgb()
{}

void sdl_qt_rgb::timerEvent(QTimerEvent * ev)
{
	for (int j = 0; j < sdl_h; j++)
	{
		int b = j * sdl_w * pix_size;
		for (int i = 0; i < sdl_w * pix_size; i += pix_size)
		{
			rgb[b + i] = 0;
			rgb[b + i + 1] = 0;
			rgb[b + i + 2] = 255;
			rgb[b + i + 3] = 255;
		}
	}
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
