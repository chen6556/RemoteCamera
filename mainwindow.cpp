#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <thread>
#include <chrono>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    _running = false;
    closeCamera();
    delete ui;
}

void MainWindow::connect()
{
    if (_sender == nullptr)
    {
        std::thread(&MainWindow::run_sender, this).detach();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    if (_client == nullptr)
    {
        std::thread(&MainWindow::run_client, this).detach();
    }  
    if (!_running)
    {
        _running = true;
        std::thread(&MainWindow::refresh_graphicsView, this).detach();
    }
}

void MainWindow::refreshGraphicsViewSize()
{
    if (_client != nullptr && !_client->frame().empty())
    {
        ui->imageLabel->resize(_frame.cols + 8, _frame.rows + 8);
    }
}

void MainWindow::run_client()
{
    boost::asio::io_context context;
    _client = new Client(context, ui->hostEdit->text().toStdString(), ui->portEdit->text().toStdString(), true);
    context.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void MainWindow::run_sender()
{
    boost::asio::io_context context;
    _sender = new Sender(context, ui->hostEdit->text().toStdString(), ui->portEdit->text().toStdString(), true);
    context.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void MainWindow::download()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd(0);
    }
}

void MainWindow::decodeQR()
{
    if (_client != nullptr)
    {
        _client->decode_QR();
    }
    // if (_sender != nullptr)
    // {
    //     _sender->get_cmd(3);
    // }
}

void MainWindow::closeCamera()
{
    _running = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (_sender != nullptr)
    {
        _sender->get_cmd(-2);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        delete _sender;
        _sender = nullptr;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (_client != nullptr)
    {
        _client->close();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        delete _client;
        _client = nullptr;
    } 
    ui->imageLabel->clear();
}

void MainWindow::refresh_graphicsView()
{
    while (_running)
    {
        if (_client == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }
        _frame = _client->frame();
        if (_frame.empty())
        {
            continue;
        }
        cv::cvtColor(_frame, _frame, cv::COLOR_BGR2RGB);
        
        ui->imageLabel->setPixmap( QPixmap::fromImage( QImage(_frame.data, _frame.cols, _frame.rows, QImage::Format::Format_RGB888)) );
         
        std::this_thread::sleep_for(std::chrono::milliseconds(17));
    }
}