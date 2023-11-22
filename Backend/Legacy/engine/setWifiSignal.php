<?php

/**
 * Set current wifi signal
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$wifi = new HardwareWork();
$result = $wifi->setCurrentWifiSignal(json_decode($argv[1]));
echo $result;
gc_collect_cycles();
gc_collect_cycles();
