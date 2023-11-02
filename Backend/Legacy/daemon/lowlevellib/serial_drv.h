/**
 * @file serial_drv.h
 * @brief Serial driver header file.
 *
 * This file contains the definitions and structures required for the serial
 * driver.
 */
#ifndef __SERIAL_DRV_H__
#define __SERIAL_DRV_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

    // Definitions of special characters used in serial communication
#define phyStart        0xF0
#define phyStop         0xF1
#define phyCtrlEsc      0xF2
#define phyXorByte      0xFF
    // Offsets for various fields in serial packets
#define CMD_Offset      0
#define ACK_Offset      1
#define SID_Offset      2
#define DATA_Offset     3
    // Masks for packet types
#define CmdACK_Mask     0x80
    // Packet types
#define NONE_Packet     0x00
#define UART_Packet     0x01
#define NUS_Packet      0x02
    // Minimum packet length
#define PacketMinLength 5

#define CMD_ACK                 0x00
#define CMD_ERR                 0x01
#define CMD_ERR_ACT_SCH_EXIST   0x02
#define CMD_ERR_BUS_RGB         0x03
#define CMD_ERR_BUS_TMR         0x04
#define CMD_ERR_LICENSE         0xFF
    /**
 * @brief Structure for storing received serial data.
 */
typedef struct Serial_RxData {
    uint8_t RxDataLen;
    bool RxActive;
    bool RxPacketDone;
    bool RxCtrlEsc;
    uint8_t RxDataArray[256];
} Serial_RxData_t;
/**
 * @brief Enumeration of serial input-output commands.
 */
typedef enum SIO_Command {
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
} SIO_Cmd_t;
/**
 * @brief Structure for serial input-output packets.
 */
typedef struct SIO_Packet {
    uint8_t PacketSrc;
    SIO_Cmd_t CMD;
    uint8_t ACK;
    uint8_t SID;
    uint8_t DataLen;
    uint8_t DataArray[250];
    uint16_t CRC;
} SIO_Packet_t;
/**
 * @brief Enumeration of serial input-output errors.
 */
typedef enum SIO_Errors {
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

} SIO_Errors_t;
// Function prototypes
/**
 * @brief function to assemble RX packet. checks every char for ctrl and data
 * 
 * @param RxData new char to check and add to packet
 * @param RxDataCfg pointer to structure RX packet. 
 * @return true receive packet end (phyStop)
 * @return false did't receive packet end (phyStop)
 */
bool SerialDataRx(uint8_t RxData, Serial_RxData_t* RxDataCfg);

/**
 * @brief sets the data in packet structure into the TxDataBuf array. 
 * 
 * @param TxDataBuf pointer to array for packet 
 * @param TxPacket  strucrute with packet
 * @return uint16_t array size
 */
uint16_t Set_SIO_TxPacket(uint8_t* TxDataBuf, SIO_Packet_t TxPacket);

#ifdef __cplusplus
 }
#endif //cpp
#endif //__SERIAL_DRV_H__

