#ifndef XVIDEO_VIEW_H
#define XVIDEO_VIEW_H
#include <mutex>

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
	static XVideoView* Create(RenderType = SDL);


	/**
	 * 初始化渲染窗口 线程安全，可多次调用.
	 * 
	 * \param w 窗口宽度
	 * \param h 窗口高度
	 * \param fmt 绘制的像素格式
	 * \param win_id 窗口句柄，如果为空创建新窗口
	 * \return 是否创建成功
	 */
	virtual bool Init(int w, int h, Format fmt = RGBA, void* win_id = nullptr)=0;

	/**
	 * 渲染图像.
	 * 
	 * \param data 渲染的二进制数据
	 * \param linesize 一行数据的字节数，YUV420P就是Y一行字节数，
	 * 如果linesize<=0，根据宽度和像素格式自动计算大小
	 * \return 是否渲染成功
	 */
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;

	//清理所有申请的资源
	virtual void Close() = 0;

	//处理窗口退出事件
	virtual bool IsExit() = 0;

	//显示缩放
	void Scale(int w, int h)
	{
		scale_h_ = h;
		scale_w_ = w;
	}
	
protected:
	//材质宽高
	int width_ = 0;
	int height_ = 0;
	Format fmt_ = RGBA;
	std::mutex mtx_;
	//实际显示大小
	int scale_w_ = 0;
	int scale_h_ = 0;
};


#endif

