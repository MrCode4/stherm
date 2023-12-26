#include "ScheduleCPP.h"

ScheduleCPP::ScheduleCPP(QSObjectCpp *parent) :
    QSObjectCpp(parent)
{
    connect(this, &ScheduleCPP::enableChanged, this, [=]() {
        if (!enable) {
            _active = false;
            activeChanged();
        }
    });
}
