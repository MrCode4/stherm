#include "ScheduleCPP.h"

#include "AppSpecCPP.h"

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

    // Off
    systemMode = AppSpecCPP::SystemMode::Off;
}
