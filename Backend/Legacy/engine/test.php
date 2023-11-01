<?php //declare(strict_types=1);
phpinfo();
///**
// * Copyright (c) 2020. Z-Soft All right reserved
// * @author  Suren Danielyan
// */
//
//define('DOCUMENT_ROOT', $_SERVER['DOCUMENT_ROOT']);
//define("SYSTEM_SETTINGS", parse_ini_file(DOCUMENT_ROOT . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'system.ini'));
////ini_set("display_errors","On");
////ini_set("display_startup_errors","On");
//
//require_once "../configuration/message_types.php";
//require_once "core/Z_Autoloader.php";
//
//use core\Frontend;
//
//if(session_status() != PHP_SESSION_ACTIVE) {
//    session_start();
//}
//
//
//$all_data = file_get_contents('php://input');
//if (empty($all_data)) {
//    throw new Exception("Empty JSON data");
//}
//$income_data = json_decode($all_data);
//if (json_last_error() !== JSON_ERROR_NONE) { // WRONG DATA
//    throw new Exception(json_last_error_msg());
//}
//
//$_SESSION['user_id'] = 0;
//$_SESSION['token'] = '';
//$_SESSION['host_id'] = 0;
//$_SESSION['type_id'] = 0;
//$_SESSION['lang_id'] = 0;
//
//
//$class = 'hardware';
//$method = 'getBacklight';
//$params = [];
////$result = false;
//
//$result = Frontend::execute($class, $method, $params);
//echo json_encode($result);