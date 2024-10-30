#ifndef FILERECEIVERTHREAD_H
#define FILERECEIVERTHREAD_H

#include <QThread>
#include "myclient.h"
#include <QMutex>
#include <QWaitCondition>
class FileReceiverThread : public QThread
{
    Q_OBJECT

public:
    explicit FileReceiverThread(ChatClient* client, QObject* parent = nullptr);
    void run() override;

signals:
    void fileReceived(const QString& fileName, long fileSize);

private:
    ChatClient* client;

};

#endif // FILERECEIVERTHREAD_H