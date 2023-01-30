#include "ui/mainwindow.h"
#include "./ui_mainwindow.h"
#include <thread>
#include <chrono>
#include <QDesktopServices>
#include <filesystem>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include "Dongle/Dongle.hpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    boost::property_tree::read_json("./config.json", _config);
    ui->setupUi(this);
    ui->hostEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))*$")));
    ui->portEdit->setValidator(new QIntValidator(0, 65536));
    if ( !Dongle().verify(_config.get<std::string>("passwd")) )
    {
        ActivateDialog* dialog = new ActivateDialog();
        dialog->exec();
        if (dialog->alive())
        {
            _config.put("passwd", dialog->passwd());
            boost::property_tree::write_json("./config.json", _config);
            delete dialog;
        }
        else
        {
            delete dialog;
            exit(0);
        }
    }
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
    _client->set_video_path(_config.get<std::string>("video_path"));
    context.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void MainWindow::run_sender()
{
    boost::asio::io_context context;
    _sender = new Sender(context, ui->hostEdit->text().toStdString(), ui->portEdit->text().toStdString(), true);
    _sender->set_frame_path(_config.get<std::string>("frame_path"));   
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
    if (_sender != nullptr)
    {
        _sender->get_cmd(3);
    }
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

void MainWindow::record()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd(1);
    }
}

void MainWindow::stopRecord()
{
    if (_sender != nullptr)
    {
        _sender->get_cmd(2);
    }
}

void MainWindow::editPath()
{
    PathDialog* dialog = new PathDialog();
    QObject::connect(dialog, &PathDialog::pathChanged, this, &MainWindow::refreshPath);
    dialog->exec();
    this->disconnect(dialog);
    delete dialog;
    dialog = nullptr;
}

void MainWindow::openVideos()
{
    if (!std::filesystem::exists(_config.get<std::string>("video_path")))
    {
        std::filesystem::create_directory(_config.get<std::string>("video_path"));
    }
    QDesktopServices::openUrl(QUrl( 
                                    std::filesystem::canonical(_config.get<std::string>("video_path")).generic_string().c_str(), 
                                    QUrl::TolerantMode)); 
}

void MainWindow::openFrames()
{
    if (!std::filesystem::exists(_config.get<std::string>("frame_path")))
    {
        std::filesystem::create_directory(_config.get<std::string>("frame_path"));
    }
    QDesktopServices::openUrl(QUrl(
                                    std::filesystem::canonical(_config.get<std::string>("frame_path")).generic_string().c_str(),
                                    QUrl::TolerantMode));
}

void MainWindow::refreshPath(const char flag, QString path)
{
    QStringList paths = path.split('|');
    switch (flag)
    {
    case 3:
        _config.put("video_path", path.toStdString());
        break;
    case 4:
        _config.put("frame_path", path.toStdString());
        break;
    case 7:
        _config.put("video_path", paths[0].toStdString());
        _config.put("frame_path", paths[1].toStdString());
        break;
    default:
        break;
    }
    boost::property_tree::write_json("./config.json", _config);
}