#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <winsock2.h>
#include <string>
#include <vector>
#include <QString>
class ChatClient {
private:
    SOCKET clientSocket;
    SOCKET fileSocket;
    std::string username;
    QString fileReceiveDir;

public:
    ChatClient();
    ~ChatClient();
    bool connectToServer(const std::string& serverAddress, int port);
    bool fileConnectToServer(const std::string& serverAddress, int port);
    void sendMessage(const std::string& message);
    bool sendFile(const char *fileName);
    bool recvFile(const char *fileName,long fileSize);
    void setUsername(const std::string& username);
    std::string receiveMessage();
    std::string receiveFileMessage();
    void closeConnection();
    std::string getUsername() const { return username; }  // 添加 getUsername 方法
};

#endif // MYCLIENT_H