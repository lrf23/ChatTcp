#pragma once
#include "ui_myqt1.h"
#include <QMainWindow>
#include "myclient.h"
#include "receivethread.h"
#include "filereceivethread.h"
#include <QKeyEvent>
#include <QFileSystemModel>
#include <thread>
class myqt1 : public QMainWindow {
    Q_OBJECT
    
public:
    myqt1(ChatClient *client,QWidget* parent = nullptr);
    ~myqt1();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // 声明 eventFilter 方法

private slots:
    void on_sendPushButton_click();
    void handleMessageReceived(const QString& message);
    void handleUserList(const QString& userList);
    void handleFileReceived(const QString& fileName,long fileSize);  // 声明处理接收文件的槽函数
    void on_chooseFileButton_click();  // 声明 chooseFileButton 的槽函数

signals:
    void fileReceived(const QString& fileName, long fileSize);  // 声明 fileReceived 信号
    
private:
    Ui_myqt1* ui;
    ChatClient *client;
    ReceiverThread* receiverThread;
    //FileReceiverThread* fileReceiverThread;
    std::thread fileReceiverThread;
    QFileSystemModel *fileModel;
    QString fileReceiveDir;
    void fileReceiverFunction();
};

