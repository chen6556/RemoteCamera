#include "mainwindow.h"

#include <QStyleFactory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("windowsvista"));

    MainWindow w;
    
    w.show();
    
    return a.exec();
}
