#include "current_stage.h"
#include "Core/hardware.h"

void NUVE::CurrentStage::setDefaultValues()
{
    mode = 0;
    stage = 0;
    timestamp = current_timestamp();
    blink_mode = 0;
    s2offtime = current_timestamp() - minuteToTimestamp(5);
}

bool NUVE::CurrentStage::getS2OffTime()
{
    return difftime(current_timestamp(), s2offtime) / 60 > 2;
}

bool NUVE::CurrentStage::getDelayTime()
{
//    NUVE::Hardware hardware();
 //  system_delay FROM settings
    int delay = 100; // 100 is temp / hardware.settings.system_delay

    return difftime(current_timestamp(), timestamp) / 60 > delay;
}
