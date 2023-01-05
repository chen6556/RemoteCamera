#include "dialog.h"
#include "./ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    boost::property_tree::read_json("./config.json", _config);
    ui->videoPathEdit->setText(_config.get<std::string>("video_path").c_str());
    ui->framePathEdit->setText(_config.get<std::string>("frame_path").c_str());
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::accept()
{
    _modified = 0;
    if (_config.get<std::string>("video_path") != ui->videoPathEdit->text().toStdString())
    {
        _config.put("video_path", ui->videoPathEdit->text().toStdString());
        ++_modified;
    }
    if (_config.get<std::string>("frame_path") != ui->framePathEdit->text().toStdString())
    {
        _config.put("frame_path", ui->videoPathEdit->text().toStdString());
        ++_modified;
    }
    if (_modified > 0)
    {
        boost::property_tree::write_json("./config.json", _config);
    }
    QDialog::accept();
}

const std::string Dialog::frame_path() const
{
    return _config.get<std::string>("frame_path");
}

const std::string Dialog::video_path() const
{
    return _config.get<std::string>("video_path");
}

int Dialog::exec()
{
    QDialog::exec();
    return _modified;
}