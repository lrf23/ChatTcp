#include "myqt1.h"
#include "logindialog.h"
#include <QApplication>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {

        return a.exec();
    }
    return 0;
}
