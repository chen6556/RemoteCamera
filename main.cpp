#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    boost::asio::io_context context;
    w.show();
    context.run();
    return a.exec();
}
