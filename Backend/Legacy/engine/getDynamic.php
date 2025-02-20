<?php

/**
 * Get dynamic info // relay_state, pairing_mode, wiring_check, backlight, brightness_mode, last_update
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->getDynamic();
echo $result;
gc_collect_cycles();

