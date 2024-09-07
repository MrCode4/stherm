#include "ScheduleCPP.h"

ScheduleCPP::ScheduleCPP(QSObjectCpp *parent) :
    QSObjectCpp(parent)
{
    // Defaults
    enable     = true;
    active     = false;
    temprature = 18; // Celsius
    humidity   = 0;  // Percentage

    // TODO: Set defaults for minimum and maximum temperatures

    // default: Away
    type = 0;
}


double ScheduleCPP::effectiveTemperature(AppSpecCPP::SystemMode systemMode) {

    return 0.0;
}
