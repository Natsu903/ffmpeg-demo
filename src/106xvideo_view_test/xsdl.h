﻿#pragma once
#include "xvideo_view.h"
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
class XSDL :public XVideoView
{
	/**
	 * 初始化渲染窗口.
	 * 线程安全
	 * \param w 窗口宽度
	 * \param h 窗口高度
	 * \param fmt 绘制的像素格式
	 * \param win_id 窗口句柄，如果为空就创建新窗口
	 * \return 是否创建成功
	 */
	bool Init(int w, int h, Format fmt = RGBA, void* win_id = nullptr) override;

	/**
	 * 渲染图像.
	 * 线程安全
	 * \param data 渲染的二进制图像
	 * \param linesize 一行数据的字节数，对于YUV420P就是Y一行字节数，
	 * 如果linesize<=0 就根据宽度和像素格式自动计算大小
	 * \return 渲染是否成功
	 */
	bool Draw(const unsigned char* data, int linesize = 0) override;

private:
	SDL_Window* win_ = nullptr;
	SDL_Renderer* render_ = nullptr;
	SDL_Texture* texture_ = nullptr;
};
