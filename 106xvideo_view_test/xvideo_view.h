#ifndef XVIDEO_VIEW_H
#define XVIDEO_VIEW_H
#include <mutex>
/*
	视频渲染接口类
	1.隐藏SDL实现
	2.渲染方案可代替
	3.线程安全
*/

class XVideoView
{
public:
	enum Format
	{
		RGBA = 0,
		ARGB,
		YUV420P
	};

	enum RenderType
	{
		SDL = 0
	};
	static XVideoView* Create(RenderType type=SDL);

	/**
	 * 初始化渲染窗口.
	 * 线程安全
	 * \param w 窗口宽度
	 * \param h 窗口高度
	 * \param fmt 绘制的像素格式
	 * \param win_id 窗口句柄，如果为空就创建新窗口
	 * \return 是否创建成功
	 */
	virtual bool Init(int w, int h, Format fmt = RGBA, void* win_id = nullptr) = 0;

	/**
	 * 渲染图像.
	 * 线程安全
	 * \param data 渲染的二进制图像
	 * \param linesize 一行数据的字节数，对于YUV420P就是Y一行字节数，
	 * 如果linesize<=0 就根据宽度和像素格式自动计算大小
	 * \return 渲染是否成功
	 */
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;

protected:
	int width_ = 0;//窗口宽高
	int height_ = 0;
	Format fmt_ = RGBA;//像素格式
	std::mutex mtx_;
};

#endif

