#ifndef PATHDIALOG_H
#define PATHDIALOG_H

#include <QDialog>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class PathDialog; }
QT_END_NAMESPACE

class PathDialog : public QDialog
{
    Q_OBJECT

public:
    PathDialog(QWidget *parent = nullptr);
    ~PathDialog();
    const std::string frame_path() const;
    const std::string video_path() const;

signals:
    void pathChanged(const char flag, QString path);

private slots:
    void accept();

private:
    Ui::PathDialog *ui;

    boost::property_tree::ptree _config;
};
#endif // DIALOG_H
