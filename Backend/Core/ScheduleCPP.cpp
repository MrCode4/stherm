#include "ScheduleCPP.h"

ScheduleCPP::ScheduleCPP(QSObjectCpp *parent) :
    QSObjectCpp(parent)
{
    // Defaults
    enable     = true;
    _active    = false;
    temprature = 18; // Celsius
    humidity   = 0; // Percentage

    connect(this, &ScheduleCPP::enableChanged, this, [=]() {
        if (!enable) {
            _active = false;
            activeChanged();
        }
    });
}
