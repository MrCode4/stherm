<?php

/**
 * Add new pending sensor
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->addPendingSensor(json_decode($argv[1])->data);
echo $result;
gc_collect_cycles();
gc_collect_cycles();
