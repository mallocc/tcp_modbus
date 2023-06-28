#include <Log.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <server.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#define SERVER_IP "127.0.0.1" // IP address of the Modbus TCP slave
#define SERVER_PORT 502       // Port number for Modbus TCP communication

uint16_t calculateCRC(unsigned char *message, int length)
{
    uint16_t crc = 0xFFFF;

    for (int i = 0; i < length; ++i)
    {
        crc ^= message[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001; // XOR with predefined value
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

int main()
{
    Server server(SERVER_IP, SERVER_PORT);

    if (server.init())
    {
        server.start(
            8,
            [&](int clientSocket, sockaddr_in clientAddress) {
                // Modbus TCP request message
                // Replace this with your actual Modbus request message
                // For example, read holding registers (function code 0x03)
                unsigned char modbusRequest[] = {0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00};

                auto crc = calculateCRC(modbusRequest, 6);
                LOG(" [MASTER] "
                    << "0x" << std::hex << crc);

                modbusRequest[6] = crc >> 8;
                modbusRequest[7] = crc & 0x00ff;

                server.sendData(clientSocket, modbusRequest, sizeof(modbusRequest));
            },
            [&](int clientSocket, sockaddr_in clientAddress, unsigned char *bufferData) {
                // Process the received Modbus response
                // Parse the response data according to the Modbus protocol
                // specifications

                std::stringstream ss;
                ss << "Modbus Response: ";
                for (int i = 0; i < 8; ++i)
                {
                    ss << "0x" << std::hex << static_cast<int>(bufferData[i]) << " ";
                }
                LOG(" [MASTER] " << ss.str());
            });

        server.stop();
    }

    LOG(" [MASTER] "
        << "exiting...");

    return 0;
}