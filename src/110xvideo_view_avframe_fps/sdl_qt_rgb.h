#pragma once

#include <QtWidgets/QWidget>
#include <thread>
#include "ui_sdl_qt_rgb.h"

class sdl_qt_rgb : public QWidget
{
    Q_OBJECT

public:
    sdl_qt_rgb(QWidget *parent = nullptr);
    ~sdl_qt_rgb() 
    {
        is_exit_ = true;
        //等待渲染线程退出
        th_.join();
    }
    void timerEvent(QTimerEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
    //线程函数用于刷新视频
    void Main();
    
signals:
    void ViewS();

public slots:
    void View();

private:
    std::thread th_;
    bool is_exit_ = false;
    Ui::sdl_qt_rgbClass ui;
};
