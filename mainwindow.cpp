#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (_client != nullptr)
    {
        delete _client;
    }
    if (_sender != nullptr)
    {
        delete _sender;
    }
}

void MainWindow::connect()
{
    if (_sender == nullptr)
    {
        std::thread(&MainWindow::run_sender, this).detach();
    }
    if (_client == nullptr)
    {
        std::thread(&MainWindow::run_client, this).detach();
    }
}

void MainWindow::run_client()
{
    _client = new Client(_context_c, ui->hostEdit->text().toStdString(), ui->portEdit->text().toStdString(), true);
    _context_c.run();
}

void MainWindow::run_sender()
{
    _sender = new Sender(_context_s, ui->hostEdit->text().toStdString(), ui->portEdit->text().toStdString(), true);
    _context_s.run();
}

void MainWindow::download()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd("Download");
    }
}

void MainWindow::decodeQR()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd("QR");
    }
}

void MainWindow::record()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd("RE");
    }
}

void MainWindow::stopRecord()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd("StopRE");
    }
}

void MainWindow::closeCamera()
{
    if (_sender != nullptr)
    {
        _sender->close();
        delete _sender;
        _sender = nullptr;
    }
    if (_client != nullptr)
    {
        _client->close();
        delete _client;
        _client = nullptr;
    }
}