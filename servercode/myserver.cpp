#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <sstream>
#include <thread>
#include <map>
#include <mutex>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

const int PORT = 12345;
const int filePORT = 12346;
const int BUFFER_SIZE = 1024;
const string MYIP="127.0.0.1";

class ChatServer {
private:
    SOCKET serverSocket;
    SOCKET fileSocket;
    vector<SOCKET> clientSockets;
    vector<SOCKET> fileClientSockets;
    map<SOCKET, string> userMap;
    mutex mtx;
    mutex filemtx;

public:
    ChatServer() : serverSocket(INVALID_SOCKET) {}

    bool start() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData)!= 0) {
            cerr << "WSAStartup failed." << endl;
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Error creating server socket: " << WSAGetLastError() << endl;
            WSACleanup();
            return false;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = inet_addr(MYIP.c_str());

        if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Error binding server socket: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            cerr << "Error listening on server socket: " << WSAGetLastError() << endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        cout << "Server started. Waiting for connections..." << endl;

        return true;
    }

    bool startFile()
    {
        fileSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (fileSocket == INVALID_SOCKET) {
            cerr << "Error creating file socket: " << WSAGetLastError() << endl;
            WSACleanup();
            return false;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12346);
        serverAddr.sin_addr.s_addr = inet_addr(MYIP.c_str());

        if (bind(fileSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Error binding file socket: " << WSAGetLastError() << endl;
            closesocket(fileSocket);
            WSACleanup();
            return false;
        }

        if (listen(fileSocket, SOMAXCONN) == SOCKET_ERROR) {
            cerr << "Error listening on file socket: " << WSAGetLastError() << endl;
            closesocket(fileSocket);
            WSACleanup();
            return false;
        }

        cout << "File Server started. Waiting for connections..." << endl;

        return true;
    }

    void acceptClients() {
        while (true) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                cerr << "Error accepting client connection: " << WSAGetLastError() << endl;
                continue;
            }

            clientSockets.push_back(clientSocket);
            cout << "New client connected." << endl;
            thread clientThread(&ChatServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }

    void acceptFileClients()
    {
        while (true) {
            SOCKET fileClientSocket = accept(fileSocket, nullptr, nullptr);
            if (fileClientSocket == INVALID_SOCKET) {
                cerr << "Error accepting file client connection: " << WSAGetLastError() << endl;
                continue;
            }

            fileClientSockets.push_back(fileClientSocket);
            cout << "New file client connected." << endl;
            thread fileClientThread(&ChatServer::handleFileClient, this, fileClientSocket);
            fileClientThread.detach();
        }
    }
    bool sendFile(SOCKET s, const char* fileName) {
        FILE* read = fopen(fileName, "rb");
        if (!read) {
            perror("file read failed:\n");
            return false;
        }

        char buffer[BUFFER_SIZE];
        int nCount;
        int totalBytesSent = 0;

        while ((nCount = fread(buffer, 1, BUFFER_SIZE, read)) > 0) {
            if (send(s, buffer, nCount, 0) == SOCKET_ERROR) {
                perror("Send error");
                fclose(read);
                return false;
            }
            totalBytesSent += nCount;
        }

        fclose(read);
        cout << "File sent successfully! Bytes: " << totalBytesSent << endl;
        return true;
    }

    bool recvFile(SOCKET s, const char* fileName,long fileSize) {
        char buffer[BUFFER_SIZE];
        FILE* write = fopen(fileName, "wb");
        if (!write) {
            cout<<"file write failed"<<endl;
            return false;
        }
        int totalBytesReceived = 0;
        int nCount;
        cout<<"prepare to receive file"<<endl;
        while (totalBytesReceived < fileSize) {
                nCount = recv(s, buffer, BUFFER_SIZE, 0);
                if (nCount <= 0) {
                    break;
                }
                if (fwrite(buffer, 1, nCount, write) != nCount) {
                    cout << "File write error" << endl;
                    fclose(write);
                    return false;
                }
                totalBytesReceived += nCount;
            }

        cout<<"OK"<<endl;
        if (nCount == SOCKET_ERROR) {
            cout<<"Receive error"<<endl;
            fclose(write);
            return false;
        }

        fclose(write);
        cout << "File received successfully! Bytes: " << totalBytesReceived << endl;
        return true;
    }

    void handleClient(SOCKET clientSocket) {
        char buffer[BUFFER_SIZE];
        int bytesRead;
        while ((bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytesRead] = '\0';
            string message(buffer);
            vector<string> parts = split(message, ':');
            if (parts.size() > 1 && parts[0] == "USER_LOGIN") {
                string username = parts[1];
                cout << "User logged in: " << username << endl;
                {
                    lock_guard<mutex> lock(mtx);//上锁，禁止其他线程访问，因为userMap是共享资源
                    userMap[clientSocket] = username;
                }
                string userList="USER_LIST:";
                {
                    lock_guard<mutex> lock(mtx);
                    for (const auto& pair : userMap) {
                        userList += pair.second + ",";
                    }
                }

                for (SOCKET sock : clientSockets) {
                    send(sock, userList.c_str(), userList.size(), 0);
                }
                continue;
            }
            //多登陆，一人发送其他人都看到,包括自己
            for (SOCKET sock : clientSockets) {
                send(sock, buffer, bytesRead, 0);
            }
        }
            
        {
            lock_guard<mutex> lock(mtx);
            string username = userMap[clientSocket];
            userMap.erase(clientSocket);
            cout << "User logged out: " << username << endl;
        }
        string userList="USER_LIST:";
        {
            lock_guard<mutex> lock(mtx);
            for (const auto& pair : userMap) {
                userList += pair.second + ",";
            }
        }
        for (SOCKET sock : clientSockets) {
            if (sock == clientSocket) {
                continue;
            }
            send(sock, userList.c_str(), userList.size(), 0);
        }
        for (auto it = clientSockets.begin(); it!= clientSockets.end(); ++it) {
            if (*it == clientSocket) {
                clientSockets.erase(it);
                break;
            }
        }

        closesocket(clientSocket);
        cout << "Client disconnected." << endl;
    }

    void handleFileClient(SOCKET fileClientSocket)
    {
        char buffer[BUFFER_SIZE];
        int bytesRead;
        while ((bytesRead = recv(fileClientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytesRead] = '\0';
            string message(buffer);
            vector<string> parts = split(message, ':');
            if (parts.size() > 1 && parts[0] == "FILE") {
                string fileName = parts[1];
                long fileSize = stol(parts[2]);
                if (recvFile(fileClientSocket, fileName.c_str(),fileSize)) {
                    cout << "File received successfully: " << fileName << endl;

                    // 分发文件给其他客户端
                    for (SOCKET sock : clientSockets) {
                        string tempmessage="Some one send a file: "+fileName;
                        send(sock, tempmessage.c_str(), tempmessage.size(), 0);
                    }
                    for (SOCKET sock : fileClientSockets) {
                        string header = "FILE:" + fileName + ":"+ to_string(fileSize);
                        send(sock, header.c_str(), header.size(), 0);
                        sendFile(sock, fileName.c_str());
                    }
                }
                continue;
            }
        }
        for (auto it = fileClientSockets.begin(); it!= fileClientSockets.end(); ++it) {
            if (*it == fileClientSocket) {
                fileClientSockets.erase(it);
                break;
            }
        }

        closesocket(fileClientSocket);
        cout << "FileClient disconnected." << endl;
    }
    vector<string> split(const string& s, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(s);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    ~ChatServer() {
        for (SOCKET sock : clientSockets) {
            closesocket(sock);
        }
        closesocket(serverSocket);
        WSACleanup();
    }
};

int main() {
    ChatServer server;
    if (!server.start() || !server.startFile()) {
        return -1;
    }

    thread client1(&ChatServer::acceptClients, &server);
    thread client2(&ChatServer::acceptFileClients, &server);
    client1.join();
    client2.join();
    return 0;
}