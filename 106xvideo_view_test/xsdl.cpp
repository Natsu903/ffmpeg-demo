#include "xsdl.h"
#include <SDL2/SDL.h>
#include <iostream>
#pragma comment(lib,"SDL2.lib")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

/**
 * 初始化SDL库
 * 
 * \return 是否初始化成功
 */
static bool InitVideo()
{
    static bool is_first = true;
    static std::mutex mux;
    std::unique_lock<std::mutex> sdl_lock(mux);
    if (!is_first) return true;
    else is_first = false;
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool XSDL::Init(int w, int h, Format fmt, void* win_id)
{
    if (w <= 0 || h <= 0) return false;
    //初始化SDL视频库
    InitVideo();
    //设定锁确定线程安全
    std::unique_lock<std::mutex> sdl_lock(mtx_);
    width_ = w;
    height_ = h;
    fmt_ = fmt;

    //创建窗口
    if (!win_)
    {
        //新建窗口
		if (!win_id)
		{
			win_ = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}
		else
		{
			//渲染到控件窗口
			win_ = SDL_CreateWindowFrom(win_id);
		}
    }
    if (!win_)
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }
    render_=SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED);
	if (!render_)
	{
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
    //创建材质
    unsigned int sdl_fmt = SDL_PIXELFORMAT_RGBA8888;
    switch (fmt)
    {
    case XVideoView::RGBA:
        break;
    case XVideoView::ARGB:
        sdl_fmt = SDL_PIXELFORMAT_ARGB32;
        break;
    case XVideoView::YUV420P:
        sdl_fmt = SDL_PIXELFORMAT_IYUV;
        break;
    default:
        break;
    }
    texture_ = SDL_CreateTexture(render_, sdl_fmt, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!texture_)
	{
		std::cout << SDL_GetError() << std::endl;
		return false;
	}

    return true;
}

bool XSDL::Draw(const unsigned char* data, int linesize)
{
    if (!data)return false;
    std::unique_lock<std::mutex> sdl_lock(mtx_);
    if (!texture_ || !render_ || !win_ || width_ <= 0 || height_ <= 0)return false;
    if (linesize <= 0)
    {
        switch (fmt_)
        {
        case XVideoView::RGBA:
        case XVideoView::ARGB:
            linesize = width_ * 4;
            break;
        case XVideoView::YUV420P:
            linesize = width_;
            break;
        default:
            break;
        }
    }
    if (linesize <= 0)return false;
    //复制内存到显存
    auto re = SDL_UpdateTexture(texture_, NULL, data, linesize);
    if (re != 0)
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }
    //清空屏幕
    SDL_RenderClear(render_);
    //复制材质到渲染器
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = width_;
    rect.h = height_;
    re = SDL_RenderCopy(render_, texture_, NULL, &rect);
    if (!re)
	{
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
    SDL_RenderPresent(render_);
    return true;
}
