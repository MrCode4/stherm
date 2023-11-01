<?php

/**
 * Set brightness data
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->setBrightnessData($argv[1]);
echo $result;
gc_collect_cycles();

