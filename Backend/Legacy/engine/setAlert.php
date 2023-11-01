<?php

/**
 * Set new alert
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->setAlert(json_decode($argv[1]));
echo $result;
gc_collect_cycles();


