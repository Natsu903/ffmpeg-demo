#ifndef XVIDEO_VIEW_H
#define XVIDEO_VIEW_H
#include <mutex>
#include <fstream>

struct AVFrame;

void MSleep(unsigned int ms);

long long NowMs();

class XVideoView
{
public:
	enum Format//枚举的值与ffmpeg一致
	{
		YUV420P = 0,
		NV12 = 23,
		ARGB = 25,
		RGBA = 26,
		BGRA = 28
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
	 * \return 是否创建成功
	 */
	virtual bool Init(int w, int h, Format fmt = RGBA)=0;

	/**
	 * 渲染图像.
	 * 
	 * \param data 渲染的二进制数据
	 * \param linesize 一行数据的字节数，YUV420P就是Y一行字节数，
	 * 如果linesize<=0，根据宽度和像素格式自动计算大小
	 * \return 是否渲染成功
	 */
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;
	virtual bool Draw(const unsigned char* y, int y_pitch, const unsigned char* u, int u_pitch, const unsigned char* v, int v_pitch) = 0;

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
	
	bool DrawFrame(AVFrame* frame);

	int render_fps(){return render_fps_;}

	//打开文件
	bool Open(std::string filepath);

	/**
	 * .
	 * 读取一帧数据，并维护AVFrame空间
	 * 每次调用会覆盖上一次数据
	 */
	AVFrame* Read();

	void set_win_id(void* win) { win_id_ = win; }
	virtual ~XVideoView();

protected:
	void* win_id_ = nullptr;//窗口句柄
	int render_fps_ = 0;//显示帧率
	//材质宽高
	int width_ = 0;
	int height_ = 0;
	Format fmt_ = RGBA;
	std::mutex mtx_;
	//实际显示大小
	int scale_w_ = 0;
	int scale_h_ = 0;
	long long beg_ms_ = 0;//计时开始时间
	int count_ = 0;//统计显示次数

private:
	std::ifstream ifs_;
	AVFrame* frame_ = nullptr;
	unsigned char* cache_ = nullptr;//用来复制NV12的缓冲
};


#endif

