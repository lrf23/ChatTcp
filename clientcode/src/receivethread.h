#ifndef RECEIVERTHREAD_H
#define RECEIVERTHREAD_H

#include <QThread>
#include "myclient.h"
#include <QMutex>
class ReceiverThread : public QThread
{
    Q_OBJECT

public:
    explicit ReceiverThread(ChatClient* client, QObject* parent = nullptr);
    void run() override;

signals:
    void messageReceived(const QString& message);
    void updateUserList(const QString& userList);

private:
    ChatClient* client;
    QMutex mutex;
};

#endif // RECEIVERTHREAD_H