#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
    if (_client != nullptr)
    {
        return;
    }
    
}