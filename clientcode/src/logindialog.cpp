#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include "myclient.h"
#include "myqt1.h"
#include <QRegExp>
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
    client(new ChatClient)
{
    ui->setupUi(this);

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::on_loginButton_click);
}

LoginDialog::~LoginDialog()
{
    delete ui;
    delete client;
}

void LoginDialog::on_loginButton_click()
{
    QString username = ui->usernameLineEdit->text();
    QString ip = ui->ipLineEdit->text();
    QString port=ui->portLineEdit->text();
    QRegExp regex("^[a-zA-Z0-9\u4e00-\u9fa5]+$");
    if (!regex.exactMatch(username)) {
        QMessageBox::warning(this, "Login Failed", "Invalid username");
        return;
    }
    if (port=="12345") {
        if (client->connectToServer(ip.toStdString(), port.toInt()) && client->fileConnectToServer(ip.toStdString(), 12346)) {
            QMessageBox::information(this, "Login Success", "Welcome, " + username);
            client->setUsername(username.toStdString());
            myqt1 *mainWindow = new myqt1(client);
            mainWindow->show();
        } else {
            QMessageBox::warning(this, "Login Failed", "Connection to server failed");
            return;
        }
        accept();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid port");
    }
}