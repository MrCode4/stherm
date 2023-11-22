<?php

/**
 * Get device's config
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->getConfig();
echo $result;
gc_collect_cycles();





