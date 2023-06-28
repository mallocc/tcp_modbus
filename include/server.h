#pragma once

#include <Log.h>
#include <arpa/inet.h>
#include <errno.h>
#include <functional>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

struct Server
{
    int socketId = -1;

    std::string serverIp;
    int serverPort;
    sockaddr_in serverAddress;

    std::map<int, sockaddr_in> clientConnections;

    bool isListening;

    Server(std::string serverIp, int serverPort) : serverIp(serverIp), serverPort(serverPort), isListening(false)
    {
        LOG1("Entry");
    }

    bool init()
    {
        LOG1("Entry");

        // Set up server address
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort);
        serverAddress.sin_addr.s_addr = inet_addr(serverIp.c_str());

        LOG("Server init: " << serverIp << ":" << serverPort);

        bool success = openSocket();
        success &= bindSocket();
        return success;
    }

    bool openSocket()
    {
        LOG1("Entry");

        socketId = socket(AF_INET, SOCK_STREAM, 0);

        if (socketId == -1)
        {
            ERR("Failed to create socket.");
            ERR("Errno: " << strerror(errno)); 
            return false;
        }

        return true;
    }

    bool bindSocket()
    {
        LOG1("Entry");

        // Bind the socket to the server address
        int bindResult = bind(socketId, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));
        if (bindResult == -1)
        {
            ERR("Failed to bind socket to the server address.");
            ERR("Errno: " << strerror(errno)); 
            close(socketId);
            return false;
        }

        LOG("Server bound to: " << serverAddress.sin_addr.s_addr << ":" << serverAddress.sin_port);

        return true;
    }

    bool start(int bufferSize, std::function<void(int, sockaddr_in)> handleConnect,
               std::function<void(int, sockaddr_in, unsigned char *)> handleRecieve)
    {
        LOG1("Entry");

        bool success = false;

        // Listen for incoming connections
        success = listen(socketId, SOMAXCONN) != -1;
        if (!success)
            ERR("Failed to listen for incoming connections.");

        LOG("Server listening...");

        isListening = true;
        while (isListening && success)
        {
            // Accept a connection from a client
            sockaddr_in clientAddress{};
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(socketId, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
            if (clientSocket == -1)
            {
                ERR("Failed to accept the incoming connection.");
                ERR("Errno: " << strerror(errno)); 
                close(clientSocket);
                success = false;
            }
            else
            {
                LOG("Connected to: " << clientAddress.sin_addr.s_addr << ":" << clientAddress.sin_port
                                     << " on socket id: " << clientSocket);
                clientConnections[clientSocket] = clientAddress;
                handleConnect(clientSocket, clientAddress);
                startRecieving(clientSocket, clientAddress, handleRecieve, bufferSize);
            }
        }

        LOG("Server terminated.");

        // Close the socket
        close(socketId);

        return true;
    }

    void startRecieving(int clientSocket, sockaddr_in clientAddress,
                        std::function<void(int, sockaddr_in, unsigned char *)> handleRecieve, int bufferSize)
    {
        LOG1("Entry");

        std::thread t([=]() {
            LOG("Listening on clientSocket = " << clientSocket);

            while (isListening)
            {
                // TCP response message buffer
                unsigned char buffer[bufferSize];
                                
                LOG("Waiting to recieve data on client connection " << clientSocket << ".");
                // Receive the response message from the client
                ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), MSG_WAITALL);
                if (bytesRead == -1)
                {
                    ERR("Errno: " << strerror(errno));
                    break;
                }
                else
                {
                    LOG("Recieved from: " << clientAddress.sin_addr.s_addr << ":" << clientAddress.sin_port
                                           << " on socket id: " << clientSocket);

                    LOG("bytesRead = " << bytesRead);

                    if (bytesRead > 0)
                    {
                        handleRecieve(clientSocket, clientAddress, buffer);
                    }
                    else
                    {
                        // Connection closed by the remote side
                        LOG("Connection closed by the remote side.");
                        break;
                    }
                }
            }
            
            LOG("Terminating client connection " << clientSocket << ".");

            close(clientSocket);
        });

        // Join or detach the thread to properly manage its lifecycle
        t.detach();
    }

    bool sendData(int clientSocket, unsigned char *buffer, int size)
    {
        LOG1("Entry");

        if (clientConnections.find(clientSocket) == clientConnections.end())
            return false;

        auto client = clientConnections.at(clientSocket);

        // Send the message to the client
        ssize_t bytesSent = send(clientSocket, buffer, size, 0);
        if (bytesSent == -1)
        {
            ERR("Failed to send data to: " << client.sin_addr.s_addr << ":" << client.sin_port);
            ERR("Errno: " << strerror(errno)); 
            return false;
        }

        return true;
    }

    void stop()
    {
        LOG1("Entry");

        isListening = false;
    }
};