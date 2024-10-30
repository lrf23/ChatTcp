#include "myclient.h"
#include <iostream>
#include <sstream>
#include <QmessageBox>
#include <QFileInfo>
#include <QDir>
#pragma comment(lib, "ws2_32.lib")

const int BUFFER_SIZE = 1024;

ChatClient::ChatClient() : clientSocket(INVALID_SOCKET), fileSocket(INVALID_SOCKET),fileReceiveDir(QDir::currentPath() + "/ReceivedFiles") {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

ChatClient::~ChatClient() {
    closesocket(clientSocket);
    WSACleanup();
}

bool ChatClient::connectToServer(const std::string& serverAddress, int port) {
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(serverAddress.c_str());
    serverInfo.sin_port = htons(port);

    if (connect(clientSocket, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed" << std::endl;
        closesocket(clientSocket);
        return false;
    }

    return true;
}

bool ChatClient::fileConnectToServer(const std::string& serverAddress, int port) {
    fileSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fileSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(serverAddress.c_str());
    serverInfo.sin_port = htons(port);

    if (connect(fileSocket, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed" << std::endl;
        closesocket(fileSocket);
        return false;
    }

    return true;
}
void ChatClient::sendMessage(const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}

void ChatClient::setUsername(const std::string& username) {
    this->username = username;
}


std::string ChatClient::receiveMessage() {
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        return std::string(buffer, bytesReceived);
    }
    return "";
}

std::string ChatClient::receiveFileMessage() {
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(fileSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        return std::string(buffer, bytesReceived);
    }
    return "";
}
void ChatClient::closeConnection() {
    closesocket(clientSocket);
    closesocket(fileSocket);
    clientSocket = INVALID_SOCKET;
    fileSocket = INVALID_SOCKET;
}

bool ChatClient::sendFile(const char* filePath) {
    QFileInfo fileInfo(QString::fromLocal8Bit(filePath));  // 使用 fromLocal8Bit 处理文件路径
    QString fileName = fileInfo.fileName();  // 提取文件名，不包含路径前缀
    FILE* file = fopen(fileInfo.absoluteFilePath().toLocal8Bit().constData(), "rb");  // 使用 toLocal8Bit 处理文件路径
    if (!file) {
        QMessageBox::warning(nullptr, "File send failed", fileName);
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    
    std::ostringstream oss;
    oss << "FILE:" << fileName.toLocal8Bit().constData() << ":" << fileSize;  // 使用 toLocal8Bit 处理文件名
    std::string message = oss.str();
    send(fileSocket, message.c_str(), message.size(), 0);

    char buffer[BUFFER_SIZE];
    while (!feof(file)) {
        size_t bytesRead = fread(buffer, 1, BUFFER_SIZE, file);
        if (bytesRead > 0) {
            size_t bytesSent = 0;
            while (bytesSent < bytesRead) {
                int result = send(fileSocket, buffer + bytesSent, bytesRead - bytesSent, 0);
                if (result == SOCKET_ERROR) {
                    //qDebug() << "Error sending file data:" << WSAGetLastError();
                    fclose(file);
                    return false;
                }
                bytesSent += result;
            }
        }
    }

    fclose(file);
    return true;
}

bool ChatClient::recvFile(const char* fileName,long fileSize) {
    QString fullPath =fileReceiveDir+ "/" + QString::fromLocal8Bit(fileName);
    FILE* file = fopen(fullPath.toLocal8Bit().constData(), "wb");
    if (!file) {
        std::cerr << "File write failed" << std::endl;
        return false;
    }

    char buffer[BUFFER_SIZE];
    int totalBytesReceived = 0;
    int bytesRead;
    while (totalBytesReceived < fileSize) {
        bytesRead = recv(fileSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            break;
        }
        if (totalBytesReceived + bytesRead > fileSize) {
            bytesRead = fileSize - totalBytesReceived;  // 调整 bytesRead 以确保不超过 fileSize
        }
        if (fwrite(buffer, 1, bytesRead, file) != bytesRead) {
            perror("File write error");
            fclose(file);
            return false;
        }
        totalBytesReceived += bytesRead;
    }

    if (bytesRead < 0) {
        perror("Receive error");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}