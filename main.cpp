#include "mainwindow.h"
#include "OpenGLWidget.hpp"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    std::unique_ptr<OpenGLWidget> my3DOpenGLWidget;

    my3DOpenGLWidget = std::make_unique<OpenGLWidget>(&w);
    my3DOpenGLWidget->resize(w.width(), w.height());

    w.setCentralWidget(my3DOpenGLWidget.get());
    w.show();

    return a.exec();
}
