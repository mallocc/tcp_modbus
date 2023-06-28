#pragma once

#include <Log.h>
#include <arpa/inet.h>
#include <errno.h>
#include <functional>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

struct Client
{
    int socketId = -1;

    std::string serverIp;
    int serverPort;
    sockaddr_in serverAddress;

    bool isListening;

    Client(std::string serverIp, int serverPort) : serverIp(serverIp), serverPort(serverPort), isListening(false)
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

        LOG("Client init: " << serverIp << ":" << serverPort);

        bool success = openSocket();
        success &= connectSocket();
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

    bool connectSocket()
    {
        LOG1("Entry");

        LOG("TCP client is waiting for a connection...");

        // Bind the socket to the server address
        int result = connect(socketId, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));
        if (result == -1)
        {
            ERR("Failed to connect socket to the server address.");
            ERR("Errno: " << strerror(errno));
            close(socketId);
            return false;
        }

        LOG("Client connected to: " << serverAddress.sin_addr.s_addr << ":" << serverAddress.sin_port);

        return true;
    }

    bool start(int bufferSize, std::function<void()> handleConnect,
               std::function<void(unsigned char *)> handleRecieve)
    {
        LOG1("Entry");

        handleConnect();

        unsigned char previousData[bufferSize + 1] = ""; // Variable to store the previously received data

        isListening = true;

        while (isListening)
        {
            // TCP message data buffer
            unsigned char buffer[bufferSize + 1];

            LOG("Waiting to recieve data...");
            // Receive message from server
            ssize_t bytesRead = recv(socketId, buffer, sizeof(buffer) - 1, MSG_WAITALL);
            if (bytesRead == -1)
            {
                ERR("Failed to receive data.");
                ERR("Errno: " << strerror(errno));
                isListening = false;
            }
            else
            {
                LOG1("bytesRead = " << bytesRead);

                if (bytesRead > 0)
                {
                    buffer[bytesRead] = '\0'; // Null-terminate the received data

                    // Compare with previous data to check for changes
                    if (strcmp((const char *)buffer, (char *)previousData) != 0)
                    {
                        // New data has arrived, process and display it
                        LOG("Received data from server.");

                        // Update the previous data
                        strcpy((char *)previousData, (const char *)buffer);

                        handleRecieve(buffer);
                    }
                }
                else
                {
                    // Connection closed by the remote side
                    LOG("Connection closed by the remote side.");
                    isListening = false;
                }
            }
        }

        // Close the server socket
        close(socketId);

        LOG("Client terminated.");

        return true;
    }

    bool sendData(unsigned char *buffer, int size)
    {
        LOG1("Entry");

        // Send the message to the server
        ssize_t bytesSent = send(socketId, buffer, size, 0);
        if (bytesSent == -1)
        {
            ERR("Failed to send data to: " << serverAddress.sin_addr.s_addr << ":" << serverAddress.sin_port);
            ERR("Errno: " << strerror(errno));
            return false;
        }
        else
        {            
            LOG("Data successfully sent to server.");
        }

        return true;
    }

    void stop()
    {
        LOG1("Entry");

        isListening = false;
    }
};