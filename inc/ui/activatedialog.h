#ifndef ACTIVATEDIALOG_H
#define ACTIVATEDIALOG_H

#include <QDialog>
#include "Dongle/Dongle.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class ActivateDialog; }
QT_END_NAMESPACE

class ActivateDialog : public QDialog
{
    Q_OBJECT

public:
    ActivateDialog(QWidget *parent = nullptr);
    ~ActivateDialog();
    const bool& alive() const;
    const std::string passwd() const;

private:
    Ui::ActivateDialog *ui;
    bool _alive = false;
    Dongle _d;

private slots:
    void accept();

};
#endif // ACTIVATEDIALOG_H
