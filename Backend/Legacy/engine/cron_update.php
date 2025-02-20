<?php

ini_set('display_errors', '1');
ini_set('display_startup_errors', '1');
error_reporting(E_ALL);
use project\Update;
require_once "project/Update.php";

$update = new Update();
$update->install();