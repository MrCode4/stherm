<?php

/**
 * Get dynamic info // relay_state, pairing_mode, wiring_check, backlight, brightness_mode, last_update
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
for($i = 0; $i < 10; $i++) {
    $hardware = new HardwareWork();
    $result = $hardware->getDynamic();

    // Outputs the value stored in $result followed by a newline (PHP_EOL)
    echo $result . PHP_EOL;

    // Pauses the script execution for 1 second
    sleep(1); // second
}
gc_collect_cycles();
gc_collect_cycles();