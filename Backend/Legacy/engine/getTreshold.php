<?php

/**
 * Get sensor's config
 */
use project\HardwareWork;
require_once "project/HardwareWork.php";
$hardware = new HardwareWork();
$result = $hardware->getTreshold();
echo $result;
gc_collect_cycles();
gc_collect_cycles();