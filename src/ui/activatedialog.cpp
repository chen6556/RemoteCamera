#include "ui/activatedialog.h"
#include "./ui_activatedialog.h"


ActivateDialog::ActivateDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActivateDialog)
{
    ui->setupUi(this);
    ui->requestCode->setText(_d.request_code().c_str());
}

ActivateDialog::~ActivateDialog()
{
    delete ui;
}

const bool& ActivateDialog::alive() const
{
    return _alive;
}

void ActivateDialog::accept()
{
    _alive = _d.verify(ui->lineEdit->text().toStdString());
    if (_alive)
    {
        QDialog::accept();
    }
    else
    {
        ui->codeError->setText("激活码错误！");
    }
}

const std::string ActivateDialog::passwd() const
{
    char temp[36] = {'\0'};
    std::string str = ui->lineEdit->text().toStdString();
    if (str.length() > 33)
    {
        for (size_t i = 0, end = str.length(); i < end; ++i)
        {
            temp[i] = str[i];
        }
    }
    else
    {
        for (size_t i = 0, j = 0, end = str.length(); i < end; ++i, ++j)
        {
            temp[j] = str[i];
            if (i > 0 && i % 8 == 0)
            {
                temp[++j] = '-';
            }
        }
    }
    return std::string(temp);
}