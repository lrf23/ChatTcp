#include "myqt1.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

myqt1::myqt1(ChatClient *client,QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_myqt1),
    client(client),
    fileReceiveDir(QDir::currentPath() + "/ReceivedFiles")  // 初始化文件接收目录
{
    ui->setupUi(this);
    connect(ui->sendPushButton, &QPushButton::clicked, this, &myqt1::on_sendPushButton_click);
    connect(ui->chooseFileButton, &QPushButton::clicked, this, &myqt1::on_chooseFileButton_click);

    receiverThread = new ReceiverThread(client, this);
    connect(receiverThread, &ReceiverThread::messageReceived, this, &myqt1::handleMessageReceived);
    connect(receiverThread, &ReceiverThread::updateUserList, this, &myqt1::handleUserList);  // 连接信号
    receiverThread->start();

    connect(this, &myqt1::fileReceived, this, &myqt1::handleFileReceived);  // 连接信号和槽函数
    fileReceiverThread = std::thread(&myqt1::fileReceiverFunction, this);
    fileReceiverThread.detach();

    ui->chatTextEdit->setReadOnly(true);
    ui->userTextEdit->setReadOnly(true);
    
    ui->inputLineEdit->installEventFilter(this);
    std::string messsage="USER_LOGIN:"+client->getUsername();
    client->sendMessage(messsage);
    QDir dir(fileReceiveDir);
    if (dir.exists()) {
        QStringList files = dir.entryList(QDir::Files);
        for (const QString &file : files) {
            dir.remove(file);
        }
    }
    QDir().mkpath(fileReceiveDir);
    
    fileModel=new QFileSystemModel(this);
    fileModel->setRootPath(fileReceiveDir);
    ui->fileTreeView->setModel(fileModel);
    ui->fileTreeView->setRootIndex(fileModel->index(fileReceiveDir));
}

myqt1::~myqt1()
{
    std::string message = "USER_LOGOUT:" + client->getUsername();
    client->sendMessage(message);

    receiverThread->quit();
    receiverThread->wait();
    delete fileModel;
    delete receiverThread;
    delete ui;
}

void myqt1::on_sendPushButton_click()
{
    QString message = ui->inputLineEdit->text();
    if(!message.isEmpty())
    {
        QString username = QString::fromStdString(client->getUsername());
        QString fullMessage = username + ": " + message;  // 在消息前添加用户名
        client->sendMessage(fullMessage.toStdString());
        ui->inputLineEdit->clear();
    }
    else{
        return ;
    }

}

void myqt1::on_chooseFileButton_click()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("所有文件 (*.*)"));
    if (!fileName.isEmpty()) {
        // 处理选择的文件，例如发送文件
        //qDebug() << "选择的文件:" << fileName;
        if (client->sendFile(fileName.toLocal8Bit().constData()))
        {
            //QMessageBox::information(this, "文件发送成功", "文件发送成功");
        }
        else
        {
            QMessageBox::warning(this, "文件发送失败", "文件发送失败");
        }
    }
}

bool myqt1::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->inputLineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            on_sendPushButton_click();  // 调用发送消息的槽函数
            return true;  // 事件已处理
        }
    }
    return QMainWindow::eventFilter(obj, event);  // 调用基类的事件处理
}

void myqt1::handleMessageReceived(const QString& message)
{
    //qDebug() << "Message received:" << message;
    ui->chatTextEdit->append(message);  // 将消息显示在 TextEdit 上
}

void myqt1::handleUserList(const QString& userList)
{
    //qDebug() << "User list received:" << userList;
    ui->userTextEdit->clear();
    QStringList users = userList.split(',', QString::SkipEmptyParts);
    for (const QString& user : users) {
        ui->userTextEdit->append(user);
    }
}

void myqt1::handleFileReceived(const QString& fileName, long fileSize)
{
    // 构建文件的完整路径
    if (fileName == "Error") {
        QMessageBox::warning(this, "文件接收失败", "文件接收失败");
        return;
    }
    QString fullPath = fileReceiveDir + "/" + fileName;

    // 将接收到的文件显示在 QListView 中
    QFileInfo fileInfo(fullPath);
    QString filePath = fileInfo.absoluteFilePath();
    QMessageBox::information(this, "文件接收成功", fullPath);
    QModelIndex index = fileModel->index(filePath);
    ui->fileTreeView->setRootIndex(fileModel->index(fileReceiveDir));  // 设置根索引为文件接收目录
    ui->fileTreeView->scrollTo(index);  // 滚动到新接收的文件
}

void myqt1::fileReceiverFunction()
{
    while (true) {
        // 假设接收到的文件信息格式为 "FILE:filename:filesize"
        std::string message = client->receiveFileMessage();
        if (!message.empty()) {
            QString qMessage = QString::fromStdString(message);
            if(qMessage.startsWith("FILE"))
            {
                QString fileName = qMessage.section(':', 1, 1);
                long fileSize = qMessage.section(':', 2, 2).toLong();
                if (client->recvFile(fileName.toStdString().c_str(), fileSize))
                {
                    emit fileReceived(fileName, fileSize);
                }
                else{
                    emit fileReceived("Error", 0);
                }
            }

        }
    }
}