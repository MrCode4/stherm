#include "Scheme.h"

Scheme::Scheme()
{

}

void Scheme::checkVacation()
{
    if ($current_mode === 'cooling') {
        if ($current_temp > $set_temp - self::STAGE1_OFF_RANGE) { // before stage 1 off
            $real_set_mode = 'cooling';
        } elseif ($current_temp > $set_temp - self::STAGE1_ON_RANGE) { // before stage 1 on
            $real_set_mode = 'off';
        } else {  // stage 1 on
            $real_set_mode = 'heating';
        }
    } elseif ($current_mode === 'heating') {
        if ($current_temp < $set_temp + self::STAGE1_OFF_RANGE) { // before stage 1 off
            $real_set_mode = 'heating';
        } elseif ($current_temp < $set_temp + self::STAGE1_ON_RANGE) { // before stage 1 on
            $real_set_mode = 'off';
        } else {  // stage 1 on
            $real_set_mode = 'cooling';
        }
    } else { // OFF
        if ($current_temp < $set_temp - self::STAGE1_ON_RANGE) {
            $real_set_mode = 'heating';
        } elseif ($current_temp > $set_temp + self::STAGE1_ON_RANGE) {
            $real_set_mode = 'cooling';
        }
    }
}
