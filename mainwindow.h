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
    void record();
    void stopRecord();
    void closeCamera();

private:
    Ui::MainWindow *ui;
    Client* _client = nullptr;
    Sender* _sender = nullptr;
    boost::asio::io_context _context_c;
    boost::asio::io_context _context_s;

    void run_client();
    void run_sender();
};
#endif // MAINWINDOW_H
