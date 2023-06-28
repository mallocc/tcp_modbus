#include <Log.h>
#include <arpa/inet.h>
#include <client.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <functional>

#define SERVER_IP "127.0.0.1" // IP address of the Modbus TCP slave
#define SERVER_PORT 502       // Port number for Modbus TCP communication

enum class FunctionCode : uint8_t
{
    READ_COILS = 0x01,
    READ_DISCRETE_INPUTS = 0x02,
    READ_HOLDING_REGISTERS = 0x03,
    READ_INPUT_REGISTERS = 0x04,
    WRITE_SINGLE_COIL = 0x05,
    WRITE_SINGLE_REGISTER = 0x06,
    WRITE_MULTIPLE_COILS = 0x0f,
    WRITE_MULTIPLE_REGISTERS = 0x10,
};

struct RequestHeader
{
    uint8_t id;
    FunctionCode functionCode;
};

struct RequestCoilsMessage
{
    RequestHeader header;
    uint16_t startAddress;
    uint16_t quantity;
    uint16_t crc;
};

struct ResponseCoilsMessage
{
    RequestHeader header;
    uint8_t byteCount;
    uint8_t *coilStatus;
    uint16_t crc;
};

uint16_t swapByteOrder(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

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

const int coilAddresses = 256;
uint16_t coilData[coilAddresses] = {0x0};

const int registerAddresses = 256;
uint16_t registerData[registerAddresses] = {0x0};

int main()
{
    Client client(SERVER_IP, SERVER_PORT);

    if (client.init())
    {
        client.start(
            8,
            [&]() {
                LOG(" [MASTER] Connected.");
            },
            [&](unsigned char *buffer) {
                std::stringstream ss;
                ss << "Modbus Request: ";
                for (int i = 0; i < sizeof(buffer); ++i)
                {
                    ss << "0x" << std::hex << static_cast<int>(buffer[i]) << " ";
                }
                LOG(" [SLAVE ] " << ss.str());

                // Process the received Modbus request
                // Parse the request data according to the Modbus protocol specifications
                // Prepare the appropriate Modbus response based on the request

                auto &requestHeader = *reinterpret_cast<RequestHeader *>(buffer);

                LOG(" [SLAVE ] " << static_cast<int>(requestHeader.id) << ", "
                                 << static_cast<int>(requestHeader.functionCode));

                std::stringstream ssMsg;

                ssMsg << std::endl;

                switch (requestHeader.functionCode)
                {
                case FunctionCode::READ_COILS: {
                    auto &msg = *reinterpret_cast<RequestCoilsMessage *>(buffer);

                    if (swapByteOrder(msg.crc) == calculateCRC(buffer, 6))
                    {
                        ssMsg << "id: " << static_cast<uint16_t>(msg.header.id) << std::endl;
                        ssMsg << "functionCode: " << static_cast<int>(msg.header.functionCode) << std::endl;
                        ssMsg << "startAddress: " << static_cast<uint16_t>(swapByteOrder(msg.startAddress))
                              << std::endl;
                        ssMsg << "quantity: " << static_cast<uint16_t>(swapByteOrder(msg.quantity)) << std::endl;
                        ssMsg << "crc: " << std::hex << static_cast<uint16_t>(swapByteOrder(msg.crc)) << std::endl;
                    }
                    else
                    {
                        WARN(" [SLAVE ] "
                             << "INVALID CRC - id: " << static_cast<uint16_t>(msg.header.id)
                             << ", functionCode: " << static_cast<int>(msg.header.functionCode));
                    }

                    break;
                }
                default:
                    break;
                }

                LOG(" [SLAVE ] " << ssMsg.str());

                // Modbus TCP response message
                // Replace this with your actual Modbus response message
                // For example, send the data requested by the master
                unsigned char modbusResponse[] = {
                    0x00, // Slave Address
                    0x03, // Function Code (Read Holding Registers)
                    0x04, // Byte Count (Number of bytes in the data section)
                    0x00, // Data (Holding Register Value High Byte)
                    0x01, // Data (Holding Register Value Low Byte)
                    0x00, // Data (Holding Register Value High Byte)
                    0x02, // Data (Holding Register Value Low Byte)
                };
                
                client.sendData(modbusResponse, sizeof(modbusResponse));
            });

        client.stop();
    }

    LOG(" [SLAVE] "
        << "Exiting...");

    return 0;
}