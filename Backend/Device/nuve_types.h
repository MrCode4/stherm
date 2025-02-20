#pragma once

#include <cstdint>
#include <ctime>
#include <string>

namespace NUVE {

/////////////// Configurations

/// Is dayligh savings time used?  0 = no, 1 = yes, -1 = attempt to let os decide
static const int LOCALE_USE_DST = -1;

/////////////// Definitions.  I know that this file is not likely the right place, but for now its a signle inlcude

/// Set the interval, in days, after which the data will be deleted
static const int DELETE_INFO_INTERVAL = 7; // 1 week

/// This is the timeout that is used whenver code calls the 'exec' commadn
static const int EXEC_TIMEOUT_INTERVAL = 30;

///////////////////// Web URLs
// TODO check the validity of hte following descriptions
/// Set the url used to view the technician qr code
/// TODO refactor this nameing
inline static const std::string TECHNIC_QR = "#EN/USA/technician/view/";
/// set the url used to edit hte techicion qr code
inline static const std::string TECHNIC_EDIT_QR = "#EN/USA/technician/edit/";

// Common types

/** @brief the type for the system unique ID */
typedef std::string cpuid_t;

// TODO class, with limits and getters/setters?
typedef struct rgbVal
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;
} rgbVal_t;

// TODO timestamp needs a class

/**
 * @brief timestamp type
 */
typedef time_t timestamp_t;

// TODO refactor, e.g. timestampNow()
static timestamp_t current_timestamp(void)
{
    return std::time(0);
}

// TODO refactor, e.g. timestampFromMinutes?
static timestamp_t minuteToTimestamp(uint32_t mins)
{
    return mins * 60;
}

} // namespace NUVE
