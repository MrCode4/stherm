#include "currentstage.h"

CurrentStage::CurrentStage(QObject *parent)
    : QObject{parent}
{
    mode = 0;
    stage = 0;
    timestamp = current_timestamp();
    blink_mode = 0;
    s2offtime = current_timestamp() - minuteToTimestamp(5);;

}
