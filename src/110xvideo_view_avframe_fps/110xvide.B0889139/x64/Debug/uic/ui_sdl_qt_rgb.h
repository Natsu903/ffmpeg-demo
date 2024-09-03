/********************************************************************************
** Form generated from reading UI file 'sdl_qt_rgb.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SDL_QT_RGB_H
#define UI_SDL_QT_RGB_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sdl_qt_rgbClass
{
public:
    QLabel *label;

    void setupUi(QWidget *sdl_qt_rgbClass)
    {
        if (sdl_qt_rgbClass->objectName().isEmpty())
            sdl_qt_rgbClass->setObjectName("sdl_qt_rgbClass");
        sdl_qt_rgbClass->resize(600, 400);
        label = new QLabel(sdl_qt_rgbClass);
        label->setObjectName("label");
        label->setGeometry(QRect(40, 30, 400, 300));

        retranslateUi(sdl_qt_rgbClass);

        QMetaObject::connectSlotsByName(sdl_qt_rgbClass);
    } // setupUi

    void retranslateUi(QWidget *sdl_qt_rgbClass)
    {
        sdl_qt_rgbClass->setWindowTitle(QCoreApplication::translate("sdl_qt_rgbClass", "sdl_qt_rgb", nullptr));
        label->setText(QCoreApplication::translate("sdl_qt_rgbClass", "video", nullptr));
    } // retranslateUi

};

namespace Ui {
    class sdl_qt_rgbClass: public Ui_sdl_qt_rgbClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDL_QT_RGB_H
