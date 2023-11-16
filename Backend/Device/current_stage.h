#pragma once

#include "nuve_types.h"

namespace NUVE {

struct CurrentStage
{
    CurrentStage() = default;

    uint32_t mode;
    uint32_t stage;
    timestamp_t timestamp;
    uint32_t blink_mode;
    timestamp_t s2offtime;

    void setDefaultValues(void);
};

} // namespace NUVE
