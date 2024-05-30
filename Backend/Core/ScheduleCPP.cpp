#include "ScheduleCPP.h"

ScheduleCPP::ScheduleCPP(QSObjectCpp *parent) :
    QSObjectCpp(parent)
{
    // Defaults
    enable     = true;
    active     = false;
    temprature = 18; // Celsius
    humidity   = 0;  // Percentage

    // default: Away
    type = 0;
}
