#pragma once

#include <QString>

// Packet types
#define NONE_Packet     0x00
#define UART_Packet     0x01
#define NUS_Packet      0x02

namespace STHERM {
/**
 * @brief Enumeration of serial input-output commands.
 */
enum SIOCommand {
    SetConfig = 0x01,
    SetColorRGB,
    InitMcus = 0x21,
    SetChildStatus,
    StartPairing,
    StopPairing,
    GetSensorData,
    GetNewChild,
    Get_packets,
    Send_packet,
    SetRelay = 0x31,
    GetRelaySensor,
    Check_Wiring,
    feed_wtd,
    Get_addr,
    set_wtd,
    set_update,
    GetConfig = 0x41,
    GetStatus,
    GetInfo,
    GetTemperature,
    GetHumidity,
    GetAQsData,
    GetBarometer,
    GetAmbientLight,
    GetTOF,
    GetSensors,
    GetIntoDFU,
    GET_DEV_ID,
    ShutDown,
    CommandNotSupported=0xfe,
    NoCommand
};

/**
 * @brief Enumeration of serial input-output errors.
 */
enum SIOErrors {
    ERROR_NO = 0x00,
    ERROR_01,         // Error: I2C_BUS
    ERROR_02,          // Error: Temperature/Humidity is not updated
    ERROR_GPIO_INIT,
    ERROR_UNKNOWN_COMMAND,
    ERROR_CRC,
    ERROR_TX,
    ERROR_DATA_LEN,
    ERROR_RELAY_NOT_FOUND,
    USAGE_ERROR,//programm call argument count error
    ARGUMENT_ERROR,//programm call argument expression error
    RESOURCE_BUSY_ERROR,//can not open recource associated with the programm
    INTERNAL_ERROR,//error on the side of microcontrollers
    ERROR_WIRING_NOT_CONNECTED,
    ERROR_COULD_NOT_SET_RELAY

} ;

enum PacketType {
    NONEPacket = 0,
    UARTPacket,
    NUSPacket
};

/**
 * @brief Structure for serial input-output packets.
 */
struct SIOPacket {
    uint8_t PacketSrc;
    SIOCommand CMD;
    uint8_t ACK;
    uint8_t SID;
    uint8_t DataLen;
    uint8_t DataArray[250];
    uint16_t CRC;
};

/**
 * @brief Structure for storing received serial data.
 */
struct SerialRxData {
    uint8_t RxDataLen;
    bool RxActive;
    bool RxPacketDone;
    bool RxCtrlEsc;
    uint8_t RxDataArray[256];
};
}

/*! ***********************************************************************************************
 * The UtilityHelper class is a container that encapsulates
 *  a collection of static methods providing a range of general-purpose functionalities.
 *  It acts as a helper, offering a diverse set of operations that are commonly
 *  needed across various components of application.
 * ************************************************************************************************/

class UtilityHelper
{
public:

    //! This function exports the specified SW GPIO, configures it as input, and sets the edge detection to
    //! falling.
    //! gpio The GPIO pin number to be configured.
    //! return Returns true if successful
    //!  or false if an error occurred.
    static bool configurePins(int gpio);

    //! Open direction file and set pin
    static void exportGPIOPin(int pinNumber);

    //! Get start mode
    //! Read data from gpio%0/value in system
    //! return 1 if value is "0"
    //! or return 0 if has different  value
    static int getStartMode(int pinNumber);

    //! Get CPU info
    //! return a string
    //! todo: Adjust the format to meet the UI requirements
    static QString getCPUInfo();

    //! setBrightness, value is a number between 0 and 254
    static void setBrightness(int value);

    //! Set time zone
    static void setTimeZone(int offset);


    //! Prepares a serial packet for transmission.
    //! This function takes a packet of data and processes it for transmission,
    //! including calculating the CRC and adding escape sequences for special
    //! characters.
    //! @param[out] TxDataBuf The buffer containing the processed packet to be
    //! transmitted.
    //! @param TxPacket The SIO_Packet_t struct containing the original packet data.
    //! @return The length of the processed packet in bytes.
    static uint16_t setSIOTxPacket(uint8_t* TxDataBuf, STHERM::SIOPacket TxPacket);

    //! brief Processes a received byte of serial data.
    //! This function handles the received serial data, managing the escape
    //! sequences and constructing the received packet.
    //!
    //! @param RxData The received data byte.
    //! @param[out] RxDataCfg The Serial_RxData_t configuration struct containing
    //!                       the current state of the received packet.
    //! @return True if the packet is successfully received and has a valid length,
    //!         false otherwise.
    bool SerialDataRx(uint8_t RxData, STHERM::SerialRxData* RxDataCfg);

    //! Calculates the CRC-16 checksum for the given data.
    //! This function calculates the CRC-16 checksum for the given data buffer
    //! and size using the specified polynomial.
    //! @param data_p A pointer to the data buffer to be checksummed.
    //! @param length The size of the data buffer in bytes.
    //! @return The CRC-16 checksum for the given data.
    static unsigned short crc16(unsigned char* data_p, unsigned short length);

    //! Return Packet Type with packetType enum
    static uint8_t packetType(STHERM::PacketType packetType);
};

