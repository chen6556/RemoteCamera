#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    const std::string frame_path() const;
    const std::string video_path() const;

signals:
    void pathChanged(QString path);

private slots:
    void accept();

private:
    Ui::Dialog *ui;

    boost::property_tree::ptree _config;
};
#endif // DIALOG_H
