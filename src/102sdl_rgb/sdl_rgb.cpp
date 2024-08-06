#include <iostream>
#include <SDL2/SDL.h>
using namespace std;
#pragma comment(lib, "SDL2.lib")
#undef main

int main(int argc, char* argv[])
{
	int w = 800;
	int h = 600;
	//初始化SDL video库
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		cout << SDL_GetError() << endl;
		return -1;
	}
	//生成SDL窗口
	auto screen = SDL_CreateWindow("test sdl ffmpeg",0,0, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		cout << SDL_GetError() << endl;
		return -1;
	}
	//生成渲染器
	auto render = SDL_CreateRenderer(screen, NULL,SDL_RENDERER_ACCELERATED);
	if (!render)
	{
		cout << SDL_GetError() << endl;
		return -1;
	}
	//生成材质
	auto texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!texture)
	{
		cout << SDL_GetError() << endl;
		return -1;
	}
	//存放图像数据
	shared_ptr<unsigned char> rgb(new unsigned char[w * h * 4]);
	auto r = rgb.get();
	for (int j = 0; j < h; j++)
	{
		int b = j * w * 4;
		for (int i = 0; i < w*4; i += 4)
		{
			r[b + i] = 0;
			r[b + i + 1] = 0;
			r[b + i + 2] = 255;
			r[b + i + 3] = 255;
		}
	}
	//将内存数据写入材质
	SDL_UpdateTexture(texture, NULL, r, w * 4);

	//清理屏幕
	SDL_RenderClear(render);
	SDL_Rect sdl_rect;
	sdl_rect.x = 0;
	sdl_rect.y = 0;
	sdl_rect.w = w;
	sdl_rect.h = h;
	//复制材质到渲染器
	SDL_RenderCopy(render, texture, NULL, &sdl_rect);
	//渲染
	SDL_RenderPresent(render);
	getchar();
	return 0;
}