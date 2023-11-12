#pragma once

#include <QString>

// Packet types
#define NONE_Packet     0x00
#define UART_Packet     0x01
#define NUS_Packet      0x02

namespace STHERM {

#define AQS_DATA_SIZE 10

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
 * @brief Enum representing the different types of RF devices.
 */
enum RFDevTypes {
    Main_dev = 0x01,   ///< Main device type
    AQ_TH_PR,          ///< Air Quality, Temperature, Humidity, Pressure sensors
    NO_TYPE = 0xFF     ///< No device type
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

    //! Use QByteArray
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

/**
 * @brief Structure Vacation data.
 */
struct Vacation {
    double minimumTemperature;
    double maximumTemperature;
    double minimumHumidity;
    double maximumHumidity;
    bool   isEnable;
};

/**
 * @brief Enumeration for system modes.
 */
enum RelayMode
{
    NoWire = 0,
    ON,
    OFF
};
struct Relay
{
    Relay() {
        g     = RelayMode::NoWire;
        y1    = RelayMode::NoWire;
        y2    = RelayMode::NoWire;
        y3    = RelayMode::NoWire;
        acc2  = RelayMode::NoWire;
        w1    = RelayMode::NoWire;
        w2    = RelayMode::NoWire;
        w3    = RelayMode::NoWire;
        o_b   = RelayMode::NoWire;
        acc1p = RelayMode::NoWire;
        acc1n = RelayMode::NoWire;
    }

    RelayMode g;
    RelayMode y1;
    RelayMode y2;
    RelayMode y3;
    RelayMode acc2;
    RelayMode w1;
    RelayMode w2;
    RelayMode w3;
    RelayMode o_b;
    RelayMode acc1p;
    RelayMode acc1n;
    RelayMode hum_wiring;

};

/**
 * @brief Struct containing the values for air quality, temperature, humidity, and pressure sensors.
 */
struct AQ_TH_PR_vals {
    uint8_t humidity{};       ///< Humidity value (up to 100%)
    uint8_t etoh{};           ///< ETOH value (up to 20 ppm)
    uint8_t Tvoc{};           ///< TVOC value (0.1 to 10+ mg/m^3)
    uint8_t iaq{};            ///< IAQ value (1 to 5+)
    uint16_t c02{};           ///< CO2 value (400 to 5000 ppm)
    int16_t temp{};           ///< Temperature value (up to +127�C)
    uint16_t pressure{};      ///< Pressure value (up to 1200 hPa)
};

/**
  * @brief Identifies RF Device types.
  */
struct DeviceType
{
    DeviceType() {
        address = {};
        type = RFDevTypes::NO_TYPE;
        paired = false;
    }

    uint32_t address;
    uint8_t type;
    bool paired;
};

/**
 * @brief Enumeration for alert types.
 */
enum AlertTypes
{
    Alert_temp_high = 1,// +127 max
    Alert_temp_low, // -128 low
    Alert_Tvoc_high, // 255 max (tvoc value range 0.1 to 10+ mg/m^3 value is divided by 10.0)
    Alert_etoh_high, //up to 20ppm
    Alert_iaq_high, //1 to 5
    Alert_humidity_high,// up to 100%
    Alert_humidity_low,//as low as 0%
    Alert_pressure_high, //up to 1200 hPa
    Alert_c02_high,//400 to 5000ppm
    Alert_wiring_not_connected,
    Alert_could_not_set_relay,
    NO_ALlert
};

/**
 * @brief Enumeration for alert levels.
 */
enum AlertLevel
{
    LVL_Emergency = 1,
    LVL_Warning,
    LVL_UNIMPORTANT
};

/**
 * @brief Enumeration for system modes.
 */
enum SystemMode
{
    Cooling = 0,
    Heating,
    Auto,
    Vacation,
    Off,
    Emergency
};

/**
 * @brief Enumeration for LED behavior.
 */
enum LedEffect
{
    LED_STABLE = 0,
    LED_FADE,
    LED_BLINK,
    LED_NO_MODE
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
    static bool setBrightness(int value);

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
    static bool SerialDataRx(uint8_t RxData, STHERM::SerialRxData* RxDataCfg);

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

