#include "receivethread.h"
#include <QString>

ReceiverThread::ReceiverThread(ChatClient* client, QObject* parent)
    : QThread(parent), client(client)
{
}

void ReceiverThread::run()
{
    while (true) {
        std::string message = client->receiveMessage();
        if (!message.empty()) {
            QString qMessage = QString::fromStdString(message);

            // 假设服务器发送的用户登录信息格式为 "USER_LOGIN:username"
            if (qMessage.startsWith("USER_LIST"))
            {
                emit updateUserList(qMessage.mid(10));
            }
            else {
                emit messageReceived(qMessage);
            }
        }
    }
}