#include "filereceivethread.h"
#include <QString>


FileReceiverThread::FileReceiverThread(ChatClient* client, QObject* parent)
    : QThread(parent), client(client)
{
}

void FileReceiverThread::run()
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
            }
        }

    }
}