#include "sdl_qt_rgb.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    sdl_qt_rgb w;
    w.show();
    return a.exec();
}
