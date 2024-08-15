#pragma once

#include <QtWidgets/QWidget>
#include "ui_sdl_qt_rgb.h"

class sdl_qt_rgb : public QWidget
{
    Q_OBJECT

public:
    sdl_qt_rgb(QWidget *parent = nullptr);
    ~sdl_qt_rgb();
    void timerEvent(QTimerEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
private:
    Ui::sdl_qt_rgbClass ui;
};
