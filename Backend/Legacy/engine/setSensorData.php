<?php

/**
 * Set sensor's data
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->setSensorData(json_decode($argv[1]));
//$result = $argv[1];
echo $result.PHP_EOL;
gc_collect_cycles();

