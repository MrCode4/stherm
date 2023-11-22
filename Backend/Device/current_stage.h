#pragma once

#include "nuve_types.h"

namespace NUVE {

struct CurrentStage
{
    CurrentStage() = default;

    uint32_t mode;
    uint32_t stage;

    // update from Scheme (startWork)
    timestamp_t timestamp;
    uint32_t blink_mode;

    // stage_2_off_time = (current_timestamp - s2offtime)/60 > 2) ? 1 : 0
    // update from Scheme (startWork)
    timestamp_t s2offtime;

    void setDefaultValues(void);

    bool getS2OffTime();

    bool getDelayTime();
};

} // namespace NUVE
