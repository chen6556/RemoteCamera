#include "ui/pathdialog.h"
#include "./ui_pathdialog.h"
#include <boost/filesystem.hpp>

PathDialog::PathDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PathDialog)
{
    ui->setupUi(this);
    boost::property_tree::read_json("./config.json", _config);
    ui->videoPathEdit->setText(_config.get<std::string>("video_path").c_str());
    ui->framePathEdit->setText(_config.get<std::string>("frame_path").c_str());
}

PathDialog::~PathDialog()
{
    delete ui;
}

void PathDialog::accept()
{
    char modified = 0;
    std::string video_path = ui->videoPathEdit->text().toStdString();
    if (_config.get<std::string>("video_path") != video_path)
    {
        if (boost::filesystem::is_directory(video_path) || !boost::filesystem::is_regular_file(video_path))
        {
            modified += 3;
        }
    }

    std::string frame_path = ui->framePathEdit->text().toStdString();
    if (_config.get<std::string>("frame_path") != frame_path)
    {
        if (boost::filesystem::is_directory(frame_path) || !boost::filesystem::is_regular_file(frame_path))
        {
            modified += 4;
        }
    }

    switch (modified)
    {
    case 3:
        emit pathChanged(3, ui->videoPathEdit->text());
        break;
    case 4:
        emit pathChanged(4, ui->framePathEdit->text());
        break;
    case 7:
        emit pathChanged(7, ui->videoPathEdit->text() + '|' + ui->framePathEdit->text());
        break;
    default:
        break;
    }
    QDialog::accept();
}

const std::string PathDialog::frame_path() const
{
    return _config.get<std::string>("frame_path");
}

const std::string PathDialog::video_path() const
{
    return _config.get<std::string>("video_path");
}