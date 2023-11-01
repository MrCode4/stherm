/**
 * @file php_interface.h
 * @brief Handles communication with web backend and provides helper functions to manage devices and their data.
 * @author Narek Aleksanyan
 */
#pragma once
#include "Peripheral.h"
#define WEB_DIR "/usr/share/apache2/default-site/htdocs/engine/"
#define TOF_BR_FILE "/usr/share/apache2/default-site/htdocs/brightness.json"
#define TOF_IRQ_MAX_RANGE 1000 //mm
#define TOF_IRQ_MIN_RANGE 60 //mm

 /**
  * @brief Identifies RF Device types.
  */
struct device_t
{
    uint32_t address{};
    uint8_t type{ NO_TYPE };
    bool paired{false};
};
/**
 * @brief Time configurations for external sensor modules.
 */
struct config_time {
    uint8_t ext_sens_type{ NO_TYPE };
    uint8_t TT_if_ack{};//in 15s increments
    uint8_t TT_if_nack{};//in 100ms increments
    uint16_t TP_internal_sesn_poll{};// 10 ms increments
};
#define CONF_TIME_SIZE 5
/**
 * @brief Thresholds for each sensor type.
 */
struct config_thresholds {
    uint8_t sens_type{SNS_NO_TYPE};
    uint16_t min_alert_value{};//in 15s increments
    uint16_t max_alert_value{};//in 100ms increments
};
/**
 * @brief RGB values for NRF.
 */
struct rgb_vals {
    uint8_t red{};
    uint8_t green{};
    uint8_t blue{};
    uint8_t mode{};
    bool operator ==(rgb_vals const& obj) {
        return (this->blue == obj.blue &&
            this->red == obj.red &&
            this->green == obj.green &&
            this->mode == obj.mode);
    }
};
/// <summary>
/// <para>gets device id from cpuinfo</para>
/// </summary>
/// <returns><para>true on successful system call</para> false otherwise </returns>
bool get_device_id();
/// <summary>
/// <para>gets paired list from web backend </para>
/// NOTE: vector is cleared in function
/// </summary>
/// <param name="paired_devs">get:: std::vector to be filled with paired devices</param>
/// <returns><para>true on successful system call</para> false otherwise </returns>
bool get_paired_list(std::vector<device_t>& paired_devs);
/// <summary>
/// gets thresholds for each sensor type
/// </summary>
/// <param name="paired_devs">get:: empty std::vector to be filled with thresholds</param>
/// <returns><para>true on successful system call</para> false otherwise </returns>
bool get_thresholds_list(std::vector<config_thresholds>& thresholds);
/**
@brief gets time configs for each type of external sensors
@param[out] time_configs get:: empty std::vector to be filled with configs
@return true on successful system call, false otherwise.
*/
bool get_Config_list(std::vector<config_time>& time_configs);
/**    
@brief Gets data from web backend.
This function retrieves various parameters from the web backend and manages communication with the TI board.
@param[out] relays A std::vector of 0 or 1 values to set or unset corresponding relay on the TI board
@note The vector is cleared in this function.
@link RELAY_OUT_CNT @endlink for more details.
@param[out] pairing A flag that indicates pairing mode for RF communication.
@param[out] wiring_check A flag that indicates whether to check the wiring.
@param[out] rgbm RGB values for the NRF module.
@param[out] brightness_mode A flag that indicates if the screen brightness is in auto mode.
@param[out] browser_ok Flag to clear browser cache restart it. false means restart
@param[in] line Line to be parsed as JSON
@param[in] size size of line data
@param[out] cmd shutdown from backend
@return true on successful system call, false otherwise.
*/
bool get_dynamic(std::vector<uint8_t>& relays,bool& pairing, bool& update_paired_list, bool& wiring_check, rgb_vals& rgbm,uint8_t& brighness_mode,bool& browser_ok, uint8_t* line, int size, bool& shutdownFromDynamic);
/**
 * @brief Sets wiring data to the web backend.
 *
 * @param wiring A std::vector of 0 or 1 values that indicate the wiring state of corresponding devices.
 * @return True on successful system call, false otherwise.
 */
bool setWiring(std::vector<uint8_t>& wiring);
/**
 * @brief Alerts the web about an error or an emergency.
 *
 * @param dev_id The serial number of the device.
 * @param err_code Error code, see Alert_types_en.
 * @param level Level, see Alert_lvl_en.
 * @return True on successful system call, false otherwise.
 */
bool setAlert(uint32_t dev_id,uint8_t err_code,uint8_t level);
/**
 * @brief Sets a pending external device to the web database.
 *
 * @param dev_one Device information.
 * @return True on successful system call, false otherwise.
 */
bool addPendingSensor(device_t& dev_one);
/**
 * @brief Sets sensor data from devices to the web database.
 *
 * @param dev The device that sent the data.
 * @param data Array of data.
 * @param size Size of data.
 * @return True on successful system call, false otherwise.
 */
bool setSensorData(device_t& dev, uint8_t* data, uint16_t size);
/**
 * @brief Sets fan data from NRF to the web database.
 *
 * @param fan_speed Value of fan speed.
 * @return True on successful system call, false otherwise.
 */
bool set_fan_speed_INFO( uint16_t fan_speed);
/**
 * @brief Sets the brightness of the screen according to the ambient luminosity.
 *
 * @param Luminosity Luminosity from ambient light sensor.
 * @return True on successful system call, false otherwise.
 */
bool setBrightness(uint32_t Luminosity);
/**
 * @brief Sets NRF Hardware Version to the web database.
 *
 * @param HW Hardware version from NRF.
 * @param len Length of the HW string.
 * @return True on successful system call, false otherwise.
 */
bool setHW(char* HW,int len);

/**
 * @brief Saves Tof status and Luminosity value to file for web based on data that daemon received from NRF sensors
 *
 * @param RangeMilliMeter Range before some object
 * @param Luminosity Value of luminosity 
 */

void saveBrTofTofile(uint16_t RangeMilliMeter, uint32_t Luminosity);
