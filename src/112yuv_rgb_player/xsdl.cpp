#include "xsdl.h"
#include <SDL2/SDL.h>
#include <iostream>
#pragma comment(lib,"SDL2.lib")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

static bool InitVideo()
{
    static bool is_first = true;
    static std::mutex mux;
    std::unique_lock<std::mutex> sdl_lock(mux);
    if (!is_first) return true;
    is_first = false;
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }
    //设定缩放算法,线性插值
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    return true;
}

bool XSDL::Init(int w, int h, Format fmt)
{
    if (w <= 0 || h <= 0)return false;
    //初始化SDL
    InitVideo();
    std::unique_lock<std::mutex> sdl_lock(mtx_);
    width_ = w;
    height_ = h;
	fmt_ = fmt;
	//创建窗口
    if (!win_)
    {
		if (!win_id_)
		{
            //新建窗口
			win_ = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}
        else
        {
            //渲染到控件窗口
            win_ = SDL_CreateWindowFrom(win_id_);
        }
    }
    if (!win_)
    {
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }

    //创建渲染器
    if (render_) SDL_DestroyRenderer(render_);
    render_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED);
	if (!render_)
	{
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
    //创建材质
    unsigned int sdl_fmt = SDL_PIXELFORMAT_ARGB8888;
    switch (fmt)
    {
    case XVideoView::BGRA:
        sdl_fmt = SDL_PIXELFORMAT_RGBA32;
        break;
    case XVideoView::RGBA:
        sdl_fmt = SDL_PIXELFORMAT_BGRA32;
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
    if (texture_) SDL_DestroyTexture(texture_);
    texture_ = SDL_CreateTexture(render_, sdl_fmt, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!texture_)
	{
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}

    return true;
}

bool XSDL::Draw(const unsigned char* data, int linesize)
{
    if (!data)return false;
    std::unique_lock<std::mutex> sdl_lock(mtx_);
    if (!texture_ || !render_ || !win_ || width_ <= 0 || height_ <= 0)
    {
        std::cerr << "不满足!texture_ || !render_ || !win_ || width_ <= 0, height_ <= 0" << std::endl;
        return false;
    }
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
    //材质复制到渲染器
    SDL_Rect rect;
    SDL_Rect* prect = nullptr;
    if (scale_w_ > 0)//用户手动设置缩放
    {
		rect.x = 0;
		rect.y = 0;
		rect.w = scale_w_;
		rect.h = scale_h_;
        prect = &rect;
    }
    
    
    re = SDL_RenderCopy(render_, texture_, NULL, prect);
	if (re != 0)
	{
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
    SDL_RenderPresent(render_);

    return true;
}

bool XSDL::Draw(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch)
{
	if (!y||!u||!v)return false;
	std::unique_lock<std::mutex> sdl_lock(mtx_);
	if (!texture_ || !render_ || !win_ || width_ <= 0 || height_ <= 0)
	{
		std::cerr << "不满足!texture_ || !render_ || !win_ || width_ <= 0, height_ <= 0" << std::endl;
		return false;
	}
	//复制内存到显存
	auto re = SDL_UpdateYUVTexture(texture_, NULL, y,y_pitch,u,u_pitch,v,v_pitch);
	if (re != 0)
	{
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
	//清空屏幕
	SDL_RenderClear(render_);
	//材质复制到渲染器
	SDL_Rect rect;
	SDL_Rect* prect = nullptr;
	if (scale_w_ > 0)//用户手动设置缩放
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = scale_w_;
		rect.h = scale_h_;
		prect = &rect;
	}


	re = SDL_RenderCopy(render_, texture_, NULL, prect);
	if (re != 0)
	{
		std::cout << SDL_GetError() << std::endl;
		return false;
	}
	SDL_RenderPresent(render_);

	return true;
}

void XSDL::Close()
{
    std::unique_lock<std::mutex> sdl_lock(mtx_);
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (render_)
    {
        SDL_DestroyRenderer(render_);
        render_ = nullptr;
    }
    if (win_)
    {
        SDL_DestroyWindow(win_);
        win_ = nullptr;
    }
}

bool XSDL::IsExit()
{
    SDL_Event ev;
    SDL_WaitEventTimeout(&ev,1);
    if (ev.type == SDL_QUIT)
        return true;
    return false;
}
