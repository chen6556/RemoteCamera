#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Client.hpp"
#include "Sender.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void connect();
    void download();
    void decodeQR();
    void closeCamera();
    void refreshGraphicsViewSize();
    void record();
    void stopRecord();

private:
    Ui::MainWindow *ui;
    Client* _client = nullptr;
    Sender* _sender = nullptr;

    cv::Mat _frame;
    bool _running = false;

    void run_client();
    void run_sender();
    void refresh_graphicsView();
};
#endif // MAINWINDOW_H
