#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ui_logindialog.h"  // 确保包含 ui_logindialog.h
#include "myclient.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_loginButton_click();

private:
    Ui::LoginDialog *ui;
    ChatClient *client;
};

#endif // LOGINDIALOG_H