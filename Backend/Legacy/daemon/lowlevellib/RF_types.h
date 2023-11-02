#pragma once
/**
 * @brief Enum representing the different types of RF devices.
 */
enum RF_Dev_types {
    Main_dev = 0x01,   ///< Main device type
    AQ_TH_PR,          ///< Air Quality, Temperature, Humidity, Pressure sensors
    NO_TYPE = 0xFF     ///< No device type
};
/**
 * @brief Struct containing threshold values for air quality, temperature, humidity, and pressure sensors.
 */
struct AQ_TH_PR_thld {
    uint16_t pressure_high{ 1200 };  ///< Pressure threshold high (up to 1200 hPa)
    uint16_t c02_high{ 2000 };       ///< CO2 threshold high (400 to 5000 ppm)
    uint8_t Tvoc_high{ 50 };         ///< TVOC threshold high (0.1 to 10+ mg/m^3)
    uint8_t etoh_high{ 70 };         ///< ETOH threshold high (up to 20 ppm)
    uint8_t iaq_high{ 40 };          ///< IAQ threshold high (1 to 5)
    int8_t temp_high{ 60 };          ///< Temperature threshold high (up to +127°C)
    int8_t temp_low{ -40 };          ///< Temperature threshold low (as low as -128°C)
    uint8_t humidity_high{ 80 };     ///< Humidity threshold high (up to 100%)
    uint8_t humidity_low{ 10 };      ///< Humidity threshold low (as low as 0%)
};

#define AQS_THRSHLD_SIZE 11
/**
 * @brief Struct containing the values for air quality, temperature, humidity, and pressure sensors.
 */
struct AQ_TH_PR_vals {
    uint8_t humidity{};       ///< Humidity value (up to 100%)
    uint8_t etoh{};           ///< ETOH value (up to 20 ppm)
    uint8_t Tvoc{};           ///< TVOC value (0.1 to 10+ mg/m^3)
    uint8_t iaq{};            ///< IAQ value (1 to 5+)
    uint16_t c02{};           ///< CO2 value (400 to 5000 ppm)
    int16_t temp{};           ///< Temperature value (up to +127°C)
    uint16_t pressure{};      ///< Pressure value (up to 1200 hPa)
};
#define AQS_DATA_SIZE 10
/**
 * @brief Struct containing response times for different sensor communications.
 */
struct Response_Time {
    uint8_t TT_if_ack{};            ///< Response time if ACK received (in 15s increments)
    uint8_t TT_if_nack{};           ///< Response time if NACK received (in 100ms increments)
    uint16_t TP_internal_sesn_poll{};///< Internal sensor polling time (in 10 ms increments)
};
/**
 * @brief Enum representing the child device status.
 */
enum Child_status {
    pending,  ///< Device status pending
    pair      ///< Device status paired
};
/**
 * @brief Enum representing different sensor types.
 */
enum Sns_type {
    SNS_temperature = 1, ///< Temperature sensor type
    SNS_humidity,        ///< Humidity sensor type
    SNS_co2,             ///< CO2 sensor type
    SNS_etoh,            ///< ETOH sensor type
    SNS_Tvoc,            ///< TVOC sensor type
    SNS_iaq,             ///< IAQ sensor type
    SNS_pressure,        ///< Pressure sensor type
    SNS_NO_TYPE          ///< No sensor type
};