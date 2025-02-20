<?php

/**
 * Get brightness mode
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->getBrightnessMode();
echo $result;
gc_collect_cycles();
gc_collect_cycles();