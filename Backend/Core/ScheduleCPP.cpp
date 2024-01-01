#include "ScheduleCPP.h"

ScheduleCPP::ScheduleCPP(QSObjectCpp *parent) :
    QSObjectCpp(parent)
{
    // Defaults
    enable     = true;
    temprature = 18; // Celsius
    humidity   = 0; // Percentage
}
