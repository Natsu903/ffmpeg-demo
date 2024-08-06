#pragma once

#include <QtWidgets/QWidget>
#include "ui_testrgb.h"

class TestRGB : public QWidget
{
    Q_OBJECT

public:
    TestRGB(QWidget *parent = nullptr);
    //÷ÿ‘ÿªÊ÷∆∫Ø ˝
    void paintEvent(QPaintEvent* ev) override;
    ~TestRGB();

private:
    Ui::TestRGBClass ui;
};
