#include "dialog.h"
#include "./ui_dialog.h"
#include <boost/filesystem.hpp>

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
    std::string video_path = ui->videoPathEdit->text().toStdString();
    if (_config.get<std::string>("video_path") != video_path)
    {
        if (boost::filesystem::is_directory(video_path))
        {
            _config.put("video_path", video_path);
            ++_modified;
        }
        else if (!boost::filesystem::is_regular_file(video_path))
        {
            boost::filesystem::create_directory(video_path);
            if (boost::filesystem::is_directory(video_path))
            {
                _config.put("video_path", video_path);
                ++_modified;
                boost::filesystem::remove(video_path);
            }
        }
    }

    std::string frame_path = ui->framePathEdit->text().toStdString();
    if (_config.get<std::string>("frame_path") != frame_path)
    {
        if (boost::filesystem::is_directory(frame_path))
        {
            _config.put("frame_path", frame_path);
            ++_modified;
        }
        else if (!boost::filesystem::is_regular_file(frame_path))
        {
            boost::filesystem::create_directory(frame_path);
            if (boost::filesystem::is_directory(frame_path))
            {
                _config.put("frame_path", frame_path);
                ++_modified;
                boost::filesystem::remove(frame_path);
            }
        }
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