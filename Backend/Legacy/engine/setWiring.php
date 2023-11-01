<?php

/**
 * Set wirings state
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->setWiring($argv[1]);
echo $result;
gc_collect_cycles();
gc_collect_cycles();