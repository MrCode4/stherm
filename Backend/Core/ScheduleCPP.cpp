#include "ScheduleCPP.h"

#include "AppSpecCPP.h"
#include "LogHelper.h"

ScheduleCPP::ScheduleCPP(QSObjectCpp *parent) :
    QSObjectCpp(parent)
{
    // Defaults
    enable     = true;
    active     = false;
    humidity   = 0;  // Percentage

    // Set defaults for minimum and maximum temperatures
    // Celsius
    minimumTemperature = 15.5556;
    maximumTemperature = 29.4444;

    // default: Away
    type = 0;

    // Invalid id
    id   = -1;

    // Off
    systemMode = AppSpecCPP::SystemMode::Off;
}

double ScheduleCPP::effectiveTemperature(const AppSpecCPP::SystemMode &sysMode) {
    double eff = minimumTemperature;

    if (sysMode == AppSpecCPP::SystemMode::Cooling) {
        // Cool to
        eff = maximumTemperature;

    } else if (sysMode == AppSpecCPP::SystemMode::Heating) {
        // Heat to
        eff = minimumTemperature;

    } else {
        TRACE << "Mode not supported. mode: " << sysMode;
    }

    return eff;
}
