<?php

/**
 * Get paired sensors
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->getPaired();
echo $result;
gc_collect_cycles();

